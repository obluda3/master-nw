#include "cpu.h"
#include <string.h>
#include <stdio.h>
#include "io.h"
#include "mem.h"

#define INLINED __attribute__((always_inline)) inline

Z80* cpu;
int bonus_cycles = 0;

int execute_cb();
int execute_displacedcb(u16* reg);

const u8 cycles[] = {
    0x04, 0x0A, 0x07, 0x06, 0x04, 0x04, 0x07, 0x04, 0x04, 0x0B, 0x07, 0x06, 0x04, 0x04, 0x07, 0x04,
    0x08, 0x0A, 0x07, 0x06, 0x04, 0x04, 0x07, 0x04, 0x0C, 0x0B, 0x07, 0x06, 0x04, 0x04, 0x07, 0x04,
    0x07, 0x0A, 0x10, 0x06, 0x04, 0x04, 0x07, 0x04, 0x07, 0x0B, 0x10, 0x06, 0x04, 0x04, 0x07, 0x04,
    0x07, 0x0A, 0x0D, 0x06, 0x0B, 0x0B, 0x0A, 0x04, 0x07, 0x0B, 0x0D, 0x06, 0x04, 0x04, 0x07, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
    0x05, 0x0A, 0x0A, 0x0A, 0x0A, 0x0B, 0x07, 0x0B, 0x05, 0x0A, 0x0A, 0x00, 0x0A, 0x11, 0x07, 0x0B,
    0x05, 0x0A, 0x0A, 0x0B, 0x0A, 0x0B, 0x07, 0x0B, 0x05, 0x04, 0x0A, 0x0B, 0x0A, 0x00, 0x07, 0x0B,
    0x05, 0x0A, 0x0A, 0x13, 0x0A, 0x0B, 0x07, 0x0B, 0x05, 0x04, 0x0A, 0x04, 0x0A, 0x00, 0x07, 0x0B,
    0x05, 0x0A, 0x0A, 0x04, 0x0A, 0x0B, 0x07, 0x0B, 0x05, 0x06, 0x0A, 0x04, 0x0A, 0x00, 0x07, 0x0B,
};

const u8 cycles_ixiy[] = { 
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x0E, 0x14, 0x0A, 0x08, 0x08, 0x0B, 0x04, 0x04, 0x0F, 0x14, 0x0A, 0x08, 0x08, 0x0B, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x17, 0x17, 0x13, 0x04, 0x04, 0x0F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x13, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x13, 0x08,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x04, 0x13, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x13, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x0E, 0x04, 0x17, 0x04, 0x0F, 0x04, 0x04, 0x04, 0x08, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0A, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
};

const u8 cycles_cb[] = { 
  8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,8,
  8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,8,
  8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,8,
  8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,8,
  8,8,8,8,8,12,8,8,8,8,8,8,8,12,8,8,
  8,8,8,8,8,12,8,8,8,8,8,8,8,12,8,8,
  8,8,8,8,8,12,8,8,8,8,8,8,8,12,8,8,
  8,8,8,8,8,12,8,8,8,8,8,8,8,12,8,8,
  8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,8,
  8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,8,
  8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,8,
  8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,8,
  8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,8,
  8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,8,
  8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,8,
  8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,
};

const u8 cycles_ed[] = {
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x0C, 0x0C, 0x0F, 0x14, 0x08, 0x0E, 0x08, 0x09, 0x0C, 0x0C, 0x0F, 0x14, 0x08, 0x0E, 0x08, 0x09,
  0x0C, 0x0C, 0x0F, 0x14, 0x08, 0x0E, 0x08, 0x09, 0x0C, 0x0C, 0x0F, 0x14, 0x08, 0x0E, 0x08, 0x09,
  0x0C, 0x0C, 0x0F, 0x14, 0x08, 0x0E, 0x08, 0x12, 0x0C, 0x0C, 0x0F, 0x14, 0x08, 0x0E, 0x08, 0x12,
  0x0C, 0x0C, 0x0F, 0x14, 0x08, 0x0E, 0x08, 0x08, 0x0C, 0x0C, 0x0F, 0x14, 0x08, 0x0E, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x10, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x08,
  0x10, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};

INLINED bool parity(u8 value) {
  value ^= value >> 4;
  value &= 0xF;
  return !((0x6996 >> value) & 1);
}

INLINED void incr_r() {
  cpu->R++;
  cpu->R = cpu->R & 0x7F;
}

void init_cpu(Z80* CPU) {
  cpu = CPU;
  memset(cpu, 0, sizeof(Z80));
}
int opcode_cycles[] = {};

INLINED bool overflow_flag8(u8 reg1, u8 reg2, u8 res) {
  return ((reg1 & 0x80) == (reg2 & 0x80)) && ((reg1 & 0x80) != (res & 0x80));
}

INLINED void SBC_R8_R8(u8* reg, u8 reg2) {
  u8 reg1 = *reg;
  u8 c = cpu->main.singles.F.c;
  s16 result = reg1 - reg2 - c;

  u8 newVal = result & 0xFF;
  *reg = newVal;

  cpu->main.singles.F.c = result < 0;
  cpu->main.singles.F.n = 1;
  cpu->main.singles.F.pv = (((reg1 ^ reg2) & (reg1 ^ result)) & 0x80) != 0;
  cpu->main.singles.F.h = ((reg1 ^ reg2 ^ result) & 0x10) != 0;
  cpu->main.singles.F.z = newVal == 0;
  cpu->main.singles.F.s = (newVal & 0x80) != 0;
  cpu->main.singles.F.b3 = (newVal & 0x8) >> 3;
  cpu->main.singles.F.b5 = (newVal & 0x20) >> 5;
}

INLINED void SBC_R8_N(u8* reg) {
  SBC_R8_R8(reg, read_u8(cpu->PC));
  cpu->PC += 1;
}

INLINED void update_flags_bitwise(u8 result) {
  cpu->main.singles.F.c = 0;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = parity(result);
  cpu->main.singles.F.h = 0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = (result & 0x80) != 0;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
}

INLINED void AND_R8_R8(u8* reg, u8 reg2) {
  u8 result = *reg & reg2;
  *reg = result;
  update_flags_bitwise(result);
  cpu->main.singles.F.h = 1;
}

INLINED void AND_R8_N(u8* reg) {
  AND_R8_R8(reg, read_u8(cpu->PC));
  cpu->PC += 1;
}

INLINED void XOR_R8_R8(u8* reg, u8 reg2) {
  u8 result = *reg ^ reg2;
  *reg = result;
  update_flags_bitwise(result);
}

INLINED void XOR_R8_N(u8* reg) {
  XOR_R8_R8(reg, read_u8(cpu->PC));
  cpu->PC += 1;
}

INLINED void OR_R8_R8(u8* reg, u8 reg2) {
  u8 result = *reg | reg2;
  *reg = result;
  update_flags_bitwise(result);
}

INLINED void OR_R8_N(u8* reg) {
  OR_R8_R8(reg, read_u8(cpu->PC));
  cpu->PC += 1;
}

u8 _SUB_8(u8 op1, u16 op2) {
  int res = op1 - op2;
  u8 result = (u8)res;
  
  cpu->main.singles.F.c = op2 > op1;
  cpu->main.singles.F.n = 1;
  cpu->main.singles.F.pv = ((op2 ^ op1) & (op1 ^ res) & 0x80) != 0;
  cpu->main.singles.F.h = (op1 & 0xF) < (op2 & 0xF);
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = (result & 0x80) != 0;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
  return result;
}

INLINED void SUB_R8_R8(u8* reg, u8 reg2) {
  *reg = _SUB_8(*reg, reg2);
}

INLINED void CP_R8_R8(u8* reg, u8 reg2) {
  u8 r = *reg;
  SUB_R8_R8(reg, reg2);
  cpu->main.singles.F.b3 = (reg2 & 0x8) >> 3;
  cpu->main.singles.F.b5 = (reg2 & 0x20) >> 5;
  *reg = r;
}

INLINED void CP_R8_N(u8* reg) {
  CP_R8_R8(reg, read_u8(cpu->PC));
  cpu->PC += 1;
}

INLINED void LD_R16_NN(u16* reg) {
  *reg = read_u16(cpu->PC);
  cpu->PC += 2;
}

INLINED void LD_R16_NNa(u16* reg) {
  *reg = read_u16(read_u16(cpu->PC));
  cpu->PC += 2;
}

INLINED void LD_R8_R8(u8* reg, u8 val) {
  *reg = val;
}

INLINED void LD_NN_R16(u16 addr, u16 reg) {
  write_u16(addr, reg);
  cpu->PC += 2;
}

INLINED void LD_NN_R8(u8 reg) {
  write_u8(read_u16(cpu->PC), reg);
  cpu->PC += 2;
}

INLINED void LD_R8_N(u8* reg) {
  *reg = read_u8(cpu->PC);
  cpu->PC += 1;
}

INLINED void LD_R8_NN(u8* reg) {
  *reg = read_u8(read_u16(cpu->PC));
  cpu->PC += 2;
}

INLINED void LD_R16_R8(u16 dest, u8 value) {
  write_u8(dest, value);
}

INLINED void LD_R8_R16(u8* dest, u16 addr) {
  *dest = read_u8(addr);
}

INLINED void ADD_R16_R16(u16* dest, u16* value) {
  u16 dst = *dest;
  u16 val = *value;
  u16 result = dst + val;
  *dest = result;
  u8 high = (result & 0xFF00) >> 8;
  cpu->main.singles.F.c = (dst + val) > 0xFFFF;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.b3 = (high & 0x8) >> 3;
  cpu->main.singles.F.b5 = (high & 0x20) >> 5;
  cpu->main.singles.F.h = (dst & 0xFFF) + (val & 0xFFF) > 0xFFF;
}

