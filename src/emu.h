#pragma once
#include "types.h"
#include "mem.h"
#include "vdp.h"

typedef union {
  struct {
    struct { 
      bool c : 1;
      bool n : 1;
      bool pv : 1;
      bool b3 : 1;
      bool h : 1;
      bool b5 : 1;
      bool z : 1;
      bool s : 1;
    } F;
    u8 A;
    u8 C;
    u8 B;
    u8 E;
    u8 D;
    u8 L;
    u8 H;
  } singles;
  struct {
    u16 AF;
    u16 BC;
    u16 DE;
    u16 HL;
  } pairs;
} Registers;

typedef struct {
  Registers main;
  Registers alt;
  u16 IX;
  u16 IY;
  u16 SP;
  u8 I;
  u8 R;
  u16 PC;
  bool FF1;
  bool FF2;
} Z80;

typedef struct {
  Z80 cpu;
  Memory mem;
  VDP vdp;
} Emu;

void emu_loop(Emu* emu);
void pause();
void check_interrupts(Emu* emu);