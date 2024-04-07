#include "emu.h"
#include "cpu.h"
#include "time.h"
#include <stdio.h>
#include "io.h"
#include <stdlib.h>
#include "vdp.h"
#include "platform.h"
#include <string.h>
#include "inst.h"

bool halted = false;
void handle_interrupts(Emu* emu, bool reset) {
  if (emu->cpu.FF2 && reset) {
    emu->cpu.FF1 = false;
    emu->cpu.SP -= 2;
    u16 pc = emu->cpu.PC;
    if (halted) pc += 1;
    write_u16(emu->cpu.SP, pc);
    emu->cpu.PC = 0x66;
    halted = false;
  }
  else if (emu->cpu.FF1 && vdp_is_interrupt()) {
    emu->cpu.SP -= 2;
    u16 pc = emu->cpu.PC;
    if (halted) pc += 1;
    write_u16(emu->cpu.SP, pc);
    emu->cpu.PC = 0x38;
    halted = false;
  }
}

int step(Emu* emu) {
  int cpuTicks = execute_cpu(&halted);
  int machineTicks = cpuTicks * 3;
  float vdpCycles = machineTicks / 2;
  handle_interrupts(emu, false);
  vdp_update(vdpCycles);
  return machineTicks;
}

#ifdef DEBUG
#include "inst.h"
char* get_inst(Emu* emu) {
  char* op;
  u8 opcode = read_u8(emu->cpu.PC);
  op = op_names[opcode];
  if (opcode == 0xCB) op = cb_names[read_u8(emu->cpu.PC+1)];
  else if (opcode == 0xDD) {
    u8 op2 = read_u8(emu->cpu.PC+1);
    if (op2 != 0xCB) op = dd_names[op2];
    else op = ddcb_names[read_u8(emu->cpu.PC+2)];
  }
  else if (opcode == 0xFD) {
    u8 op2 = read_u8(emu->cpu.PC+1);
    if (op2 != 0xCB) op = fd_names[op2];
    else op = fdcb_names[read_u8(emu->cpu.PC+2)];
  }
  else if (opcode == 0xED) {
    u8 op2 = read_u8(emu->cpu.PC+1);
    op = ed_names[op2];
  }
  return op;
}

int get_breakpoint() {
  char input[64] = {0};
  for (int i = 0; i < 64; i++) input[i] = '\0';
  input[0] = '0';
  int len = 1;
  #define PUSHCHAR(c) input[len] = c; len++; eadk_timing_msleep(250);
  while (1) {
    eadk_keyboard_state_t keys = eadk_keyboard_scan();
    if (eadk_keyboard_key_down(keys, eadk_key_exe)) {
      eadk_timing_millis(500);
      return (int)strtol(input, NULL, 16);
    }
    if (eadk_keyboard_key_down(keys, eadk_key_back)) {
      return -1;
    }
    if (eadk_keyboard_key_down(keys, eadk_key_backspace) && len > 0) {
        input[len-1] = ' ';
        eadk_display_draw_string(input, (eadk_point_t){100, 115}, false, eadk_color_white, eadk_color_black);
        input[len-1] = '\0';
        len--;
        eadk_timing_msleep(250);
    }

    if (len < 8) {
      if (eadk_keyboard_key_down(keys, eadk_key_zero)) { PUSHCHAR('0') };
      if (eadk_keyboard_key_down(keys, eadk_key_one)) { PUSHCHAR('1') };
      if (eadk_keyboard_key_down(keys, eadk_key_two)) { PUSHCHAR('2') };
      if (eadk_keyboard_key_down(keys, eadk_key_three)) { PUSHCHAR('3') };
      if (eadk_keyboard_key_down(keys, eadk_key_four)) { PUSHCHAR('4') };
      if (eadk_keyboard_key_down(keys, eadk_key_five)) { PUSHCHAR('5') };
      if (eadk_keyboard_key_down(keys, eadk_key_six)) { PUSHCHAR('6') };
      if (eadk_keyboard_key_down(keys, eadk_key_seven)) { PUSHCHAR('7') };
      if (eadk_keyboard_key_down(keys, eadk_key_eight)) { PUSHCHAR('8') };
      if (eadk_keyboard_key_down(keys, eadk_key_nine)) { PUSHCHAR('9') };
      if (eadk_keyboard_key_down(keys, eadk_key_exp)) { PUSHCHAR('A') };
      if (eadk_keyboard_key_down(keys, eadk_key_ln)) { PUSHCHAR('B') };
      if (eadk_keyboard_key_down(keys, eadk_key_log)) { PUSHCHAR('C') };
      if (eadk_keyboard_key_down(keys, eadk_key_imaginary)) { PUSHCHAR('D') };
      if (eadk_keyboard_key_down(keys, eadk_key_comma)) { PUSHCHAR('E') };
      if (eadk_keyboard_key_down(keys, eadk_key_power)) { PUSHCHAR('F') };
    }

    
    eadk_display_draw_string("bp addr", (eadk_point_t){100, 90}, false, eadk_color_white, eadk_color_black);
    eadk_display_draw_string(input, (eadk_point_t){100, 115}, false, eadk_color_white, eadk_color_black);

  }
}