INLINED void INC_R16(u16* reg) {
  *reg += 1;
}

INLINED void INC_R16a(u16 addr) {
  u8 value = read_u8(addr);
  u8 result = value + 1;

  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = value == 0x7F;
  cpu->main.singles.F.h = (value & 0xF) == 0xF;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
  write_u8(addr, result);
}

INLINED void INC_R8(u8* reg) {
  u8 value = *reg;
  u8 result = value + 1;
  *reg = result;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = value == 0x7F;
  cpu->main.singles.F.h = (value & 0xF) == 0xF;
  cpu->main.singles.F.z = value == 0xFF;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
  cpu->main.singles.F.s = result >> 7;
}

INLINED void ADD_R8_R8(u8* reg, u8 reg2) {
  u8 reg1 = *reg;
  u16 result = reg1 + reg2;
  u8 newVal = result & 0xFF;
  *reg = newVal;
  cpu->main.singles.F.c = result > 0xFF;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = overflow_flag8(reg1, reg2, newVal);
  cpu->main.singles.F.h = ((reg1 & 0xF) + (reg2 & 0xF)) > 0xF;
  cpu->main.singles.F.z = newVal == 0;
  cpu->main.singles.F.b3 = (newVal & 0x8) >> 3;
  cpu->main.singles.F.b5 = (newVal & 0x20) >> 5;
  cpu->main.singles.F.s = (newVal & 0x80) != 0;
}

INLINED void ADD_R8_N(u8* reg) {
  ADD_R8_R8(reg, read_u8(cpu->PC));
  cpu->PC += 1;
}

INLINED void ADC_R8_R8(u8* reg, u8 reg2) {
  u8 reg1 = *reg;
  u8 c = cpu->main.singles.F.c;
  u16 result = reg1 + reg2 + c;
  u8 newVal = result & 0xFF;
  *reg = newVal;
  cpu->main.singles.F.c = result > 0xFF;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = overflow_flag8(reg1, reg2, newVal);
  cpu->main.singles.F.h = ((reg1 & 0xF) + (reg2 & 0xF) + c) > 0xF;
  cpu->main.singles.F.z = newVal == 0;
  cpu->main.singles.F.s = (newVal & 0x80) != 0;
  cpu->main.singles.F.b3 = (newVal & 0x8) >> 3;
  cpu->main.singles.F.b5 = (newVal & 0x20) >> 5;
}

INLINED void ADC_R8_N(u8* reg) {
  ADC_R8_R8(reg, read_u8(cpu->PC));
  cpu->PC += 1;
}

INLINED void ADC_R16_R16(u16* reg1, u16 reg2) {
  int val = reg2 + cpu->main.singles.F.c;
  u16 result = *reg1 + val;

  cpu->main.singles.F.c = (val + *reg1) > 0xFFFF;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = ((reg2 ^ *reg1 ^ 0x8000) & (reg2 ^ result) & 0x8000);
  cpu->main.singles.F.h = ((*reg1 & 0xFFF) + (val & 0xFFF)) > 0xFFF;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 15;
  cpu->main.singles.F.b3 = ((result >> 8) & 0x8) >> 3;
  cpu->main.singles.F.b5 = ((result >> 8) & 0x20) >> 5;

  *reg1 = result;
}

INLINED void SUB_R8_N(u8* reg) {
  SUB_R8_R8(reg, read_u8(cpu->PC));
  cpu->PC += 1;
}

INLINED void SBC_R16_R16(u16* reg1, u16 reg2) {
  int val = reg2 + cpu->main.singles.F.c;
  u16 result = *reg1 - val;

  cpu->main.singles.F.c = val > *reg1;
  cpu->main.singles.F.n = 1;
  cpu->main.singles.F.pv = (((reg2 ^ *reg1) & (*reg1 ^ result) & 0x8000)) >> 15;
  cpu->main.singles.F.h = (*reg1 & 0xFFF) < (val & 0xFFF);
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 15;

  *reg1 = result;
}


INLINED void DEC_R8(u8* reg) {
  u8 value = *reg;
  u8 result = value - 1;
  *reg = result;
  cpu->main.singles.F.n = 1;
  cpu->main.singles.F.pv = value == 0x80;
  cpu->main.singles.F.h = (value & 0xF) == 0x0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
}

INLINED void DEC_R16a(u16 addr) {
  u8 value = read_u8(addr);
  u8 result = value - 1;
  write_u8(addr, result);
  cpu->main.singles.F.n = 1;
  cpu->main.singles.F.pv = value == 0x80;
  cpu->main.singles.F.h = (value & 0xF) == 0x0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
}

INLINED void DEC_R16(u16* reg) {
  *reg -= 1;
}

INLINED void JR() {
  s8 offset = (s8)read_u8(cpu->PC);
  cpu->PC += 1 + offset;
}

INLINED void JR_cond(bool condition) {
  s8 offset = (s8)read_u8(cpu->PC);
  cpu->PC += 1;
  if (condition) {
    cpu->PC += offset;
    bonus_cycles = 5;
  }
}

INLINED u16 POP() {
  u16 result = read_u16(cpu->SP);
  cpu->SP += 2;
  return result;  
}

INLINED void PUSH(u16 value) {
  cpu->SP -= 2;
  write_u16(cpu->SP, value);
}

INLINED void RST(u8 addr) {
  PUSH(cpu->PC);
  cpu->PC = addr;
}

INLINED void RET() {
  cpu->PC = POP(); 
}

INLINED void RET_cond(bool condition) {
  if (condition) {
    bonus_cycles = 6;
    RET();
  }
}

INLINED void JP() {
  cpu->PC = read_u16(cpu->PC);
}

INLINED void JP_cond(bool condition) {
  if (condition) JP();
  else cpu->PC += 2;
}

INLINED void CALL() {
  PUSH(cpu->PC+2);
  cpu->PC = read_u16(cpu->PC);
}

INLINED void CALL_cond(bool condition) {
  if (condition) {
    bonus_cycles = 7;
    CALL();
  }
  else cpu->PC += 2;
}

INLINED void EI() {
  cpu->FF1 = true;
  cpu->FF2 = true;
}

INLINED void DI() {
  cpu->FF1 = false;
  cpu->FF2 = false;
}

int call_cnt = 0;
int execute_ddfd(bool dd) {
  u16* reg = dd ? &cpu->IX : &cpu->IY;
  u8 inst = read_u8(cpu->PC);
  incr_r();
  cpu->PC++;
  #define ADDR() *reg + (s8)read_u8(cpu->PC++)
  switch (inst) {
    case 0x09:  // add ixiy, bc
      ADD_R16_R16(reg, &cpu->main.pairs.BC);
      break;
    case 0x19:  // add ixiy, de
      ADD_R16_R16(reg, &cpu->main.pairs.DE);
      break;
    case 0x21:  // ld ixiy, nn
      LD_R16_NN(reg);
      break;
    case 0x22:  // ld (nn), ixiy
      LD_NN_R16(read_u16(cpu->PC), *reg);
      break;
    case 0x23:  // inc ixiy
      INC_R16(reg);
      break;
    case 0x29:  // add ixiy, ixiy
      ADD_R16_R16(reg, reg);
      break;
    case 0x2A:  { // ld ix, (nn) 
      call_cnt += 1;
      if (call_cnt == 34) {
        LD_R16_NN(reg);
      }
      else {
        LD_R16_NNa(reg);
      }
      break;
    }
    case 0x2B:  // dec ixiy
      DEC_R16(reg);
      break;
    case 0x34:  // inc (ixiy + d)
      INC_R16a(ADDR());
      break;
    case 0x35:  // dec (ixiy + d)
      DEC_R16a(ADDR());
      break;
    case 0x36: { // ld (ixiy + d), n
      u16 addr = ADDR();
      u8 val = read_u8(cpu->PC);
      write_u8(addr, val);
      cpu->PC++;
      break;
    }
    case 0x39:  // add ixiy, sp
      ADD_R16_R16(reg, &cpu->SP);
      break;
    case 0x46:  // ld b, (ixiy + d)
      LD_R8_R16(&cpu->main.singles.B, ADDR());
      break;
    case 0x4E:  // ld c, (ixiy + d)
      LD_R8_R16(&cpu->main.singles.C, ADDR());
      break;
    case 0x56:  // ld d, (ixiy + d)
      LD_R8_R16(&cpu->main.singles.D, ADDR());
      break;
    case 0x5E:  // ld e, (ixiy + d)
      LD_R8_R16(&cpu->main.singles.E, ADDR());
      break;
    case 0x66:  // ld h, (ixiy + d)
      LD_R8_R16(&cpu->main.singles.H, ADDR());
      break;
    case 0x6E:  // ld l, (ixiy + d)
      LD_R8_R16(&cpu->main.singles.L, ADDR());
      break;
    case 0x70:  // ld (ix+d), b
      LD_R16_R8(ADDR(), cpu->main.singles.B);
      break;
    case 0x71:  // ld (ix+d), c
      LD_R16_R8(ADDR(), cpu->main.singles.C);
      break;
    case 0x72:  // ld (ix+d), d
      LD_R16_R8(ADDR(), cpu->main.singles.D);
      break;
    case 0x73:  // ld (ix+d), e
      LD_R16_R8(ADDR(), cpu->main.singles.E);
      break;
    case 0x74:  // ld (ix+d), h
      LD_R16_R8(ADDR(), cpu->main.singles.H);
      break;
    case 0x75:  // ld (ix+d), l
      LD_R16_R8(ADDR(), cpu->main.singles.L);
      break;
    case 0x77:  // ld (ix+d), a
      LD_R16_R8(ADDR(), cpu->main.singles.A);
      break;
    case 0x7E:  // ld a, (ix+d)
      LD_R8_R16(&cpu->main.singles.A, ADDR());
      break;
    case 0x86:  // add a, (ix+d)
      ADD_R8_R8(&cpu->main.singles.A, read_u8(ADDR()));
      break;
    case 0x8E:  // adc a, (ix+d)
      ADC_R8_R8(&cpu->main.singles.A, read_u8(ADDR()));
      break;
    case 0x96:  // sub (ix+d)
      SUB_R8_R8(&cpu->main.singles.A, read_u8(ADDR()));
      break;
    case 0x9E:  // sbc a, (ix+d)
      SUB_R8_R8(&cpu->main.singles.A, read_u8(ADDR()));
      break;
    case 0xA6:  // and (ix+d)
      AND_R8_R8(&cpu->main.singles.A, read_u8(ADDR()));
      break;
    case 0xAE:  // xor (ix+d)
      XOR_R8_R8(&cpu->main.singles.A, read_u8(ADDR()));
      break;
    case 0xB6:  // or (ix+d)
      OR_R8_R8(&cpu->main.singles.A, read_u8(ADDR()));
      break;
    case 0xBE:  // cp (ix+d)
      CP_R8_R8(&cpu->main.singles.A, read_u8(ADDR()));
      break;
    case 0xCB:  // cb ix
      return execute_displacedcb(reg);
    case 0xE1:  // pop ix
      *reg = POP();
      break;
    case 0xE3:{  // ex (sp), iy
      u16 tmp = read_u16(cpu->SP);
      write_u16(cpu->SP, *reg);
      *reg = tmp;
      break;
    }
    case 0xE5:  // push ix
      PUSH(*reg);
      break;
    case 0xE9:  // jp (ix)
      cpu->PC = *reg;
      break;
    case 0xF9:  // ld sp, ix
      cpu->SP = *reg;
      break;
    default:
      printf("undocumented ddfd inst %02x\n", inst);
      break;
  }
  int result = cycles_ixiy[inst] + bonus_cycles;
  bonus_cycles = 0;
  return result;
}

