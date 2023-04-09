#include "types.h"

typedef struct {
  u8 internal_memory[0x10000];
  u8 ram_banks[0x4000][2];
  u8* cartridge_memory;
} Memory;

void write_u8(u16 addr, u8 data);
u8 read_u8(u16 addr);
void init_mem(u8* cartridge);