#include "cpu.h"

#include <string.h>

#include "mem.h"

Z80* cpu;
int bonus_cycles = 0;

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

void LD_R8_N(u8* reg) {
  *reg = read_u8(cpu->PC);
  cpu->PC += 1;
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

int execute_cpu2() {
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
    case 0x20:  // jr nz, d
      JR_cond(!cpu->main.singles.F.z);
      break;
    case 0x21:  // ld hl, nn
      LD_R16_NN(&cpu->main.pairs.HL);
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
    case 0x28:  // jr z, d
      JR_cond(cpu->main.singles.F.z);
      break;
    case 0x29:  // add hl, hl
      ADD_R16_R16(&cpu->main.pairs.HL, &cpu->main.pairs.HL);
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
    case 0x30:  // jr nc, d
      JR_cond(!cpu->main.singles.F.c);
      break;
    case 0x31:  // ld af, nn
      LD_R16_NN(&cpu->main.pairs.AF);
      break;
    case 0x38:  // jr c, d
      JR_cond(cpu->main.singles.F.c);
      break;
    case 0x39:  // add hl, sp
      ADD_R16_R16(&cpu->main.pairs.HL, &cpu->main.pairs.HL);
      break;
    case 0x3B:
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
    case 0x46:  // ld b, (hl)
      LD_R8_R16(&cpu->main.singles.B, cpu->main.pairs.HL);
      break;
    case 0x4E:  // ld c, (hl)
      LD_R8_R16(&cpu->main.singles.C, cpu->main.pairs.HL);
      break;
    case 0x56:  // ld d, (hl)
      LD_R8_R16(&cpu->main.singles.D, cpu->main.pairs.HL);
      break;
    case 0x5E:  // ld e, (hl)
      LD_R8_R16(&cpu->main.singles.E, cpu->main.pairs.HL);
      break;
    case 0x66:  // ld h, (hl)
      LD_R8_R16(&cpu->main.singles.H, cpu->main.pairs.HL);
      break;
    case 0x6E:  // ld l, (hl)
      LD_R8_R16(&cpu->main.singles.L, cpu->main.pairs.HL);
      break;
    case 0x7E:  // ld a, (hl)
      LD_R8_R16(&cpu->main.singles.A, cpu->main.pairs.HL);
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
    case 0x76:  // ld (hl), A
      LD_R16_R8(cpu->main.pairs.HL, cpu->main.singles.A);
      break;
  }
  int cycles = opcode_cycles[inst] + bonus_cycles;
  bonus_cycles = 0;
  return cycles;
}