INLINED void IN_R8_R8(u8* reg, u8 port) {
  u8 val = read_io(port);
  *reg = val;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = parity(val);
  cpu->main.singles.F.h = 0;
  cpu->main.singles.F.s = val >> 7;
  cpu->main.singles.F.z = val == 0;
}

int execute_ed() {
  u8 inst = read_u8(cpu->PC++);
  incr_r();
  switch (inst) {
    case 0x40:
      IN_R8_R8(&cpu->main.singles.B, cpu->main.singles.C);
      break;
    case 0x41:
      write_io(cpu->main.singles.C, cpu->main.singles.B);
      break;
    case 0x42:
      SBC_R16_R16(&cpu->main.pairs.HL, cpu->main.pairs.BC);
      break;
    case 0x43:
      LD_NN_R16(read_u16(cpu->PC), cpu->main.pairs.BC);
      break;
    case 0x44:
      cpu->main.singles.A = _SUB_8(0, cpu->main.singles.A);
      break;
    case 0x45:
      cpu->PC = POP();
      cpu->FF1 = cpu->FF2;
      break;
    case 0x47:
      LD_R8_R8(&cpu->I, cpu->main.singles.A);
      break;
    case 0x48:
      IN_R8_R8(&cpu->main.singles.C, cpu->main.singles.C);
      break;
    case 0x49:
      write_io(cpu->main.singles.C, cpu->main.singles.C);
      break;
    case 0x4A:
      ADC_R16_R16(&cpu->main.pairs.HL, cpu->main.pairs.BC);
      break;
    case 0x4B:
      LD_R16_NNa(&cpu->main.pairs.BC);
      break;
    case 0x4D:
      cpu->PC = POP();
      cpu->FF1 = 1;
      break;
    case 0x4F:
      LD_R8_R8(&cpu->R, cpu->main.singles.A);
      break;
    case 0x50:
      IN_R8_R8(&cpu->main.singles.D, cpu->main.singles.C);
      break;
    case 0x51:
      write_io(cpu->main.singles.C, cpu->main.singles.D);
      break;
    case 0x52:
      SBC_R16_R16(&cpu->main.pairs.HL, cpu->main.pairs.DE);
      break;
    case 0x53:
      LD_NN_R16(read_u16(cpu->PC), cpu->main.pairs.DE);
      break;
    case 0x57:
      LD_R8_R8(&cpu->main.singles.A, cpu->I);
      break;
    case 0x58:
      IN_R8_R8(&cpu->main.singles.E, cpu->main.singles.C);
      break;
    case 0x59:
      write_io(cpu->main.singles.C, cpu->main.singles.E);
      break;
    case 0x5A:
      ADC_R16_R16(&cpu->main.pairs.HL, cpu->main.pairs.BC);
      break;
    case 0x5B:
      LD_R16_NNa(&cpu->main.pairs.DE);
      break;
    case 0x5F:
      LD_R8_R8(&cpu->main.singles.A, cpu->R);
      break;
    case 0x60:
      IN_R8_R8(&cpu->main.singles.H, cpu->main.singles.C);
      break;
    case 0x61:
      write_io(cpu->main.singles.C, cpu->main.singles.H);
      break;
    case 0x62:
      SBC_R16_R16(&cpu->main.pairs.HL, cpu->main.pairs.HL);
      break;
    case 0x67: {
      u8 val = read_u8(cpu->main.pairs.HL);
      write_u8(cpu->main.pairs.HL, (cpu->main.singles.A << 4) | (val >> 4));
      u8 result = (cpu->main.singles.A & 0xF0) | (val & 0x0F);
      cpu->main.singles.A = result;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.pv = parity(result);
      cpu->main.singles.F.z = result == 0;
      cpu->main.singles.F.s = result >> 7;
      break; 
    }
    case 0x68:
      IN_R8_R8(&cpu->main.singles.L, cpu->main.singles.C);
      break;
    case 0x69:
      write_io(cpu->main.singles.C, cpu->main.singles.L);
      break;
    case 0x6A:
      ADC_R16_R16(&cpu->main.pairs.HL, cpu->main.pairs.HL);
      break;
    case 0x6F: {
      u8 val = read_u8(cpu->main.pairs.HL);
      write_u8(cpu->main.pairs.HL, (cpu->main.singles.A & 0x0F) | (val << 4));
      u8 result = (cpu->main.singles.A & 0xF0) | (val >> 4);
      cpu->main.singles.A = result;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.pv = parity(result);
      cpu->main.singles.F.z = result == 0;
      cpu->main.singles.F.s = result >> 7;
      break; 
    }
    case 0x70: {
      u8 result = 0;
      IN_R8_R8(&result, cpu->main.singles.C);
      break;
    }
    case 0x71:
      write_io(cpu->main.singles.C, 0);
      break;
    case 0x72:
      SBC_R16_R16(&cpu->main.pairs.HL, cpu->SP);
      break;
    case 0x73:
      LD_NN_R16(read_u16(cpu->PC), cpu->SP);
      break;
    case 0x78:
      IN_R8_R8(&cpu->main.singles.A, cpu->main.singles.C);
      break;
    case 0x79:
      write_io(cpu->main.singles.C, cpu->main.singles.A);
      break;
    case 0x7A:
      ADC_R16_R16(&cpu->main.pairs.HL, cpu->SP);
      break;
    case 0x7B:
      LD_R16_NNa(&cpu->SP);
      break;
    case 0xA0: {
      u8 writtenByte = read_u8(cpu->main.pairs.HL);
      write_u8(cpu->main.pairs.DE, writtenByte);
      cpu->main.pairs.HL++;
      cpu->main.pairs.DE++;
      cpu->main.pairs.BC--;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      cpu->main.singles.F.pv = cpu->main.pairs.BC != 0;
      cpu->main.singles.F.b3 = ((cpu->main.singles.A + writtenByte) & 0x8) >> 3;
      cpu->main.singles.F.b5 = ((cpu->main.singles.A + writtenByte) & 0x2) >> 1;
      break;
    }
    case 0xA1: {
      u8 val = read_u8(cpu->main.pairs.HL);
      u8 result = cpu->main.singles.A - val;
      cpu->main.singles.F.h = (cpu->main.singles.A & 0xF) < (val & 0x0F);
      cpu->main.singles.F.z = result == 0;
      cpu->main.singles.F.s = result >> 7;
      cpu->main.singles.F.n = 1;
      result = result - cpu->main.singles.F.h;
      cpu->main.singles.F.b3 = (result & 0x8) >> 3;
      cpu->main.singles.F.b5 = (result & 0x2) >> 1;
      cpu->main.pairs.HL++;
      cpu->main.pairs.BC--;
      cpu->main.singles.F.pv = cpu->main.pairs.BC != 0;
      break;
    }
    case 0xA2: {
      write_u8(cpu->main.pairs.HL, read_io(cpu->main.singles.C));
      cpu->main.pairs.HL++;
      DEC_R8(&cpu->main.singles.B);
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.z = cpu->main.singles.B == 0;
      break;
    }
    case 0xA3:
      DEC_R8(&cpu->main.singles.B);
      write_io(cpu->main.singles.C, read_u8(cpu->main.pairs.HL));
      cpu->main.pairs.HL++;
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.z = cpu->main.singles.B == 0;
      break;
    case 0xA8: {
      u8 writtenByte = read_u8(cpu->main.pairs.HL);
      write_u8(cpu->main.pairs.DE, writtenByte);
      cpu->main.pairs.HL--;
      cpu->main.pairs.DE--;
      cpu->main.pairs.BC--;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      cpu->main.singles.F.pv = cpu->main.pairs.BC != 0;
      cpu->main.singles.F.b3 = ((cpu->main.singles.A + writtenByte) & 0x8) >> 3;
      cpu->main.singles.F.b5 = ((cpu->main.singles.A + writtenByte) & 0x2) >> 1;
      break;
    }
    case 0xA9: {
      u8 val = read_u8(cpu->main.pairs.HL);
      u8 result = cpu->main.singles.A - val;
      cpu->main.singles.F.h = (cpu->main.singles.A & 0xF) < (val & 0x0F);
      cpu->main.singles.F.z = result == 0;
      cpu->main.singles.F.s = result >> 7;
      result = result - cpu->main.singles.F.h;
      cpu->main.singles.F.b3 = (result & 0x8) >> 3;
      cpu->main.singles.F.b5 = (result & 0x2) >> 1;
      cpu->main.pairs.HL--;
      cpu->main.pairs.BC--;
      cpu->main.singles.F.pv = cpu->main.pairs.BC != 0;
      break;
    }
    case 0xAA:
      write_u8(cpu->main.pairs.HL, read_io(cpu->main.singles.C));
      cpu->main.pairs.HL--;
      DEC_R8(&cpu->main.singles.B);
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.z = cpu->main.singles.B == 0;
      break;
    case 0xAB:
      DEC_R8(&cpu->main.singles.B);
      write_io(cpu->main.singles.C, read_u8(cpu->main.pairs.HL));
      cpu->main.pairs.HL--;
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.z = cpu->main.singles.B == 0;
      break;
    case 0xB0: {
      u8 writtenByte = read_u8(cpu->main.pairs.HL);
      write_u8(cpu->main.pairs.DE, writtenByte);
      cpu->main.pairs.HL++;
      cpu->main.pairs.DE++;
      cpu->main.pairs.BC--;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      cpu->main.singles.F.pv = cpu->main.pairs.BC != 0;
      cpu->main.singles.F.b3 = ((cpu->main.singles.A + writtenByte) & 0x8) >> 3;
      cpu->main.singles.F.b5 = ((cpu->main.singles.A + writtenByte) & 0x2) >> 1;
      if (cpu->main.pairs.BC != 0) {
        bonus_cycles = 5;
        cpu->PC -= 2;
      }
      break;
    }
    case 0xB1: {
      u8 val = read_u8(cpu->main.pairs.HL);
      u8 result = cpu->main.singles.A - val;
      cpu->main.singles.F.h = (cpu->main.singles.A & 0xF) < (val & 0x0F);
      cpu->main.singles.F.z = result == 0;
      cpu->main.singles.F.s = result >> 7;
      cpu->main.singles.F.n = 1;
      result = result - cpu->main.singles.F.h;
      cpu->main.singles.F.b3 = (result & 0x8) >> 3;
      cpu->main.singles.F.b5 = (result & 0x2) >> 1;
      cpu->main.pairs.HL++;
      cpu->main.pairs.BC--;
      cpu->main.singles.F.pv = cpu->main.pairs.BC != 0;
      if (cpu->main.pairs.BC != 0 && cpu->main.singles.F.z != 1) {
        bonus_cycles = 5;
        cpu->PC -= 2;
      }
      break;
    }
    case 0xB2:
      write_u8(cpu->main.pairs.HL, read_io(cpu->main.singles.C));
      cpu->main.pairs.HL++;
      DEC_R8(&cpu->main.singles.B);
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.z = cpu->main.singles.B == 0;
      if (cpu->main.singles.B != 0) {
        bonus_cycles = 5;
        cpu->PC -= 2;
      }
      break;
    case 0xB3:
      DEC_R8(&cpu->main.singles.B);
      write_io(cpu->main.singles.C, read_u8(cpu->main.pairs.HL));
      cpu->main.pairs.HL++;
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.z = cpu->main.singles.B == 0;
      if (cpu->main.singles.B != 0) {
        bonus_cycles = 5;
        cpu->PC -= 2;
      }
      break;
    case 0xB8: {
      u8 writtenByte = read_u8(cpu->main.pairs.HL);
      write_u8(cpu->main.pairs.DE, writtenByte);
      cpu->main.pairs.HL--;
      cpu->main.pairs.DE--;
      cpu->main.pairs.BC--;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      cpu->main.singles.F.pv = cpu->main.pairs.BC != 0;
      cpu->main.singles.F.b3 = ((cpu->main.singles.A + writtenByte) & 0x8) >> 3;
      cpu->main.singles.F.b5 = ((cpu->main.singles.A + writtenByte) & 0x2) >> 1;
      if (cpu->main.pairs.BC != 0) {
        bonus_cycles = 5;
        cpu->PC -= 2;
      }
      break;
    }
    case 0xB9: {
      u8 val = read_u8(cpu->main.pairs.HL);
      u8 result = cpu->main.singles.A - val;
      cpu->main.singles.F.h = (cpu->main.singles.A & 0xF) < (val & 0x0F);
      cpu->main.singles.F.z = result == 0;
      cpu->main.singles.F.s = result >> 7;
      cpu->main.singles.F.n = 1;
      result = result - cpu->main.singles.F.h;
      cpu->main.singles.F.b3 = (result & 0x8) >> 3;
      cpu->main.singles.F.b5 = (result & 0x2) >> 1;
      cpu->main.pairs.HL--;
      cpu->main.pairs.BC--;
      cpu->main.singles.F.pv = cpu->main.pairs.BC != 0;
      if (cpu->main.pairs.BC != 0 && cpu->main.singles.F.z != 1) {
        bonus_cycles = 5;
        cpu->PC -= 2;
      }
      break;
    }
    case 0xBA:
      write_u8(cpu->main.pairs.HL, read_io(cpu->main.singles.C));
      cpu->main.pairs.HL--;
      DEC_R8(&cpu->main.singles.B);
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.z = cpu->main.singles.B == 0;
      if (cpu->main.singles.B != 0) {
        bonus_cycles = 5;
        cpu->PC -= 2;
      }
      break;
    case 0xBB:
      DEC_R8(&cpu->main.singles.B);
      write_io(cpu->main.singles.C, read_u8(cpu->main.pairs.HL));
      cpu->main.pairs.HL--;
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.z = cpu->main.singles.B == 0;
      if (cpu->main.singles.B != 0) {
        bonus_cycles = 5;
        cpu->PC -= 2;
      }
      break;
      
    default:
      printf("undocumented ed inst %02x\n", inst);
      break;


  }
  return cycles_ed[inst];
}

