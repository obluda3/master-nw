#include "mem.h"
#include <string.h>

Memory mem;

u8 slot1 = 0;
u8 slot2 = 1;
u8 slot3 = 2;
u8 ram_bank = 0;
bool ram_slot3 = false;

void init_mem(u8* cartridge) {
  mem.cartridge_memory = cartridge;
  memset(mem.internal_memory, 0, sizeof(mem.internal_memory));
}

void update_pages(u16 addr, u8 data) {
  if (addr == 0xFFFC) {
    ram_slot3 = (data & 8) == 8;
    if (ram_slot3)
      ram_bank = (data & 4) >> 2; 
  }
  else if (addr == 0xFFFD) {
    slot1 = data;
  }
  else if (addr == 0xFFFE) {
    slot2 = data;
  }
  else if (addr == 0xFFFF) {
    slot3 = data;
  }
}

void write_u8(u16 addr, u8 data) {
  // writing to rom is impossible
  if (addr < 0x8000) {
    return;
  }
  // handle slot3
  if (addr < 0xC000) {
    // can't write if ram isn't mapped
    if (!ram_slot3)
      return;
    u16 real_addr = addr - 0x8000;
    mem.ram_banks[ram_bank][real_addr] = data;
    return;
  }

  mem.internal_memory[addr] = data;
  if (addr >= 0xFFFC) update_pages(addr, data);

  // memory mirroring
  u16 mirrorred = addr - 0xC000 >= 0x2000 ? addr - 0x2000 : addr + 0x2000;
  mem.internal_memory[mirrorred] = data;
}

u8 read_u8(u16 addr) {
  if (addr < 0x4000) return mem.cartridge_memory[0x4000 * slot1 + addr];
  if (addr < 0x8000) return mem.cartridge_memory[0x4000 * slot2 + (addr - 0x4000)];
  if (addr < 0xC000) {
    if (ram_slot3) return mem.ram_banks[ram_bank][addr - 0x8000];
    else return mem.cartridge_memory[0x4000 * slot3 + (addr - 0x8000)];
  }
  return mem.internal_memory[addr];
}