int execute_cpu() {
  u8 inst = read_u8(cpu->PC++);
  switch (inst) {
    case 0x00:  // nop
      return 4;

    case 0x01:  // ld bc, nn
      cpu->main.pairs.BC = read_u16(cpu->PC);
      cpu->PC += 2;
      return 10;

    case 0x02:  // ld (bc), a
      write_u8(cpu->main.pairs.BC, cpu->main.singles.A);
      return 7;

    case 0x03:  // inc bc
      cpu->main.pairs.BC += 1;
      return 6;

    case 0x04: {  // inc b
      u8 b = cpu->main.singles.B;
      u8 result = b + 1;
      cpu->main.singles.B = result;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.pv = b == 0x7F;
      cpu->main.singles.F.h = (b & 0xF) == 0xF;
      cpu->main.singles.F.z = b == 0xFF;
      cpu->main.singles.F.s = result >> 7;
      return 4;
    }

    case 0x05: {  // dec b
      u8 b = cpu->main.singles.B;
      u8 result = b - 1;
      cpu->main.singles.B = result;
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.pv = b == 0x80;
      cpu->main.singles.F.h = (b & 0xF) == 0x0;
      cpu->main.singles.F.z = result == 0;
      cpu->main.singles.F.s = result >> 7;
      return 4;
    }

    case 0x06:  // ld b,n
      cpu->main.singles.B = read_u8(cpu->PC++);
      return 7;

    case 0x07: {  // rlca
      u8 a = cpu->main.singles.A;
      u8 c = a >> 7;
      cpu->main.singles.A = (a << 1) | c;
      cpu->main.singles.F.c = c;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      return 4;
    }

    case 0x08: {  // ex af, af'
      u16 tmp = cpu->main.pairs.AF;
      cpu->main.pairs.AF = cpu->alt.pairs.AF;
      cpu->alt.pairs.AF = tmp;
      return 4;
    }

    case 0x09: {  // add hl, bc
      u16 hl = cpu->main.pairs.HL;
      u16 bc = cpu->main.pairs.BC;
      u16 result = hl + bc;
      cpu->main.pairs.HL = result;
      cpu->main.singles.F.c = (hl + bc) > 0xFFFF;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = (hl & 0xF) + (bc & 0xF) > 0xF;
      return 11;
    }

    case 0x0A:  // ld a, (bc)
      cpu->main.singles.A = read_u8(cpu->main.pairs.BC);
      return 7;

    case 0x0B:  // dec bc
      cpu->main.pairs.BC -= 1;
      return 6;

    case 0x0C: {  // inc c
      u8 c = cpu->main.singles.C;
      u8 result = c + 1;
      cpu->main.singles.C = result;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.pv = c == 0x7F;
      cpu->main.singles.F.h = (c & 0xF) == 0xF;
      cpu->main.singles.F.z = c == 0xFF;
      cpu->main.singles.F.s = result >> 7;
      return 4;
    }

    case 0x0D: {  // dec c
      u8 c = cpu->main.singles.C;
      u8 result = c - 1;
      cpu->main.singles.C = result;
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.pv = c == 0x80;
      cpu->main.singles.F.h = (c & 0xF) == 0x0;
      cpu->main.singles.F.z = result == 0;
      cpu->main.singles.F.s = result >> 7;
      return 4;
    }

    case 0x0E:  // ld c,n
      cpu->main.singles.C = read_u8(cpu->PC++);
      return 7;

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
      cpu->main.singles.B -= 1;
      if (cpu->main.singles.B != 0) {
        cpu->PC += (s8)read_u8(cpu->PC) + 1;
        return 13;
      }
      cpu->PC += 1;
      return 8;

    case 0x11:  // ld de, nn
      cpu->main.pairs.DE = read_u16(cpu->PC);
      cpu->PC += 2;
      return 10;

    case 0x12:  // ld (de), a
      write_u8(cpu->main.pairs.DE, cpu->main.singles.A);
      return 7;

    case 0x13:  // inc de
      cpu->main.pairs.DE += 1;
      return 6;

    case 0x14: {  // inc d
      u8 d = cpu->main.singles.D;
      u8 result = d + 1;
      cpu->main.singles.D = result;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.pv = d == 0x7F;
      cpu->main.singles.F.h = (d & 0xF) == 0xF;
      cpu->main.singles.F.z = d == 0xFF;
      cpu->main.singles.F.s = result >> 7;
      return 4;
    }

    case 0x15: {  // dec d
      u8 d = cpu->main.singles.D;
      u8 result = d - 1;
      cpu->main.singles.D = result;
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.pv = d == 0x80;
      cpu->main.singles.F.h = (d & 0xF) == 0x0;
      cpu->main.singles.F.z = result == 0;
      cpu->main.singles.F.s = result >> 7;
      return 4;
    }

    case 0x16:  // ld d,n
      cpu->main.singles.D = read_u8(cpu->PC++);
      return 7;

    case 0x17: {  // rla
      u8 a = cpu->main.singles.A;
      u8 c = (a & 0x80) >> 7;
      a = a << 1 | cpu->main.singles.F.c;
      cpu->main.singles.F.c = c;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      return 4;
    }

    case 0x18:  // jr d
      cpu->PC += (s8)read_u8(cpu->PC) + 1;
      return 4;

    case 0x19: {  // add hl, de
      u16 hl = cpu->main.pairs.HL;
      u16 de = cpu->main.pairs.DE;
      u16 result = hl + de;
      cpu->main.pairs.HL = result;
      cpu->main.singles.F.c = (hl + de) > 0xFFFF;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = (hl & 0xF) + (de & 0xF) > 0xF;
      return 11;
    }

    case 0x1A:  // ld a, (de)
      cpu->main.singles.A = read_u8(cpu->main.pairs.DE);
      return 7;

    case 0x1B:  // dec de
      cpu->main.pairs.DE -= 1;
      return 6;

    case 0x1C: {  // inc e
      u8 e = cpu->main.singles.E;
      u8 result = e + 1;
      cpu->main.singles.E = result;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.pv = e == 0x7F;
      cpu->main.singles.F.h = (e & 0xF) == 0xF;
      cpu->main.singles.F.z = e == 0xFF;
      cpu->main.singles.F.s = result >> 7;
      return 4;
    }

    case 0x1D: {  // dec e
      u8 e = cpu->main.singles.E;
      u8 result = e - 1;
      cpu->main.singles.E = result;
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.pv = e == 0x80;
      cpu->main.singles.F.h = (e & 0xF) == 0x0;
      cpu->main.singles.F.z = result == 0;
      cpu->main.singles.F.s = result >> 7;
      return 4;
    }

    case 0x1E:  // ld e,n
      cpu->main.singles.E = read_u8(cpu->PC++);
      return 7;

    case 0x1F: {  // rra
      u8 a = cpu->main.singles.A;
      u8 c = a & 1;
      cpu->main.singles.A = a >> 1 | (cpu->main.singles.F.c << 7);
      cpu->main.singles.F.c = c;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = 0;
      return 4;
    }

    case 0x20:
      if (!cpu->main.singles.F.z) {
        cpu->PC += (s8)read_u8(cpu->PC) + 1;
        return 12;
      }
      cpu->PC += 1;
      return 7;

    case 0x21:  // ld hl, nn
      cpu->main.pairs.HL = read_u16(cpu->PC);
      cpu->PC += 2;
      return 10;

    case 0x22:  // ld (nn), hl
      write_u8(read_u16(cpu->PC), cpu->main.singles.A);
      cpu->PC += 2;
      return 7;

    case 0x23:  // inc hl
      cpu->main.pairs.HL += 1;
      return 6;

    case 0x24: {  // inc h
      u8 h = cpu->main.singles.H;
      u8 result = h + 1;
      cpu->main.singles.H = result;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.pv = h == 0x7F;
      cpu->main.singles.F.h = (h & 0xF) == 0xF;
      cpu->main.singles.F.z = h == 0xFF;
      cpu->main.singles.F.s = result >> 7;
      return 4;
    }

    case 0x25: {  // dec h
      u8 h = cpu->main.singles.H;
      u8 result = h - 1;
      cpu->main.singles.H = result;
      cpu->main.singles.F.n = 1;
      cpu->main.singles.F.pv = h == 0x80;
      cpu->main.singles.F.h = (h & 0xF) == 0x0;
      cpu->main.singles.F.z = result == 0;
      cpu->main.singles.F.s = result >> 7;
      return 4;
    }

    case 0x26:  // ld h,n
      cpu->main.singles.H = read_u8(cpu->PC++);
      return 7;

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
      return 4;

    case 0x28:  // jz z, d
      if (cpu->main.singles.F.z) {
        cpu->PC += (s8)read_u8(cpu->PC) + 1;
        return 12;
      }
      cpu->PC += 1;
      return 7;

    case 0x29: {  // add hl, hl
      u16 hl = cpu->main.pairs.HL;
      u16 result = hl + hl;
      cpu->main.pairs.HL = result;
      cpu->main.singles.F.c = result > 0xFFFF;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = (hl & 0xF) + (hl & 0xF) > 0xF;
      return 11;
    }

    case 0x2A: {  // ld hl, (nn)
      cpu->main.pairs.HL = read_u16(read_u16(cpu->PC));
      cpu->PC += 2;
      return 16;
    }

    case 0x2B:  // dec hl
      cpu->main.pairs.HL -= 1;
      return 6;

    default: {
      return -1;
    }
  }
  return 0;
}