int execute_cpu(bool* halted) {
  u8 inst = read_u8(cpu->PC++);
  incr_r();
  switch (inst) {
    case 0x0:
      break;
    case 0x01:  // ld bc, nn
      LD_R16_NN(&cpu->main.pairs.BC);
      break;
    case 0x02:  // ld (bc), a
      LD_R16_R8(cpu->main.pairs.BC, cpu->main.singles.A);
      break;
    case 0x03:  // inc bc
      INC_R16(&cpu->main.pairs.BC);
      break;
    case 0x04:  // inc b
      INC_R8(&cpu->main.singles.B);
      break;
    case 0x05:  // dec b
      DEC_R8(&cpu->main.singles.B);
      break;
    case 0x06:  // ld b, n
      LD_R8_N(&cpu->main.singles.B);
      break;
    case 0x07: {  // rlca
      u8 a = cpu->main.singles.A;
      u8 c = a >> 7;
      u8 res = (a << 1) | c;
      cpu->main.singles.A = res;
      cpu->main.singles.F.c = c;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      cpu->main.singles.F.b3 = (res & 0x8) >> 3;
      cpu->main.singles.F.b5 = (res & 0x20) >> 5;
      break;
    }
    case 0x08: {  // ex af, af'
      u16 tmp = cpu->main.pairs.AF;
      cpu->main.pairs.AF = cpu->alt.pairs.AF;
      cpu->alt.pairs.AF = tmp;
      break;
    }
    case 0x09:  // add hl, bc
      ADD_R16_R16(&cpu->main.pairs.HL, &cpu->main.pairs.BC);
      break;
    case 0x0A:  // ld a,(bc)
      LD_R8_R16(&cpu->main.singles.A, cpu->main.pairs.BC);
      break;
    case 0x0B:  // dec bc
      DEC_R16(&cpu->main.pairs.BC);
      break;
    case 0x0C:  // inc c
      INC_R8(&cpu->main.singles.C);
      break;
    case 0x0D:  // dec c
      DEC_R8(&cpu->main.singles.C);
      break;
    case 0x0E:  // ld c, n
      LD_R8_N(&cpu->main.singles.C);
      break;
    case 0x0F: {  // rrca
      u8 a = cpu->main.singles.A;
      u8 c = a & 1;
      u8 res = a >> 1 | (c << 7);
      cpu->main.singles.A = res;
      cpu->main.singles.F.c = c;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      cpu->main.singles.F.b3 = (res & 0x8) >> 3;
      cpu->main.singles.F.b5 = (res & 0x20) >> 5;
      return 4;
    }
    case 0x10:  // djnz
      JR_cond(--cpu->main.singles.B != 0);
      break;
    case 0x11:  // ld de, nn
      LD_R16_NN(&cpu->main.pairs.DE);
      break;
    case 0x12:  // ld (de), a
      LD_R16_R8(cpu->main.pairs.DE, cpu->main.singles.A);
      break;
    case 0x13:  // inc de
      INC_R16(&cpu->main.pairs.DE);
      break;
    case 0x14:  // inc d
      INC_R8(&cpu->main.singles.D);
      break;
    case 0x15:  // dec d
      DEC_R8(&cpu->main.singles.D);
      break;
    case 0x16:  // ld d, n
      LD_R8_N(&cpu->main.singles.D);
      break;
    case 0x17:{  // rla
      u8 a = cpu->main.singles.A;
      u8 c = (a & 0x80) >> 7;
      a = a << 1 | cpu->main.singles.F.c;
      cpu->main.singles.A = a;
      cpu->main.singles.F.c = c;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      cpu->main.singles.F.b3 = (a & 0x8) >> 3;
      cpu->main.singles.F.b5 = (a & 0x20) >> 5;
      return 4;
    }
    case 0x18:  // jr d
      JR();
      break;
    case 0x19:  // add hl, de
      ADD_R16_R16(&cpu->main.pairs.HL, &cpu->main.pairs.DE);
      break;
    case 0x1A:  // ld a,(de)
      LD_R8_R16(&cpu->main.singles.A, cpu->main.pairs.DE);
      break;
    case 0x1B:  // dec de
      DEC_R16(&cpu->main.pairs.DE);
      break;
    case 0x1C:  // inc e
      INC_R8(&cpu->main.singles.E);
      break;
    case 0x1D:  // dec e
      DEC_R8(&cpu->main.singles.E);
      break;
    case 0x1E:  // ld e, n
      LD_R8_N(&cpu->main.singles.E);
      break;
    case 0x1F: {// rra
      u8 a = cpu->main.singles.A;
      u8 c = a & 1;
      cpu->main.singles.A = a >> 1 | (cpu->main.singles.F.c << 7);
      cpu->main.singles.F.c = c;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      cpu->main.singles.F.b3 = (cpu->main.singles.A & 0x8) >> 3;
      cpu->main.singles.F.b5 = (cpu->main.singles.A & 0x20) >> 5;
      return 4;
    }
    case 0x20:  // jr nz, d
      JR_cond(!cpu->main.singles.F.z);
      break;
    case 0x21:  // ld hl, nn
      LD_R16_NN(&cpu->main.pairs.HL);
      break;
    case 0x22:  // ld (nn), hl
      LD_NN_R16(read_u16(cpu->PC), cpu->main.pairs.HL);
      break;
    case 0x23:  // inc hl
      INC_R16(&cpu->main.pairs.HL);
      break;
    case 0x24:  // inc h
      INC_R8(&cpu->main.singles.H);
      break;
    case 0x25:  // dec h
      DEC_R8(&cpu->main.singles.H);
      break;
    case 0x26:  // ld h, n
      LD_R8_N(&cpu->main.singles.H);
      break;
    case 0x27: { // daa 
      // https://github.com/mamedev/mame/blob/master/src/devices/cpu/z80/z80.cpp
      u8 a = cpu->main.singles.A;
      if (cpu->main.singles.F.n) {
        if (cpu->main.singles.F.h || ((a & 0xF) > 9))
          a -= 6;
        if (cpu->main.singles.F.c || (cpu->main.singles.A > 0x99)) {
          a -= 0x60; 
          cpu->main.singles.F.c = 1;
        }
      }
      else {
        if (cpu->main.singles.F.h || (a & 0xF) > 0x09) 
          a += 6;
        if (cpu->main.singles.F.c || cpu->main.singles.A > 0x99) {
          a += 0x60;
          cpu->main.singles.F.c = 1;
        }
      }
      cpu->main.singles.A = a;
      cpu->main.singles.F.pv = parity(cpu->main.singles.A);
      cpu->main.singles.F.z = cpu->main.singles.A == 0;
      cpu->main.singles.F.s = cpu->main.singles.A >> 7;
      cpu->main.singles.F.b3 = (cpu->main.singles.A & 0x8) >> 3;
      cpu->main.singles.F.b5 = (cpu->main.singles.A & 0x20) >> 5;
      break;
    }
    case 0x28:  // jr z, d
      JR_cond(cpu->main.singles.F.z);
      break;
    case 0x29:  // add hl, hl
      ADD_R16_R16(&cpu->main.pairs.HL, &cpu->main.pairs.HL);
      break;
    case 0x2A:  // ld hl, (nn)
      LD_R16_NNa(&cpu->main.pairs.HL);
      break;
    case 0x2B:  // dec hl
      DEC_R16(&cpu->main.pairs.HL);
      break;
    case 0x2C:  // inc l
      INC_R8(&cpu->main.singles.L);
      break;
    case 0x2D:  // dec l
      DEC_R8(&cpu->main.singles.L);
      break;
    case 0x2E:  // ld l, n
      LD_R8_N(&cpu->main.singles.L);
      break;
    case 0x2F: {// cpl
      u8 result = cpu->main.singles.A ^ 0xFF;
      cpu->main.singles.A = result;
      cpu->main.singles.F.h = 1;
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.b3 = (result & 0x8) >> 3;
      cpu->main.singles.F.b5 = (result & 0x20) >> 5;
      break;
    }
    case 0x30:  // jr nc, d
      JR_cond(!cpu->main.singles.F.c);
      break;
    case 0x31:  // ld sp, nn
      LD_R16_NN(&cpu->SP);
      break;
    case 0x32:  // ld (nn), a
      LD_NN_R8(cpu->main.singles.A);
      break;
    case 0x33:  // inc sp
      INC_R16(&cpu->SP);
      break;
    case 0x34:  // inc (hl)
      INC_R16a(cpu->main.pairs.HL);
      break;
    case 0x35:  // dec (hl)
      DEC_R16a(cpu->main.pairs.HL);
      break;
    case 0x36:  // ld (hl), n
      write_u8(cpu->main.pairs.HL, read_u8(cpu->PC));
      cpu->PC+=1;
      break;
    case 0x37:  // scf
      cpu->main.singles.F.c = 1;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      cpu->main.singles.F.b3 = (cpu->main.singles.A & 0x8) >> 3;
      cpu->main.singles.F.b5 = (cpu->main.singles.A & 0x20) >> 5;
      break;
    case 0x38:  // jr c, d
      JR_cond(cpu->main.singles.F.c);
      break;
    case 0x39:  // add hl, sp
      ADD_R16_R16(&cpu->main.pairs.HL, &cpu->SP);
      break;
    case 0x3A:  // ld a, (nn)
      LD_R8_NN(&cpu->main.singles.A);
      break;
    case 0x3B:  // dec sp
      DEC_R16(&cpu->SP);
      break;
    case 0x3C:  // inc a
      INC_R8(&cpu->main.singles.A);
      break;
    case 0x3D:  // dec a
      DEC_R8(&cpu->main.singles.A);
      break;
    case 0x3E:  // ld a, n
      LD_R8_N(&cpu->main.singles.A);
      break;
    case 0x3F:  // ccf
      cpu->main.singles.F.h = cpu->main.singles.F.c;
      cpu->main.singles.F.c = !cpu->main.singles.F.c;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.b3 = (cpu->main.singles.A & 0x8) >> 3;
      cpu->main.singles.F.b5 = (cpu->main.singles.A & 0x20) >> 5;
      break;
    case 0x40:  // ld b, b
      LD_R8_R8(&cpu->main.singles.B, cpu->main.singles.B);
      break;
    case 0x41:  // ld b, c
      LD_R8_R8(&cpu->main.singles.B, cpu->main.singles.C);
      break;
    case 0x42:  // ld b, d
      LD_R8_R8(&cpu->main.singles.B, cpu->main.singles.D);
      break;
    case 0x43:  // ld b, e
      LD_R8_R8(&cpu->main.singles.B, cpu->main.singles.E);
      break;
    case 0x44:  // ld b, h
      LD_R8_R8(&cpu->main.singles.B, cpu->main.singles.H);
      break;
    case 0x45:  // ld b, l
      LD_R8_R8(&cpu->main.singles.B, cpu->main.singles.L);
      break;
    case 0x46:  // ld b, (hl)
      LD_R8_R16(&cpu->main.singles.B, cpu->main.pairs.HL);
      break;
    case 0x47:  // ld b, a
      LD_R8_R8(&cpu->main.singles.B, cpu->main.singles.A);
      break;
    case 0x48:  // ld c, b
      LD_R8_R8(&cpu->main.singles.C, cpu->main.singles.B);
      break;
    case 0x49:  // ld c, c
      LD_R8_R8(&cpu->main.singles.C, cpu->main.singles.C);
      break;
    case 0x4A:  // ld c, d
      LD_R8_R8(&cpu->main.singles.C, cpu->main.singles.D);
      break;
    case 0x4B:  // ld c, e
      LD_R8_R8(&cpu->main.singles.C, cpu->main.singles.E);
      break;
    case 0x4C:  // ld c, h
      LD_R8_R8(&cpu->main.singles.C, cpu->main.singles.H);
      break;
    case 0x4D:  // ld c, l
      LD_R8_R8(&cpu->main.singles.C, cpu->main.singles.L);
      break;
    case 0x4E:  // ld c, (hl)
      LD_R8_R16(&cpu->main.singles.C, cpu->main.pairs.HL);
      break;
    case 0x4F:  // ld c, a
      LD_R8_R8(&cpu->main.singles.C, cpu->main.singles.A);
      break;
    case 0x50:  // ld d, b
      LD_R8_R8(&cpu->main.singles.D, cpu->main.singles.B);
      break;
    case 0x51:  // ld d, c
      LD_R8_R8(&cpu->main.singles.D, cpu->main.singles.C);
      break;
    case 0x52:  // ld d, d
      LD_R8_R8(&cpu->main.singles.D, cpu->main.singles.D);
      break;
    case 0x53:  // ld d, e
      LD_R8_R8(&cpu->main.singles.D, cpu->main.singles.E);
      break;
    case 0x54:  // ld d, h
      LD_R8_R8(&cpu->main.singles.D, cpu->main.singles.H);
      break;
    case 0x55:  // ld d, l
      LD_R8_R8(&cpu->main.singles.D, cpu->main.singles.L);
      break;
    case 0x56:  // ld d, (hl)
      LD_R8_R16(&cpu->main.singles.D, cpu->main.pairs.HL);
      break;
    case 0x57:  // ld b, a
      LD_R8_R8(&cpu->main.singles.D, cpu->main.singles.A);
      break;
    case 0x58:  // ld e, b
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.B);
      break;
    case 0x59:  // ld e, c
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.C);
      break;
    case 0x5A:  // ld e, d
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.D);
      break;
    case 0x5B:  // ld e, e
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.E);
      break;
    case 0x5C:  // ld e, h
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.H);
      break;
    case 0x5D:  // ld e, l
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.L);
      break;
    case 0x5E:  // ld e, (hl)
      LD_R8_R16(&cpu->main.singles.E, cpu->main.pairs.HL);
      break;
    case 0x5F:  // ld e, a
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.A);
      break;
    case 0x60:  // ld h, b
      LD_R8_R8(&cpu->main.singles.H, cpu->main.singles.B);
      break;
    case 0x61:  // ld h, c
      LD_R8_R8(&cpu->main.singles.H, cpu->main.singles.C);
      break;
    case 0x62:  // ld h, d
      LD_R8_R8(&cpu->main.singles.H, cpu->main.singles.D);
      break;
    case 0x63:  // ld h, e
      LD_R8_R8(&cpu->main.singles.H, cpu->main.singles.E);
      break;
    case 0x64:  // ld h, h
      LD_R8_R8(&cpu->main.singles.H, cpu->main.singles.H);
      break;
    case 0x65:  // ld h, l
      LD_R8_R8(&cpu->main.singles.H, cpu->main.singles.L);
      break;
    case 0x66:  // ld h, (hl)
      LD_R8_R16(&cpu->main.singles.H, cpu->main.pairs.HL);
      break;
    case 0x67:  // ld h, a
      LD_R8_R8(&cpu->main.singles.H, cpu->main.singles.A);
      break;
    case 0x68:  // ld l, b
      LD_R8_R8(&cpu->main.singles.L, cpu->main.singles.B);
      break;
    case 0x69:  // ld l, c
      LD_R8_R8(&cpu->main.singles.L, cpu->main.singles.C);
      break;
    case 0x6A:  // ld l, d
      LD_R8_R8(&cpu->main.singles.L, cpu->main.singles.D);
      break;
    case 0x6B:  // ld l, e
      LD_R8_R8(&cpu->main.singles.L, cpu->main.singles.E);
      break;
    case 0x6C:  // ld l, h
      LD_R8_R8(&cpu->main.singles.L, cpu->main.singles.H);
      break;
    case 0x6D:  // ld l, l
      LD_R8_R8(&cpu->main.singles.L, cpu->main.singles.L);
      break;
    case 0x6E:  // ld l, (hl)
      LD_R8_R16(&cpu->main.singles.L, cpu->main.pairs.HL);
      break;
    case 0x6F:  // ld l, a
      LD_R8_R8(&cpu->main.singles.L, cpu->main.singles.A);
      break;
    case 0x70:  // ld (hl), B
      LD_R16_R8(cpu->main.pairs.HL, cpu->main.singles.B);
      break;
    case 0x71:  // ld (hl), C
      LD_R16_R8(cpu->main.pairs.HL, cpu->main.singles.C);
      break;
    case 0x72:  // ld (hl), D
      LD_R16_R8(cpu->main.pairs.HL, cpu->main.singles.D);
      break;
    case 0x73:  // ld (hl), E
      LD_R16_R8(cpu->main.pairs.HL, cpu->main.singles.E);
      break;
    case 0x74:  // ld (hl), H
      LD_R16_R8(cpu->main.pairs.HL, cpu->main.singles.H);
      break;
    case 0x75:  // ld (hl), L
      LD_R16_R8(cpu->main.pairs.HL, cpu->main.singles.L);
      break;
    case 0x76: // halt
      cpu->PC -= 1;
      *halted = true;
      break;
    case 0x77:  // ld (hl), A
      LD_R16_R8(cpu->main.pairs.HL, cpu->main.singles.A);
      break;
    case 0x78:  // ld a, b
      LD_R8_R8(&cpu->main.singles.A, cpu->main.singles.B);
      break;
    case 0x79:  // ld a, c
      LD_R8_R8(&cpu->main.singles.A, cpu->main.singles.C);
      break;
    case 0x7A:  // ld a, d
      LD_R8_R8(&cpu->main.singles.A, cpu->main.singles.D);
      break;
    case 0x7B:  // ld a, e
      LD_R8_R8(&cpu->main.singles.A, cpu->main.singles.E);
      break;
    case 0x7C:  // ld a, h
      LD_R8_R8(&cpu->main.singles.A, cpu->main.singles.H);
      break;
    case 0x7D:  // ld a, l
      LD_R8_R8(&cpu->main.singles.A, cpu->main.singles.L);
      break;
    case 0x7E:  // ld a, (hl)
      LD_R8_R16(&cpu->main.singles.A, cpu->main.pairs.HL);
      break;
    case 0x7F:  // ld a, a
      LD_R8_R8(&cpu->main.singles.A, cpu->main.singles.A);
      break;
    case 0x80:  // add a, b
      ADD_R8_R8(&cpu->main.singles.A, cpu->main.singles.B);
      break;
    case 0x81:  // add a, c
      ADD_R8_R8(&cpu->main.singles.A, cpu->main.singles.C);
      break;
    case 0x82:  // add a, d
      ADD_R8_R8(&cpu->main.singles.A, cpu->main.singles.D);
      break;
    case 0x83:  // add a, e
      ADD_R8_R8(&cpu->main.singles.A, cpu->main.singles.E);
      break;
    case 0x84:  // add a, h
      ADD_R8_R8(&cpu->main.singles.A, cpu->main.singles.H);
      break;
    case 0x85:  // add a, l
      ADD_R8_R8(&cpu->main.singles.A, cpu->main.singles.L);
      break;
    case 0x86:  // add a, (hl)
      ADD_R8_R8(&cpu->main.singles.A, read_u8(cpu->main.pairs.HL));
      break;
    case 0x87:  // add a, a
      ADD_R8_R8(&cpu->main.singles.A, cpu->main.singles.A);
      break;
    case 0x88:  // adc a, b
      ADC_R8_R8(&cpu->main.singles.A, cpu->main.singles.B);
      break;
    case 0x89:  // adc a, c
      ADC_R8_R8(&cpu->main.singles.A, cpu->main.singles.C);
      break;
    case 0x8A:  // adc a, d
      ADC_R8_R8(&cpu->main.singles.A, cpu->main.singles.D);
      break;
    case 0x8B:  // adc a, e
      ADC_R8_R8(&cpu->main.singles.A, cpu->main.singles.E);
      break;
    case 0x8C:  // adc a, h
      ADC_R8_R8(&cpu->main.singles.A, cpu->main.singles.H);
      break;
    case 0x8D:  // adc a, l
      ADC_R8_R8(&cpu->main.singles.A, cpu->main.singles.L);
      break;
    case 0x8E:  // adc a, (hl)
      ADC_R8_R8(&cpu->main.singles.A, read_u8(cpu->main.pairs.HL));
      break;
    case 0x8F:  // adc a, a
      ADC_R8_R8(&cpu->main.singles.A, cpu->main.singles.A);
      break;
    case 0x90:  // sub b
      SUB_R8_R8(&cpu->main.singles.A, cpu->main.singles.B);
      break;
    case 0x91:  // sub c
      SUB_R8_R8(&cpu->main.singles.A, cpu->main.singles.C);
      break;
    case 0x92:  // sub d
      SUB_R8_R8(&cpu->main.singles.A, cpu->main.singles.D);
      break;
    case 0x93:  // sub e
      SUB_R8_R8(&cpu->main.singles.A, cpu->main.singles.E);
      break;
    case 0x94:  // sub h
      SUB_R8_R8(&cpu->main.singles.A, cpu->main.singles.H);
      break;
    case 0x95:  // sub l
      SUB_R8_R8(&cpu->main.singles.A, cpu->main.singles.L);
      break;
    case 0x96:  // sub (hl)
      SUB_R8_R8(&cpu->main.singles.A, read_u8(cpu->main.pairs.HL));
      break;
    case 0x97:  // sub a
      SUB_R8_R8(&cpu->main.singles.A, cpu->main.singles.A);
      break;
    case 0x98:  // sbc a, b
      SBC_R8_R8(&cpu->main.singles.A, cpu->main.singles.B);
      break;
    case 0x99:  // sbc a, c
      SBC_R8_R8(&cpu->main.singles.A, cpu->main.singles.C);
      break;
    case 0x9A:  // sbc a, d
      SBC_R8_R8(&cpu->main.singles.A, cpu->main.singles.D);
      break;
    case 0x9B:  // sbc a, e
      SBC_R8_R8(&cpu->main.singles.A, cpu->main.singles.E);
      break;
    case 0x9C:  // sbc a, h
      SBC_R8_R8(&cpu->main.singles.A, cpu->main.singles.H);
      break;
    case 0x9D:  // sbc a, l
      SBC_R8_R8(&cpu->main.singles.A, cpu->main.singles.L);
      break;
    case 0x9E:  // sbc a, (hl)
      SBC_R8_R8(&cpu->main.singles.A, read_u8(cpu->main.pairs.HL));
      break;
    case 0x9F:  // sbc a, a
      SBC_R8_R8(&cpu->main.singles.A, cpu->main.singles.A);
      break;
    case 0xA0:  // and b
      AND_R8_R8(&cpu->main.singles.A, cpu->main.singles.B);
      break;
    case 0xA1:  // and c
      AND_R8_R8(&cpu->main.singles.A, cpu->main.singles.C);
      break;
    case 0xA2:  // and d
      AND_R8_R8(&cpu->main.singles.A, cpu->main.singles.D);
      break;
    case 0xA3:  // and e
      AND_R8_R8(&cpu->main.singles.A, cpu->main.singles.E);
      break;
    case 0xA4:  // and h
      AND_R8_R8(&cpu->main.singles.A, cpu->main.singles.H);
      break;
    case 0xA5:  // and l
      AND_R8_R8(&cpu->main.singles.A, cpu->main.singles.L);
      break;
    case 0xA6:  // and (hl)
      AND_R8_R8(&cpu->main.singles.A, read_u8(cpu->main.pairs.HL));
      break;
    case 0xA7:  // and a
      AND_R8_R8(&cpu->main.singles.A, cpu->main.singles.A);
      break;
    case 0xA8:  // xor b
      XOR_R8_R8(&cpu->main.singles.A, cpu->main.singles.B);
      break;
    case 0xA9:  // xor c
      XOR_R8_R8(&cpu->main.singles.A, cpu->main.singles.C);
      break;
    case 0xAA:  // xor d
      XOR_R8_R8(&cpu->main.singles.A, cpu->main.singles.D);
      break;
    case 0xAB:  // xor e
      XOR_R8_R8(&cpu->main.singles.A, cpu->main.singles.E);
      break;
    case 0xAC:  // xor h
      XOR_R8_R8(&cpu->main.singles.A, cpu->main.singles.H);
      break;
    case 0xAD:  // xor l
      XOR_R8_R8(&cpu->main.singles.A, cpu->main.singles.L);
      break;
    case 0xAE:  // xor (hl)
      XOR_R8_R8(&cpu->main.singles.A, read_u8(cpu->main.pairs.HL));
      break;
    case 0xAF:  // xor a
      XOR_R8_R8(&cpu->main.singles.A, cpu->main.singles.A);
      break;
    case 0xB0:  // or b
      OR_R8_R8(&cpu->main.singles.A, cpu->main.singles.B);
      break;
    case 0xB1:  // or c
      OR_R8_R8(&cpu->main.singles.A, cpu->main.singles.C);
      break;
    case 0xB2:  // or d
      OR_R8_R8(&cpu->main.singles.A, cpu->main.singles.D);
      break;
    case 0xB3:  // or e
      OR_R8_R8(&cpu->main.singles.A, cpu->main.singles.E);
      break;
    case 0xB4:  // or h
      OR_R8_R8(&cpu->main.singles.A, cpu->main.singles.H);
      break;
    case 0xB5:  // or l
      OR_R8_R8(&cpu->main.singles.A, cpu->main.singles.L);
      break;
    case 0xB6:  // or (hl)
      OR_R8_R8(&cpu->main.singles.A, read_u8(cpu->main.pairs.HL));
      break;
    case 0xB7:  // or a
      OR_R8_R8(&cpu->main.singles.A, cpu->main.singles.A);
      break;
    case 0xB8:  // cp b
      CP_R8_R8(&cpu->main.singles.A, cpu->main.singles.B);
      break;
    case 0xB9:  // cp c
      CP_R8_R8(&cpu->main.singles.A, cpu->main.singles.C);
      break;
    case 0xBA:  // cp d
      CP_R8_R8(&cpu->main.singles.A, cpu->main.singles.D);
      break;
    case 0xBB:  // cp e
      CP_R8_R8(&cpu->main.singles.A, cpu->main.singles.E);
      break;
    case 0xBC:  // cp h
      CP_R8_R8(&cpu->main.singles.A, cpu->main.singles.H);
      break;
    case 0xBD:  // cp l
      CP_R8_R8(&cpu->main.singles.A, cpu->main.singles.L);
      break;
    case 0xBE:  // cp (hl)
      CP_R8_R8(&cpu->main.singles.A, read_u8(cpu->main.pairs.HL));
      break;
    case 0xBF:  // cp a
      CP_R8_R8(&cpu->main.singles.A, cpu->main.singles.A);
      break;
    case 0xC0:  // ret nz
      RET_cond(!cpu->main.singles.F.z);
      break;
    case 0xC1:  // pop bc
      cpu->main.pairs.BC = POP();
      break;
    case 0xC2:  // jp nz, nn
      JP_cond(!cpu->main.singles.F.z);
      break;
    case 0xC3:  // jp nn
      JP();
      break;
    case 0xC4:  // call nz, nn
      CALL_cond(!cpu->main.singles.F.z);
      break;
    case 0xC5:  // push bc
      PUSH(cpu->main.pairs.BC);
      break;
    case 0xC6:  // add a, n
      ADD_R8_N(&cpu->main.singles.A);
      break;
    case 0xC7:  // rst 00h
      RST(0);
      break;
    case 0xC8:  // ret z
      RET_cond(cpu->main.singles.F.z);
      break;
    case 0xC9:  // ret
      RET();
      break;
    case 0xCA:  // jp z, nn
      JP_cond(cpu->main.singles.F.z);
      break;
    case 0xCB:  // CB instructions
      return execute_cb();
    case 0xCC:  // call z, nn
      CALL_cond(cpu->main.singles.F.z);
      break;
    case 0xCD:  // call nn
      CALL();
      break;
    case 0xCE:  // adc a, n
      ADC_R8_N(&cpu->main.singles.A);
      break;
    case 0xCF:  // rst 08h
      RST(8);
      break;
    case 0xD0:  // ret nc
      RET_cond(!cpu->main.singles.F.c);
      break;
    case 0xD1:  // pop de
      cpu->main.pairs.DE = POP();
      break;
    case 0xD2:  // jp nc, nn
      JP_cond(!cpu->main.singles.F.c);
      break;
    case 0xD3:  // out (n), a
      write_io(read_u8(cpu->PC), cpu->main.singles.A);
      cpu->PC += 1;
      break;
    case 0xD4:  // call nc, nn
      CALL_cond(!cpu->main.singles.F.c);
      break;
    case 0xD5:  // push de
      PUSH(cpu->main.pairs.DE);
      break;
    case 0xD6:  // sub n
      SUB_R8_N(&cpu->main.singles.A);
      break;
    case 0xD7:  // rst 10h
      RST(0x10);
      break;
    case 0xD8:  // ret c
      RET_cond(cpu->main.singles.F.c);
      break;
    case 0xD9: {// exx
      u16 temp = cpu->alt.pairs.BC;
      cpu->alt.pairs.BC = cpu->main.pairs.BC;
      cpu->main.pairs.BC = temp;
      temp = cpu->alt.pairs.DE;
      cpu->alt.pairs.DE = cpu->main.pairs.DE;
      cpu->main.pairs.DE = temp;
      temp = cpu->alt.pairs.HL;
      cpu->alt.pairs.HL = cpu->main.pairs.HL;
      cpu->main.pairs.HL = temp;
      break;
    }
    case 0xDA:  // jp c, nn
      JP_cond(cpu->main.singles.F.c);
      break;
    case 0xDB:  // in a, (n)
      cpu->main.singles.A = read_io(read_u8(cpu->PC));
      cpu->PC += 1;
      break;
    case 0xDC:  // call c, nn
      CALL_cond(cpu->main.singles.F.c);
      break;
    case 0xDD:  // dd instructions
      return execute_ddfd(true);
    case 0xDE:  // sbc a, n
      SBC_R8_N(&cpu->main.singles.A);
      break;
    case 0xDF:  // rst 18h
      RST(0x18);
      break;
    case 0xE0:  // ret po
      RET_cond(!cpu->main.singles.F.pv);
      break;
    case 0xE1:  // pop hl
      cpu->main.pairs.HL = POP();
      break;
    case 0xE2:  // jp po, nn
      JP_cond(!cpu->main.singles.F.pv);
      break;
    case 0xE3: {// ex (sp), hl
      u16 tmp = read_u16(cpu->SP);
      write_u16(cpu->SP, cpu->main.pairs.HL);
      cpu->main.pairs.HL = tmp;
      break;
    }
    case 0xE4:  // call po, nn
      CALL_cond(!cpu->main.singles.F.pv);
      break;
    case 0xE5:  // push hl
      PUSH(cpu->main.pairs.HL);
      break;
    case 0xE6:  // and n
      AND_R8_N(&cpu->main.singles.A);
      break;
    case 0xE7:  // rst 20h
      RST(0x20);
      break;
    case 0xE8:  // ret pe
      RET_cond(cpu->main.singles.F.pv);
      break;
    case 0xE9:  // jp (hl)
      cpu->PC = cpu->main.pairs.HL;
      break;
    case 0xEA:  // jp pe, nn
      JP_cond(cpu->main.singles.F.pv);
      break;
    case 0xEB: {// ex de, hl
      u16 tmp = cpu->main.pairs.HL;
      cpu->main.pairs.HL = cpu->main.pairs.DE;
      cpu->main.pairs.DE = tmp;
      break;
    }
    case 0xEC:  // call pe, nn
      CALL_cond(cpu->main.singles.F.pv);
      break;
    case 0xED: // ed instructions
      return execute_ed();
    case 0xEE: // xor n
      XOR_R8_N(&cpu->main.singles.A);
      break;
    case 0xEF: // rst 28h
      RST(0x28);
      break;
    case 0xF0:  // ret p
      RET_cond(!cpu->main.singles.F.s);
      break;
    case 0xF1:  // pop af
      cpu->main.pairs.AF = POP();
      break;
    case 0xF2:  // jp p, nn
      JP_cond(!cpu->main.singles.F.s);
      break;
    case 0xF3:  // di
      DI();
      break;
    case 0xF4:  // call p, nn
      CALL_cond(!cpu->main.singles.F.s);
      break;
    case 0xF5:  // push af
      PUSH(cpu->main.pairs.AF);
      break;
    case 0xF6:  // or n
      OR_R8_N(&cpu->main.singles.A);
      break;
    case 0xF7:  // rst 30h
      RST(0x30);
      break;
    case 0xF8:  // ret m
      RET_cond(cpu->main.singles.F.s);
      break;
    case 0xF9:  // ld sp, hl
      cpu->SP = cpu->main.pairs.HL;
      break;
    case 0xFA:  // jp m, nn
      JP_cond(cpu->main.singles.F.s);
      break;
    case 0xFB:  // ei
      EI();
      break;
    case 0xFC:  // call m, nn
      CALL_cond(cpu->main.singles.F.s);
      break;
    case 0xFD:  // fd instructions
      execute_ddfd(false);
      break;
    case 0xFE:  // cp n
      CP_R8_N(&cpu->main.singles.A);
      break;
    case 0xFF:  // rst 38h
      RST(0x38);
      break;
    default:
      printf("undocumented inst %02x\n", inst);
      break;
  }
  int result = cycles[inst] + bonus_cycles;
  bonus_cycles = 0;
  return result;
}

