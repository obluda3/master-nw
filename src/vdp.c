#include "vdp.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
VDP* vdp;
bool waitingForWrite = false;
bool writeCram = false;
u16 fullCommand = 0;
u8 readBuffer = 0;

bool inline get_bit(u8 data, int bitnum) { (data >> bitnum) & 1; }
u16 inline address_register() { return fullCommand & 0x3FFF; }

void inline increment() { 
  if (fullCommand == 0x3FFF) fullCommand &= 0xC000;
  else fullCommand++;
}

void init_vdp(VDP* _vdp) {
  vdp = _vdp;
  memset(vdp, 0, sizeof(*vdp));
}

void process_commandwrite(u8 byte) {
  if (!waitingForWrite) {   
    fullCommand &= 0xFF00;
    fullCommand |= byte;
    waitingForWrite = true;
  }
  else {
    fullCommand &= 0xFF;
    fullCommand |= byte << 8;
    u8 controlCode = fullCommand >> 14;
    switch (controlCode) {
      case 0:
        readBuffer = vdp->vram[address_register()];
        increment();
        break;
      case 1:
        writeCram = false;
        break;
      case 2:
        writeCram = false;
        u8 data = fullCommand & 0xFF;
        u8 reg = (fullCommand >> 8) & 0xF;
        vdp->registers[reg] = data;
        if (reg == 1 && get_bit(vdp->registers[1], 5) && get_bit(vdp->statusReg, 7)) vdp->requestInterrupt = true;
        break;
      case 3:
        writeCram = true;
    }
  }
}

void process_datawrite(u8 byte) {
  readBuffer = byte;
  if (writeCram)
    vdp->cram[address_register() & 0x1F] = byte;
  else
    vdp->vram[address_register()] = byte;
  increment();
}

void render_frame() {

}

u8 get_statusregister() {
  
}

u8 get_vreset();
u8 get_vresetdest();
bool needsReset = true;
void vdp_update(float vdpcycles) {
  u16 hcount = vdp->hcounter;
  int cycles = (int)vdpcycles;
  int hinc = cycles/2;

  bool nextLine = hinc + hcount > 684;
  vdp->hcounter = (hcount + hinc) % 685;

  if (nextLine) {
    u8 vcount = vdp->vcounter;
    vdp->vcounter++;
    // start new frame
    if (vcount == 255) {
      vdp->vcounter = 0;
      needsReset = true;
      render_frame();
    }

    // handle vcounter jumping
    else if (vcount == get_vreset() && needsReset) {
      vcount = get_vresetdest();
      needsReset = false;
    } 

    else if (vcount == vdp->height) {

    }



  }
}