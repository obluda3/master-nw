#include "vdp.h"
#include "platform.h"
#include <stdio.h>
#include <eadk.h>
#include <string.h>
#include <math.h>
#define INLINED __attribute__((always_inline)) inline

VDP *vdp;
bool waitingForWrite = false;
bool writeCram = false;
u16 fullCommand = 0;
u8 readBuffer = 0;

INLINED bool get_bit(u8 data, int bitnum) { return (data >> bitnum) & 1; }

u16 address_register() { return fullCommand & 0x3FFF; }
INLINED void increment()
{
  if (fullCommand == 0x3FFF)
    fullCommand &= 0xC000;
  else
    fullCommand++;
}

u8 get_statusreg()
{
  u8 result = vdp->statusReg;
  vdp->statusReg &= 0x1F;
  waitingForWrite = false;
  vdp->frameInterrupt = false;
  vdp->lineInterrupt = false;
  return result;
}

u8 get_dataport()
{
  u8 result = readBuffer;
  waitingForWrite = false;
  readBuffer = vdp->vram[address_register()];
  increment();
  return result;
}

u8 vdp_read_io(u8 port)
{
  if (port == 0xBE)
    return get_dataport();
  if (port == 0xBF)
    return get_statusreg();
  if (port == 0x7E)
    return vdp->vcounter;
  if (port == 0x7F)
    return vdp->hcounter;
  return 0;
}

void vdp_init(VDP *_vdp)
{
  vdp = _vdp;
  memset(vdp, 0, sizeof(*vdp));
}
#ifdef TARGET_LINUX

color conversion_table[] = {
    {0, 0, 0, 255},
    {85, 0, 0, 255},
    {170, 0, 0, 255},
    {255, 0, 0, 255},
    {0, 85, 0, 255},
    {85, 85, 0, 255},
    {170, 85, 0, 255},
    {255, 85, 0, 255},
    {0, 170, 0, 255},
    {85, 170, 0, 255},
    {170, 170, 0, 255},
    {255, 170, 0, 255},
    {0, 255, 0, 255},
    {85, 255, 0, 255},
    {170, 255, 0, 255},
    {255, 255, 0, 255},
    {0, 0, 85, 255},
    {85, 0, 85, 255},
    {170, 0, 85, 255},
    {255, 0, 85, 255},
    {0, 85, 85, 255},
    {85, 85, 85, 255},
    {170, 85, 85, 255},
    {255, 85, 85, 255},
    {0, 170, 85, 255},
    {85, 170, 85, 255},
    {170, 170, 85, 255},
    {255, 170, 85, 255},
    {0, 255, 85, 255},
    {85, 255, 85, 255},
    {170, 255, 85, 255},
    {255, 255, 85, 255},
    {0, 0, 170, 255},
    {85, 0, 170, 255},
    {170, 0, 170, 255},
    {255, 0, 170, 255},
    {0, 85, 170, 255},
    {85, 85, 170, 255},
    {170, 85, 170, 255},
    {255, 85, 170, 255},
    {0, 170, 170, 255},
    {85, 170, 170, 255},
    {170, 170, 170, 255},
    {255, 170, 170, 255},
    {0, 255, 170, 255},
    {85, 255, 170, 255},
    {170, 255, 170, 255},
    {255, 255, 170, 255},
    {0, 0, 255, 255},
    {85, 0, 255, 255},
    {170, 0, 255, 255},
    {255, 0, 255, 255},
    {0, 85, 255, 255},
    {85, 85, 255, 255},
    {170, 85, 255, 255},
    {255, 85, 255, 255},
    {0, 170, 255, 255},
    {85, 170, 255, 255},
    {170, 170, 255, 255},
    {255, 170, 255, 255},
    {0, 255, 255, 255},
    {85, 255, 255, 255},
    {170, 255, 255, 255},
    {255, 255, 255, 255},
};