INLINED u8 _RLC(u8 val) {
  u8 result = ((val << 1) & 0xFF) | ((val & 0x80) >> 7);
  cpu->main.singles.F.c = result & 1;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = parity(result);
  cpu->main.singles.F.h = 0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
  return result;
}

INLINED void RLC_R8(u8* reg) {
    *reg = _RLC(*reg);
}

INLINED void RLC_R16(u16 addr) {
  write_u8(addr, _RLC(read_u8(addr)));
}

INLINED u8 _RRC(u8 val) {
  u8 result = ((val >> 1) & 0xFF) | ((val & 1) << 7);
  cpu->main.singles.F.c = val & 1;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = parity(result);
  cpu->main.singles.F.h = 0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
  return result;
}

INLINED void RRC_R8(u8* reg) {
    *reg = _RRC(*reg);
}

INLINED void RRC_R16(u16 addr) {
  write_u8(addr, _RRC(read_u8(addr)));
}

INLINED u8 _RL(u8 val) {
  u8 result = ((val << 1) & 0xFF)| cpu->main.singles.F.c;
  cpu->main.singles.F.c = (val & 0x80) >> 7;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = parity(result);
  cpu->main.singles.F.h = 0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
  return result;
}

INLINED void RL_R8(u8* reg) {
    *reg = _RL(*reg);
}

