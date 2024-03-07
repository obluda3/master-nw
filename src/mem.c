#include "emu.h"
#include <string.h>
#include "vdp.h"

Memory* mem;

u8 slot1 = 0;
u8 slot2 = 1;
u8 slot3 = 2;
u8 ram_bank = 0;
u8 current_keys = 0xFF;
bool ram_slot3 = false;

void init_mem(Memory* memory, const char* cartridge) {
  mem = memory;
  mem->cartridge_memory = (u8*)cartridge;
  memset(mem->internal_memory, 0, sizeof(mem->internal_memory));
}

void update_pages(u16 addr, u8 data) {
  data &= 0x1F;
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
    mem->ram_banks[ram_bank][real_addr] = data;
    return;
  }

  mem->internal_memory[addr] = data;
  if (addr >= 0xFFFC) update_pages(addr, data);

  // memory mirroring
  u16 mirrorred = addr - 0xC000 >= 0x2000 ? addr - 0x2000 : addr + 0x2000;
  mem->internal_memory[mirrorred] = data;
}

u8 read_u8(u16 addr) {
  if (addr < 0x4000) return mem->cartridge_memory[0x4000 * slot1 + addr];
  if (addr < 0x8000) return mem->cartridge_memory[0x4000 * slot2 + (addr - 0x4000)];
  if (addr < 0xC000) {
    if (ram_slot3) return mem->ram_banks[ram_bank][addr - 0x8000];
    else return mem->cartridge_memory[0x4000 * slot3 + (addr - 0x8000)];
  }
  return mem->internal_memory[addr];
}

u16 read_u16(u16 addr) {
  return (u16)(read_u8(addr) | (read_u8(addr + 1) << 8));
}

void write_u16(u16 addr, u16 data) {
  write_u8(addr, data & 0xFF);
  write_u8(addr + 1, (data & 0xFF00) >> 8);
}

u8 read_io(u16 addr) {
  switch (addr) {
    case 0xDC: case 0xC0:
      return 0;
    case 0xDD: case 0xC1:
      return 0xFF;
    case 0x7E: case 0x7F: case 0xBE: case 0xBF:
      return vdp_read_io(addr);
    default: 
      return 0;
  }
}

void set_input(u8 val) { current_keys = val; }

void write_io(u16 addr, u8 value) {
  switch (addr) {
    case 0xBE: case 0xBF: vdp_write_io(addr, value);
  }
}