#pragma once
#include "types.h"

typedef struct {
  u8 internal_memory[0x10000];
  u8 ram_banks[0x4000][2];
  u8* cartridge_memory;
} Memory;

void write_u8(u16 addr, u8 data);
void write_u16(u16 addr, u16 data);
u8 read_u8(u16 addr);
u16 read_u16(u16 addr);
u8 read_io(u16 addr);
//void set_input(eadk_keyboard_state_t keyboardState);
void write_io(u16 addr, u8 value);
void set_input(u8 val);
void init_mem(Memory* memory, const char* cartridge);