INLINED void RL_R16(u16 addr) {
  write_u8(addr, _RL(read_u8(addr)));
}

INLINED u8 _RR(u8 val) {
  u8 result = ((val >> 1) & 0xFF) | (cpu->main.singles.F.c << 7);
  cpu->main.singles.F.c = val & 1;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = parity(result);
  cpu->main.singles.F.h = 0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
  return result;
}

INLINED void RR_R8(u8* reg) {
    *reg = _RR(*reg);
}

INLINED void RR_R16(u16 addr) {
  write_u8(addr, _RR(read_u8(addr)));
}


INLINED u8 _SLA(u8 val) {
  u8 result = (val << 1) & 0xFF;
  cpu->main.singles.F.c = (val & 0x80) >> 7;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = parity(result);
  cpu->main.singles.F.h = 0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
  return result;
}

INLINED void SLA_R8(u8* reg) {
    *reg = _SLA(*reg);
}

INLINED void SLA_R16(u16 addr) {
  write_u8(addr, _SLA(read_u8(addr)));
}


INLINED u8 _SRA(u8 val) {
  u8 result = ((val >> 1) & 0xFF) | (val & 0x80);
  cpu->main.singles.F.c = val & 1;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = parity(result);
  cpu->main.singles.F.h = 0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
  return result;
}