void emu_loop(Emu* emu) {
  bool quitting = false;
  bool vram = false;
  eadk_keyboard_state_t keys;
  int offs = 0;
  
  while (!quitting) { 
    while (1) {
      keys = eadk_keyboard_scan();
      if (eadk_keyboard_key_down(keys, eadk_key_down)) offs += 8;
      if (eadk_keyboard_key_down(keys, eadk_key_up) && offs >= 16) offs -= 8;
      if (eadk_keyboard_key_down(keys, eadk_key_right)) offs += 32;
      if (eadk_keyboard_key_down(keys, eadk_key_left) && offs >= 32) offs -= 32;
      if (eadk_keyboard_key_down(keys, eadk_key_ok)) break;
      if (eadk_keyboard_key_down(keys, eadk_key_var)) { vram = !vram; eadk_timing_msleep(500); }
      if (eadk_keyboard_key_down(keys, eadk_key_shift)) offs = emu->cpu.SP;
      if (eadk_keyboard_key_down(keys, eadk_key_alpha)) offs = emu->cpu.PC;
      if (eadk_keyboard_key_down(keys, eadk_key_exp)) {
        int addr = get_breakpoint();
        if (addr != -1) {
          while (emu->cpu.PC != addr) step(emu);
        }
        eadk_timing_millis(500);
        continue;
      }
      char message[512];
      Registers *main = &emu->cpu.main;
      Registers *alt = &emu->cpu.alt;
      sprintf(message, "PC=%04x   %s\nA=%02x F=%02x\tA'=%02x F'=%02x\nB=%02x C=%02x\tB'=%02x C'=%02x\nD=%02x E=%02x\tD'=%02x E'=%02x\nH=%02x L=%02x\tH'=%02x L'=%02x\nIX=%04x\tIY=%04x \nSP=%04x\tI=%02x R=%02x \n", emu->cpu.PC, get_inst(emu), main->singles.A, *(u8*)&main->singles.F, alt->singles.A, *(u8*)&alt->singles.F, 
                  main->singles.B, main->singles.C, alt->singles.B, alt->singles.C, 
                  main->singles.D, main->singles.E, alt->singles.D, alt->singles.E, 
                  main->singles.H, main->singles.L, alt->singles.H, alt->singles.L, 
                  emu->cpu.IX, emu->cpu.IY, emu->cpu.SP, emu->cpu.I, emu->cpu.R );
      eadk_display_draw_string(message, (eadk_point_t){0, 0}, false, eadk_color_black, eadk_color_white);

      char hexdump[256];
      char tmp[16];
      for (int i = 0; i < 10; i++) {
        u16 curOffs = offs + i*8;
        sprintf(hexdump, "%04x: ", curOffs);
        for (int j = 0; j < 8; j++) {
          u8 x = read_u8(j + offs + i*8);
          if (vram) x = emu->vdp.vram[j + offs + i*8];
          sprintf(tmp, "%02x ", x);
          strcat(hexdump, tmp);
        }
        
        strcat(hexdump, "  ");
        for (int j = 0; j < 8; j++) {
          char a = read_u8(j + offs + i*8);
          if (vram) a = emu->vdp.vram[j + offs + i*8];
          if (a < 0x20) a = '.';
          if (a > 125) a = '.';
          sprintf(tmp, "%c", a);
          strcat(hexdump, tmp);
        }
        eadk_display_draw_string(hexdump, (eadk_point_t){0, 100 + 14*i}, false, eadk_color_black, eadk_color_white);
      }
      
    }
    step(emu);
    eadk_timing_msleep(500);
  }
}
#else 

u8 get_input() {
  u8 current_keys = 0;
  #ifndef TARGET_LINUX
  eadk_keyboard_state_t keyboardState = eadk_keyboard_scan();
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_up);
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_down) << 1;
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_left) << 2;
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_right) << 3;
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_ok) << 4;
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_back) << 5;
  #else
  current_keys |= IsKeyDown(KEY_UP);
  current_keys |= IsKeyDown(KEY_DOWN) << 1;
  current_keys |= IsKeyDown(KEY_LEFT) << 2;
  current_keys |= IsKeyDown(KEY_RIGHT) << 3;
  current_keys |= IsKeyDown(KEY_W) << 4;
  current_keys |= IsKeyDown(KEY_X) << 5;
  #endif
  return ~current_keys;
}

void emu_loop(Emu* emu) {
  bool quitting = false;
  int frame = 0;
  const float ticksPerFrame = 10738580 / 60;
  while (!quitting) {
    int currentFrameTicks = 0;
    
    set_input(get_input());
    bool deb = false;
    int target = 0x43;
    while (currentFrameTicks < ticksPerFrame) {
      currentFrameTicks += step(emu);
    }
    #ifdef TARGET_LINUX
    quitting = WindowShouldClose();
    #else
    quitting = eadk_keyboard_key_down(eadk_keyboard_scan(), eadk_key_back);
    #endif  
    frame++;
  }
}
#endif


void check_interrupts(Emu* emu) {
  
}