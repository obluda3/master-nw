#include "types.h"

typedef union {
  struct {
    u8 A;
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
    u8 B;
    u8 C;
    u8 D;
    u8 E;
    u8 H;
    u8 L;
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
} Z80;

u8 execute_cpu(Z80* cpu); // return cycle number