INLINED void SRA_R8(u8* reg) {
    *reg = _SRA(*reg);
}

INLINED void SRA_R16(u16 addr) {
  write_u8(addr, _SRA(read_u8(addr)));
}

INLINED u8 _SRL(u8 val) {
  u8 result = (val >> 1) & 0xFF;
  cpu->main.singles.F.c = val & 1;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = parity(result);
  cpu->main.singles.F.h = 0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
  return result;
}

INLINED void SRL_R8(u8* reg) {
    *reg = _SRL(*reg);
}

INLINED void SRL_R16(u16 addr) {
  write_u8(addr, _SRL(read_u8(addr)));
}

INLINED u8 _SLL(u8 val) {
  u8 result = ((val << 1) & 0xFF) | 1;
  cpu->main.singles.F.c = (val & 0x80) >> 7;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = parity(result);
  cpu->main.singles.F.h = 0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
  cpu->main.singles.F.b3 = (result & 0x8) >> 3;
  cpu->main.singles.F.b5 = (result & 0x20) >> 5;
  return result;
}

INLINED void SLL_R8(u8* reg) {
    *reg = _SLL(*reg);
}

INLINED void SLL_R16(u16 addr) {
  write_u8(addr, _SLL(read_u8(addr)));
}

INLINED u8 BIT(u8 val, int bitNum) {
  u8 bit = (val >> bitNum) & 1;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.h = 1;
  cpu->main.singles.F.z = bit == 0;
  cpu->main.singles.F.pv = bit == 0;
  cpu->main.singles.F.s = bitNum == 7 ? bit : 0;
  cpu->main.singles.F.b3 = (val & 0x8) >> 3;
  cpu->main.singles.F.b5 = (val & 0x20) >> 5;
  return val;
}

