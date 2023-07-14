#include "cpu.h"

#include <string.h>

#include "mem.h"

Z80* cpu;
int bonus_cycles = 0;

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

bool parity(u8 value) {
  value ^= value >> 4;
  value &= 0xF;
  return !((0x6996 >> value) & 1);
}

void init_cpu(Z80* CPU) {
  cpu = CPU;
  memset(cpu, 0, sizeof(Z80));
}
int opcode_cycles[] = {};

void LD_R16_NN(u16* reg) {
  *reg = read_u16(cpu->PC);
  cpu->PC += 2;
}

void LD_R16_NNa(u16* reg) {
  *reg = read_u16(read_u16(cpu->PC));
  cpu->PC += 2;
}

void LD_R8_R8(u8* reg, u8 val) {
  *reg = val;
}

void LD_NN_R16(u16 addr, u16 reg) {
  write_u16(addr, reg);
  cpu->PC += 2;
}

void LD_NN_R8(u8 reg) {
  write_u8(read_u16(cpu->PC), reg);
  cpu->PC += 2;
}

void LD_R8_N(u8* reg) {
  *reg = read_u8(cpu->PC);
  cpu->PC += 1;
}

void LD_R8_NN(u8* reg) {
  *reg = read_u8(read_u16(cpu->PC));
  cpu->PC += 2;
}

void LD_R16_R8(u16 dest, u8 value) {
  write_u8(dest, value);
}

void LD_R8_R16(u8* dest, u16 addr) {
  *dest = read_u8(addr);
}

void ADD_R16_R16(u16* dest, u16* value) {
  u16 dst = *dest;
  u16 val = *value;
  u16 result = dst + val;
  *dest = result;
  cpu->main.singles.F.c = (dst + val) > 0xFFFF;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.h = (dst & 0xF) + (val & 0xF) > 0xF;
}

void INC_R16(u16* reg) {
  *reg += 1;
}

void INC_R16a(u16 addr) {
  u8 value = read_u16(addr);
  u8 result = value + 1;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = value == 0x7F;
  cpu->main.singles.F.h = (value & 0xF) == 0xF;
  cpu->main.singles.F.z = value == 0xFF;
  cpu->main.singles.F.s = result >> 7;
  write_u8(addr, result);
}

void INC_R8(u8* reg) {
  u8 value = *reg;
  u8 result = value + 1;
  *reg = result;
  cpu->main.singles.F.n = 0;
  cpu->main.singles.F.pv = value == 0x7F;
  cpu->main.singles.F.h = (value & 0xF) == 0xF;
  cpu->main.singles.F.z = value == 0xFF;
  cpu->main.singles.F.s = result >> 7;
}

void DEC_R8(u8* reg) {
  u8 value = *reg;
  u8 result = value - 1;
  *reg = result;
  cpu->main.singles.F.n = 1;
  cpu->main.singles.F.pv = value == 0x80;
  cpu->main.singles.F.h = (value & 0xF) == 0x0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
}

void DEC_R16a(u16 addr) {
  u8 value = read_u8(addr);
  u8 result = value - 1;
  write_u8(addr, result);
  cpu->main.singles.F.n = 1;
  cpu->main.singles.F.pv = value == 0x80;
  cpu->main.singles.F.h = (value & 0xF) == 0x0;
  cpu->main.singles.F.z = result == 0;
  cpu->main.singles.F.s = result >> 7;
}

void DEC_R16(u16* reg) {
  *reg -= 1;
}

void JR() {
  s8 offset = (s8)read_u8(cpu->PC);
  cpu->PC += 1 + offset;
}

void JR_cond(bool condition) {
  s8 offset = (s8)read_u8(cpu->PC);
  cpu->PC += 1;
  if (condition) {
    cpu->PC += offset;
    bonus_cycles = 5;
  }
}

int execute_cpu() {
  u8 inst = read_u8(cpu->PC);
  cpu->PC++;

  switch (inst) {
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
      cpu->main.singles.A = (a << 1) | c;
      cpu->main.singles.F.c = c;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
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
      cpu->main.singles.A = a >> 1 | (c << 7);
      cpu->main.singles.F.c = c;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
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
    case 0x17:  // rla
      u8 a = cpu->main.singles.A;
      u8 c = (a & 0x80) >> 7;
      a = a << 1 | cpu->main.singles.F.c;
      cpu->main.singles.F.c = c;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      return 4;
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
      INC_R8(&cpu->main.singles.E);
      break;
    case 0x25:  // dec h
      DEC_R8(&cpu->main.singles.H);
      break;
    case 0x26:  // ld h, n
      LD_R8_N(&cpu->main.singles.H);
      break;
    case 0x27:  // daa
      if (cpu->main.singles.F.n) {
        if (cpu->main.singles.F.c) {
          cpu->main.singles.A -= 0x60;
          cpu->main.singles.F.c = 1;
        } else
          cpu->main.singles.A -= 0x6;
      }

      else {
        if (cpu->main.singles.F.c || cpu->main.singles.A > 0x99) {
          cpu->main.singles.A += 0x60;
          cpu->main.singles.F.c = 1;
        }
        if (cpu->main.singles.F.h || (cpu->main.singles.A & 0xF) > 0x09) cpu->main.singles.A += 6;
      }
      cpu->main.singles.F.pv = parity(cpu->main.singles.A);
      cpu->main.singles.F.z = cpu->main.singles.A == 0;
      cpu->main.singles.F.s = cpu->main.singles.A >> 7;
      break;
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
      cpu->main.singles.F.h = result > 0xF;
      cpu->main.singles.F.z = result == 0;
      break;
    }
    case 0x30:  // jr nc, d
      JR_cond(!cpu->main.singles.F.c);
      break;
    case 0x31:  // ld af, nn
      LD_R16_NN(&cpu->main.pairs.AF);
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
    case 0x36:  // ld (hn), n
      write_u8(cpu->main.pairs.HL, read_u8(cpu->PC));
      cpu->PC+=1;
      break;
    case 0x37:  // scf
      cpu->main.singles.F.c = 1;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      break;
    case 0x38:  // jr c, d
      JR_cond(cpu->main.singles.F.c);
      break;
    case 0x39:  // add hl, sp
      ADD_R16_R16(&cpu->main.pairs.HL, &cpu->main.pairs.HL);
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
      cpu->main.singles.F.c = !cpu->main.singles.F.c;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = !cpu->main.singles.F.h;
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
    case 0x77:  // ld (hl), A
      LD_R16_R8(cpu->main.pairs.HL, cpu->main.singles.A);
      break;
    case 0x78:  // ld a, b
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.B);
      break;
    case 0x79:  // ld a, c
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.C);
      break;
    case 0x7A:  // ld a, d
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.D);
      break;
    case 0x7B:  // ld a, e
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.E);
      break;
    case 0x7C:  // ld a, h
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.H);
      break;
    case 0x7D:  // ld a, l
      LD_R8_R8(&cpu->main.singles.E, cpu->main.singles.L);
      break;
    case 0x7E:  // ld a, (hl)
      LD_R8_R16(&cpu->main.singles.A, cpu->main.pairs.HL);
      break;
  }
  int cycles = opcode_cycles[inst] + bonus_cycles;
  bonus_cycles = 0;
  return cycles;
}