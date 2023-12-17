#pragma once
#include "types.h"

typedef struct {
  u8 vram[0x4000];
  u8 cram[0x20];
  u8 vcounter;
  u16 realVcounter;
  u16 hcounter;
  u8 registers[11];
  u8 statusReg;
  u8 lineCounter;
  bool frameInterrupt;
  bool lineInterrupt;
  u8 vscroll;
} VDP;

void init_vdp(VDP* _vdp);
void process_datawrite(u8 byte);
void process_controlwrite(u8 byte);
u8 get_statusregister();
bool vdp_is_interrupt();
u8 get_dataport();
void vdp_update(float vdpcycles);
u16 address_register2();
bool writeram();