#else
color conversion_table[64] = {
    0x0000,
    0x5000,
    0xa000,
    0xf800,
    0x02a0,
    0x52a0,
    0xa2a0,
    0xfaa0,
    0x0540,
    0x5540,
    0xa540,
    0xfd40,
    0x07e0,
    0x57e0,
    0xa7e0,
    0xffe0,
    0x000a,
    0x500a,
    0xa00a,
    0xf80a,
    0x02aa,
    0x52aa,
    0xa2aa,
    0xfaaa,
    0x054a,
    0x554a,
    0xa54a,
    0xfd4a,
    0x07ea,
    0x57ea,
    0xa7ea,
    0xffea,
    0x0014,
    0x5014,
    0xa014,
    0xf814,
    0x02b4,
    0x52b4,
    0xa2b4,
    0xfab4,
    0x0554,
    0x5554,
    0xa554,
    0xfd54,
    0x07f4,
    0x57f4,
    0xa7f4,
    0xfff4,
    0x001f,
    0x501f,
    0xa01f,
    0xf81f,
    0x02bf,
    0x52bf,
    0xa2bf,
    0xfabf,
    0x055f,
    0x555f,
    0xa55f,
    0xfd5f,
    0x07ff,
    0x57ff,
    0xa7ff,
    0xffff,
};
#endif

void draw_line(u8 y, u8 *line)
{
  color lineBuffer[256];
  for (int x = 0; x < 256; x++)
  {
    u8 clr = line[x];
    lineBuffer[x] = conversion_table[clr];
  }

#ifdef TARGET_LINUX
  memcpy(vdp->framebuffer + y * 256, lineBuffer, sizeof(lineBuffer));
#else
// eadk_display_push_rect((eadk_rect_t){0, y, 256, 1}, lineBuffer);
#endif
}

void vdp_process_controlwrite(u8 byte)
{
  if (!waitingForWrite)
  {
    fullCommand &= 0xFF00;
    fullCommand |= byte;
    waitingForWrite = true;
  }
  else
  {
    fullCommand &= 0xFF;
    fullCommand |= (byte << 8);
    u8 controlCode = fullCommand >> 14;
    switch (controlCode)
    {
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

      break;
    case 3:
      writeCram = true;
    }
    waitingForWrite = false;
  }
}

void vdp_process_datawrite(u8 byte)
{
  readBuffer = byte;
  if (writeCram)
    vdp->cram[address_register() & 0x1F] = byte;
  else
    vdp->vram[address_register()] = byte;
  increment();
}

void vdp_write_io(u8 port, u8 value)
{
  if (port == 0xBE)
    vdp_process_datawrite(value);
  if (port == 0xBF)
    vdp_process_controlwrite(value);
}

u16 get_sprite_address_table()
{
  return (vdp->registers[5] & 0x7E) << 7;
}

u16 get_color(int index, bool sprite)
{
  return vdp->cram[16 * sprite + index];
}

u16 get_name_table()
{
  return (vdp->registers[2] & 0xE) << 10;
}

bool vdp_is_interrupt()
{
  bool a = vdp->lineInterrupt && get_bit(vdp->registers[0], 4);
  bool b = vdp->frameInterrupt && get_bit(vdp->registers[1], 5);

  return a || b;
}

