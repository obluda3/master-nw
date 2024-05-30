Q ?= @
BUILD_DIR = output
NWLINK = npx --yes -- nwlink@0.0.16
LTO = 1
LINK_GC = 1
LIB_FOLDER = /usr/local/include
LIB_BIN = /usr/local/lib

# Compiler and flags
CC = arm-none-eabi-gcc


# Determine the target environment (default is NumWorks)
TARGET_ENV ?= numworks

# Linux-specific flags
ifeq ($(TARGET_ENV), linux)
	CC = gcc
	CFLAGS = -std=c99
	CFLAGS += -Wall
	CFLAGS += -ggdb
	CFLAGS += -O0
	CFLAGS += -I$(LIB_FOLDER)
	CFLAGS += -Ilib
	CFLAGS += -DTARGET_LINUX
	CFLAGS += -Wl,-rpath,$(LIB_BIN)
	LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lc
	
	BUILD_TARGET = app_linux
	ICON_TARGET = # No icon target for Linux
	LTO = 0
	LINK_GC = 0
else
	BUILD_TARGET = app.nwa
	CFLAGS = -std=c99
	CFLAGS += $(shell $(NWLINK) eadk-cflags)
	CFLAGS += -O3 -Wall
	CFLAGS += -ggdb
	CFLAGS += -I$(LIB_FOLDER)
	# Linker flags
	LDFLAGS = -Wl,--relocatable
	LDFLAGS += -nostartfiles
	LDFLAGS += --specs=nano.specs
	ICON_TARGET = $(BUILD_DIR)/icon.o
endif

# Garbage collection sections (NumWorks specific)
ifeq ($(LINK_GC),1)
	CFLAGS += -fdata-sections -ffunction-sections
	LDFLAGS += -Wl,-e,main -Wl,-u,eadk_app_name -Wl,-u,eadk_app_icon -Wl,-u,eadk_api_level
	LDFLAGS += -Wl,--gc-sections
endif

# Link-time optimization (NumWorks specific)
ifeq ($(LTO),1)
	CFLAGS += -flto -fno-fat-lto-objects
	CFLAGS += -fwhole-program
	CFLAGS += -fvisibility=internal
	LDFLAGS += -flinker-output=nolto-rel
endif
# Automatically discover source files
SRC_DIR = src
src = $(wildcard $(SRC_DIR)/*.c)

# Object files derived from source files
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(src))

.PHONY: build build_linux check run clean

# Build target
build: $(BUILD_DIR)/$(BUILD_TARGET) $(ICON_TARGET)

# Check target
check: $(BUILD_DIR)/app.bin

# Run target
run: $(BUILD_DIR)/$(BUILD_TARGET) movethecharacter.sms
	@echo "INSTALL $<"
	$(Q) $(NWLINK) install-nwa --external-data movethecharacter.sms $<

# Binary target
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/$(BUILD_TARGET) movethecharacter.sms
	@echo "BIN	 $@"
	$(Q) $(NWLINK) nwa-bin --external-data movethecharacter.sms $< $@

# ELF target
$(BUILD_DIR)/%.elf: $(BUILD_DIR)/$(BUILD_TARGET) movethecharacter.sms
	@echo "ELF	 $@"
	$(Q) $(NWLINK) nwa-elf --external-data movethecharacter.sms $< $@

# NWA and Linux executable target
$(BUILD_DIR)/app.nwa $(BUILD_DIR)/app_linux: $(OBJ) $(ICON_TARGET)
	@echo "LD	  $@"
	$(Q) $(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compile C source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "CC	  $@"
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

# Generate icon.o from icon.png (NumWorks specific)
$(BUILD_DIR)/icon.o: $(SRC_DIR)/icon.png | $(BUILD_DIR)
	@echo "ICON	$<"
	$(Q) $(NWLINK) png-icon-o $< $@

# Create build directory
$(BUILD_DIR):
	$(Q) mkdir -p $@

# Clean target
clean:
	@echo "CLEAN"
	$(Q) rm -rf $(BUILD_DIR)
