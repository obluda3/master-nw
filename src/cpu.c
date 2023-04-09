#include "cpu.h"
#include "mem.h"

u8 execute_cpu(Z80* cpu) {
  u8 inst = read_u8(cpu->PC++);
  switch (inst) {
    case 0x00:    // nop
      return 4;

    case 0x01:    // ld bc, nn
      cpu->main.pairs.BC = read_u8(cpu->PC++);
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

    

  }
  return 0;
}