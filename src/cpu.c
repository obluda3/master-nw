#include <string.h>
#include "cpu.h"
#include "mem.h"

Z80* cpu;

void init_cpu(Z80* CPU) {
  cpu = CPU;
  memset(cpu, 0, sizeof(cpu));
}

int execute_cpu() {
  u8 inst = read_u8(cpu->PC++);
  switch (inst) {
    case 0x00:    // nop
      return 4;

    case 0x01:    // ld bc, nn
      cpu->main.pairs.BC = read_u16(cpu->PC);
      cpu->PC += 2;
      return 10;

    case 0x02:    // ld (bc), a
      write_u8(cpu->main.pairs.BC, cpu->main.singles.A);
      return 7;

    case 0x03:    // inc bc
      cpu->main.pairs.BC += 1;
      return 6;
    
    case 0x04: {  // inc b
      u8 b = cpu->main.singles.B;
      u8 result = b+1;
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

    case 0x06:    // ld b,n
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
      cpu->main.singles.F.c = (hl + bc) > 0xFFFF;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = (hl & 0xF) + (bc & 0xF) > 0xF;
      return 11;
    }

    case 0x0A:    // ld a, (bc)
      cpu->main.singles.A = read_u8(cpu->main.pairs.BC);
      return 7;

    case 0x0B:   // dec bc
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

    case 0x0E:    // ld c,n
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

    case 0x10:    // djnz
      cpu->main.singles.B -= 1;
      if (cpu->main.singles.B != 0) {
        cpu->PC += (s8)read_u8(cpu->PC) + 1;
        return 13;
      }
      cpu->PC += 1;
      return 8;
    
    case 0x11:    // ld de, nn
      cpu->main.pairs.DE = read_u16(cpu->PC);
      cpu->PC += 2;
      return 10;
    
    case 0x12:    // ld (de), a
      write_u8(cpu->main.pairs.DE, cpu->main.singles.A);
      return 7;
    
    case 0x13:    // inc de
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

    case 0x16:    // ld d,n
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

    case 0x18:    // jr d
      cpu->PC += (s8)read_u8(cpu->PC) + 1;
      return 4;
    
    case 0x19: {  // add hl, de
      u16 hl = cpu->main.pairs.HL;
      u16 de = cpu->main.pairs.DE;
      u16 result = hl + de;
      cpu->main.singles.F.c = (hl + de) > 0xFFFF;
      cpu->main.singles.F.n = 0;
      cpu->main.singles.F.h = (hl & 0xF) + (de & 0xF) > 0xF;
      return 11;
    }

    case 0x1A:    // ld a, (de)
      cpu->main.singles.A = read_u8(cpu->main.pairs.DE);
      return 7;
    
    case 0x1B:    // dec de
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

    case 0x1E:    // ld e,n
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
      return 8;
    
    case 0x21:    // ld hl, nn
      cpu->main.pairs.HL = read_u16(cpu->PC);
      cpu->PC += 2;
      return 10;
    
    case 0x22:    // ld (nn), hl
      write_u8(read_u16(cpu->PC), cpu->main.singles.A);
      cpu->PC += 2;
      return 7;
    
    case 0x23:    // inc hl
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

    case 0x26:    // ld h,n
      cpu->main.singles.H = read_u8(cpu->PC++);
      return 7;


    

    default: {
      return -1;
    }

  }
  return 0;
}