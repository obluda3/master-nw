// CHECK LA LECTURE DE CHAQUE SPRITE 
// VERIFIER QUON A LES BONS PATTERNS ET COULEURS
// IL FAUDRAIT POUVOIR AFFICHER LE VDP SEULEMENT A PARTIR DU MOMENT OU ON A FINI LE PROGRAMME

#include "vdp.h"
#include "platform.h"
#include <stdio.h>
#include <eadk.h>
#include <string.h>
#include <math.h>
#define VDP_WIDTH 256
#define VDP_HEIGHT 192
#define INLINED __attribute__((always_inline)) inline

VDP* vdp;
bool waitingForWrite = false;
bool writeCram = false;
u16 fullCommand = 0;
u8 readBuffer = 0;

INLINED bool get_bit(u8 data, int bitnum) { return (data >> bitnum) & 1; }

u16 address_register() { return fullCommand & 0x3FFF; }
INLINED void increment() { 
  if (fullCommand == 0x3FFF) fullCommand &= 0xC000;
  else fullCommand++;
}

u8 get_statusregister() { 
  u8 result = vdp->statusReg;
  vdp->statusReg &= 0x1F;
  waitingForWrite = false;
  vdp->requestInterrupt = false;
  return result; 
}

u8 get_dataport() { 
  u8 result = readBuffer;
  waitingForWrite = false;
  readBuffer = vdp->vram[address_register()];
  increment();
  return result;
}

void init_vdp(VDP* _vdp) {
  vdp = _vdp;
  memset(vdp, 0, sizeof(*vdp));
}

void process_controlwrite(u8 byte) {
  if (!waitingForWrite) { 
    fullCommand &= 0xFF00;
    fullCommand |= byte;
    waitingForWrite = true;
  }
  else {
    fullCommand &= 0xFF;
    fullCommand |= (byte << 8);
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
    waitingForWrite = false;
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

u16 get_sprite_address_table() {
  return (vdp->registers[5] & 0x7E) << 7;
}

u16 get_color(int index, bool sprite) {
  u8 clr;
  return vdp->cram[16*sprite+index];
}


void render_frame() {
  if (!get_bit(vdp->registers[1], 6)) {
    return;
  }
  bool sprite_pixels[VDP_WIDTH] = {0};
  u8 lineBuffer[VDP_WIDTH] = {0};

  // render background
  
  bool modified = false;
  // render sprites
  u8* satBase = &vdp->vram[get_sprite_address_table()]; 
  int y = vdp->realVcounter;
  for (int i = 0; i < 64; i++) {
    s8 spriteY = satBase[i] + 1;
    if (spriteY == -47) break;
    s16 spriteX = *(satBase + 128 + 2*i);
    if (get_bit(vdp->registers[0], 3))
      spriteX -= 8;

    int patternIndex = *(satBase + 128 + 2*i + 1);
    if (get_bit(vdp->registers[6], 2)) patternIndex += 256;

    if (!(y >= spriteY && y < spriteY + 8)) continue; // skip sprites that are not in scanline

    modified = true;
    int lineInSprite = y - spriteY;
    u8* pixelsLine = (&vdp->vram[32 * patternIndex] + 4 * lineInSprite);
    u8 b1 = pixelsLine[3];
    u8 b2 = pixelsLine[2];
    u8 b3 = pixelsLine[1];
    u8 b4 = pixelsLine[0];

    for (int x = 0; x < 8; x++) {
      if (sprite_pixels[spriteX + x]) {
        vdp->statusReg = vdp->statusReg | 16;
        continue;
      }
      int paletteIndex = (get_bit(b1, 7-x) << 3) + (get_bit(b2, 7-x) << 2) + (get_bit(b3, 7-x) << 1) + get_bit(b4, 7-x);
      u8 clr = get_color(paletteIndex, true);

      sprite_pixels[spriteX + x] = 1;
      lineBuffer[spriteX + x] = clr;
    }

  }
  draw_line(y, lineBuffer);
}

u8 get_vreset() { return 218; };
u8 get_vresetdest() { return 212; };
bool needsReset = true;

void vdp_update(float vdpcycles) {
  u16 hcount = vdp->hcounter;
  int cycles = (int)vdpcycles;
  int hinc = cycles/2;

  bool nextLine = hinc + hcount > 684;
  vdp->hcounter = (hcount + hinc) % 685;

  if (nextLine) {
    u16 realVcounter = vdp->realVcounter;
    if (realVcounter < VDP_HEIGHT) render_frame();
    
    else if (realVcounter >= VDP_HEIGHT) {
      vdp->vscroll = vdp->registers[9];
    }

    if (realVcounter <= VDP_HEIGHT) {
      if (vdp->lineInterrupt-- == 256) {
        vdp->lineInterrupt = vdp->registers[10];
        if (get_bit(vdp->registers[0], 4)) {
          vdp->requestInterrupt = true;
        }
      }
    }
    else vdp->lineInterrupt = vdp->registers[10];

    if (realVcounter == VDP_HEIGHT) {
      vdp->statusReg = vdp->statusReg | 0x80;
    }

    if (vdp->realVcounter == get_vreset()) vdp->vcounter = get_vresetdest();

    if (vdp->realVcounter == 261) {
      vdp->vcounter = 0;
      vdp->realVcounter = 0;
    }
    else {
      vdp->vcounter++;
      vdp->realVcounter++;
    }
  }
  if (get_bit(vdp->statusReg, 7) && get_bit(vdp->registers[1], 5))
    vdp->requestInterrupt = true;
}