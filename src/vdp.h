#pragma once
#include "types.h"
#include "platform.h"

#ifdef TARGET_LINUX
typedef Color color;
#else
typedef eadk_color_t color;
#endif

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
  color framebuffer[256*192];
} VDP;

void vdp_init(VDP* _vdp);
u8 get_statusreg();
bool vdp_is_interrupt();
u8 vdp_read_io(u8 port);
void vdp_write_io(u8 port, u8 value);
u8 get_dataport();
void vdp_update(float vdpcycles);
