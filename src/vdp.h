#pragma once
#include "types.h"

typedef struct {
  u8 vram[0x4000];
  u8 cram[0x20];
  u8 vcounter;
  u8 hcounter;
  u16 width;
  u16 height;
  u8 registers[11];
  u8 statusReg;
  bool requestInterrupt;
} VDP;

void init_vdp(VDP* _vdp);
void process_datawrite(u8 byte);
void process_controlwrite(u8 byte);
u8 get_statusregister();
u8 get_dataport();
void vdp_update(float vdpcycles);