INLINED u8 _RES(u8 val, int bitNum) {
  return val & ~(1 << bitNum);
}

INLINED void RES_R8(u8* reg, int bitNum) {
  *reg = _RES(*reg, bitNum);
}

INLINED void RES_R16(u16 addr, int bitNum) {
  write_u8(addr, _RES(read_u8(addr), bitNum));
}

INLINED u8 _SET(u8 val, int bitNum) {
  return val | (1 << bitNum);
}

INLINED void SET_R8(u8* reg, int bitNum) {
  *reg = _SET(*reg, bitNum);
}

INLINED void SET_R16(u16 addr, int bitNum) {
  write_u8(addr, _SET(read_u8(addr), bitNum));
}

u8 execute_bitwise(u8 inst, u8 input) {
  if (inst < 0x40) {
    switch ((inst & 0b00111000) >> 3) {
      default:
      case 0: 
        return _RLC(input);
      case 1: 
        return _RRC(input);
      case 2: 
        return _RL(input);
      case 3: 
        return _RR(input);
      case 4: 
        return _SLA(input);
      case 5: 
        return _SRA(input);
      case 6:
        return _SLL(input);
      case 7: 
        return _SRL(input);
    }
  }

  else if (inst < 0x80) {
    u8 bitNum = (inst & 0b00111000) >> 3;
    return BIT(input, bitNum);
  }
  
  else if (inst < 0xC0) {
    u8 bitNum = (inst & 0b00111000) >> 3;
    return _RES(input, bitNum);
  }

  u8 bitNum = (inst & 0b00111000) >> 3;
  return _SET(input, bitNum);
}
u8* get_register(u8 inst) {
  u8 regNum = inst & 7;
  switch (regNum) {
    case 0: return &cpu->main.singles.B;
    case 1: return &cpu->main.singles.C;
    case 2: return &cpu->main.singles.D;
    case 3: return &cpu->main.singles.E;
    case 4: return &cpu->main.singles.H;
    case 5: return &cpu->main.singles.L;
    case 7: return &cpu->main.singles.A;
  }
  return 0;
}

int execute_cb() {
  // trying to implement decoding to make the process easier
  u8 inst = read_u8(cpu->PC++);
  incr_r();

  u8* reg = get_register(inst);
  u8 input;
  if (reg) input = *reg;
  else input = read_u8(cpu->main.pairs.HL);

  u8 result = execute_bitwise(inst, input);
  if (reg) *reg = result;
  else write_u8(cpu->main.pairs.HL, result);

  return cycles_cb[inst];
}

int execute_displacedcb(u16* reg) {
  u16 displacedAddress = *reg + (s8)read_u8(cpu->PC++);
  u8 inst = read_u8(cpu->PC++);
  u8 input = read_u8(displacedAddress);

  u8* resultReg = get_register(inst);
  u8 result = execute_bitwise(inst, input);
  if (resultReg) *resultReg = result;
  else write_u8(displacedAddress, result);

  if (inst < 0x80 && inst >= 0x40) return 20;
  return 23;
} 