void process_line()
{
  if (!get_bit(vdp->registers[1], 6))
  {
    return;
  }
  int y = vdp->realVcounter;
  u8 lineState[256] = {0};
  u8 lineBuffer[256] = {0};

  // render background
  u16 addr = get_name_table();
  u16 *nameTable = (u16 *)&vdp->vram[addr];
  int line = y / 8;
  int yInLine = y % 8;

  int startingColumn = (32 - (vdp->registers[8] >> 3)) % 32;
  int fineScroll = vdp->registers[8] & 7;
  bool debug = IsKeyPressed(KEY_SPACE);

  for (int i = 0; i < 32; i++)
  {
    int col = (startingColumn + i) % 32;
    u16 tileInfo = nameTable[col + line * 32];
    u16 patternIndex = tileInfo & 0x1FF;
    bool priorityFlag = (tileInfo & 0x1000) != 0;
    bool paletteSelect = (tileInfo & 0x800) != 0;
    bool verticalFlip = (tileInfo & 0x400) != 0;
    bool horizontalFlip = (tileInfo & 0x200) != 0;

    u8 *pixelsLine = (&vdp->vram[32 * patternIndex] + 4 * yInLine);
    u8 b1 = pixelsLine[3];
    u8 b2 = pixelsLine[2];
    u8 b3 = pixelsLine[1];
    u8 b4 = pixelsLine[0];

    int xStart = i*8 + fineScroll;

    for (int x = 0; x < 8; x++)
    {
      int paletteIndex = (get_bit(b1, 7 - x) << 3) + (get_bit(b2, 7 - x) << 2) + (get_bit(b3, 7 - x) << 1) + get_bit(b4, 7 - x);
      u8 clr = get_color(paletteIndex, paletteSelect);
      
      int xPos = (xStart + x) % 256;
      lineState[xPos] = priorityFlag * 2;
      lineBuffer[xPos] = clr;
    }
  }

  // render sprites
  u8 *satBase = &vdp->vram[get_sprite_address_table()];
  for (int i = 0; i < 64; i++)
  {
    if (satBase[i] == 0xD0)
      break;
    u8 spriteY = satBase[i] + 1;
    s16 spriteX = *(satBase + 128 + 2 * i);
    if (get_bit(vdp->registers[0], 3))
      spriteX -= 8;

    int patternIndex = *(satBase + 128 + 2 * i + 1);
    if (get_bit(vdp->registers[6], 2))
      patternIndex += 256;

    if (!(y >= spriteY && y < spriteY + 8))
      continue; // skip sprites that are not in scanline

    int lineInSprite = y - spriteY;
    u8 *pixelsLine = (&vdp->vram[32 * patternIndex] + 4 * lineInSprite);
    u8 b1 = pixelsLine[3];
    u8 b2 = pixelsLine[2];
    u8 b3 = pixelsLine[1];
    u8 b4 = pixelsLine[0];

    for (int x = 0; x < 8; x++)
    {
      if (lineState[spriteX + x] == 1)
      {
        vdp->statusReg = vdp->statusReg | 16;
        continue;
      }
      if (lineState[spriteX + x] == 2)
        continue;
      int paletteIndex = (get_bit(b1, 7 - x) << 3) + (get_bit(b2, 7 - x) << 2) + (get_bit(b3, 7 - x) << 1) + get_bit(b4, 7 - x);
      if (paletteIndex == 0)
        continue;
      u8 clr = get_color(paletteIndex, true);

      lineState[spriteX + x] = 1;
      lineBuffer[spriteX + x] = clr;
    }
  }

  // draw overscan
  u8 clr = get_color(vdp->registers[7] & 0xF, true);
  for (int i = 0; i < 8; i++)
    lineBuffer[i] = clr;
  draw_line(y, lineBuffer);
}

u8 get_vreset() { return 218; };
u8 get_vresetdest() { return 212; };
bool needsReset = true;

void vdp_update(float vdpcycles)
{
  u16 hcount = vdp->hcounter;
  int cycles = (int)vdpcycles;
  int hinc = cycles / 2;

  bool nextLine = hinc + hcount > 684;
  vdp->hcounter = (hcount + hinc) % 685;

  if (nextLine)
  {
    u16 realVcounter = vdp->realVcounter;

    if (realVcounter < 192)
    {
      process_line();
    }
    else
    {
      vdp->vscroll = vdp->registers[9];
    }

    if (realVcounter <= 192)
    {
      vdp->lineCounter--;
      if (vdp->lineCounter == 255)
      {
        vdp->lineCounter = vdp->registers[10];
        vdp->lineInterrupt = true;
      }
    }
    else
      vdp->lineCounter = vdp->registers[10];

    if (realVcounter == 192)
    {
      vdp->statusReg |= 0x80;
      vdp->frameInterrupt = true;
    }

    if (vdp->realVcounter == 218)
      vdp->vcounter = 212;

    if (vdp->realVcounter == 261)
    {
      vdp->vcounter = 0;
      vdp->realVcounter = 0;
    }
    else
    {
      vdp->vcounter++;
      vdp->realVcounter++;
    }
  }
}