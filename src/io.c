#include "io.h"
#include "vdp.h"
#include <stdio.h>

u8 current_keys = 0;
bool controllerOn = true;

u8 read_io(u16 addr) {
  switch (addr) {
    case 0xDC: case 0xC0:
      return current_keys;
    case 0xDD: case 0xC1:
      return 0xFF;
    case 0x7E: case 0x7F: case 0xBE: case 0xBF:
      return vdp_read_io(addr);
    default: 
      printf("Unimplemented IO Read Port %x\n", addr);
      return 0;
  }
}

void set_input(u8 val) { 
    if (controllerOn) {
      current_keys = val; 
    }
}
int cnt = 1;
void write_io(u16 addr, u8 value) {
  switch (addr) {
    case 0xBE: case 0xBF: 
      vdp_write_io(addr, value);
      break;
    case 0x3E:
      controllerOn = !(value >> 2);
      current_keys = controllerOn ? current_keys : 0;
      break;
    case 0xFC:
      break;
    case 0xFD:
      break;
    default:
      printf("Unimplemented IO Write Port %x\n", addr);
      break;
  }
}