#include "emu.h"
#include "cpu.h"
#include <stdio.h>
#include <string.h>

void emu_loop(Emu* emu) {
  bool quitting = false;
  bool slow = true;
  eadk_keyboard_state_t keys;
  int offs = 0x8000;
  while (!quitting) { 
    while (1) {
      keys = eadk_keyboard_scan();
      if (eadk_keyboard_key_down(keys, eadk_key_down)) offs += 8;
      if (eadk_keyboard_key_down(keys, eadk_key_up) && offs > 16) offs -= 8;
      if (eadk_keyboard_key_down(keys, eadk_key_right)) offs += 32;
      if (eadk_keyboard_key_down(keys, eadk_key_left) && offs > 32) offs -= 32;
      if (eadk_keyboard_key_down(keys, eadk_key_ok)) break;
      if (eadk_keyboard_key_down(keys, eadk_key_back)) slow = false;
      if (eadk_keyboard_key_down(keys, eadk_key_var)) slow = true;
      if (eadk_keyboard_key_down(keys, eadk_key_shift)) offs = emu->cpu.SP;
      if (eadk_keyboard_key_down(keys, eadk_key_alpha)) offs = emu->cpu.PC;
      if (!slow) break;
      char message[512];
      Registers *main = &emu->cpu.main;
      Registers *alt = &emu->cpu.alt;
      sprintf(message, "PC=%04x\nA=%02x F=%02x\tA'=%02x F'=%02x\nB=%02x C=%02x\tB'=%02x C'=%02x\nD=%02x E=%02x\tD'=%02x E'=%02x\nH=%02x L=%02x\tH'=%02x L'=%02x\nIX=%04x\tIY=%04x \nSP=%04x\tI=%02x R=%02x \n", emu->cpu.PC, main->singles.A, *(u8*)&main->singles.F, alt->singles.A, *(u8*)&alt->singles.F, 
                  main->singles.B, main->singles.C, alt->singles.B, alt->singles.C, 
                  main->singles.D, main->singles.E, alt->singles.D, alt->singles.E, 
                  main->singles.H, main->singles.L, alt->singles.H, alt->singles.L, 
                  emu->cpu.IX, emu->cpu.IY, emu->cpu.SP, emu->cpu.I, emu->cpu.R );
      eadk_display_draw_string(message, (eadk_point_t){0, 0}, false, eadk_color_black, eadk_color_white);

      char hexdump[256];
      char tmp[16];
      for (int i = 0; i < 10; i++) {
        int curOffs = offs + i * 8;
        sprintf(hexdump, "%04x: ", curOffs);
        for (int i = 0; i < 8; i++) {
          sprintf(tmp, "%02x ", read_u8(curOffs+i));
          strcat(hexdump, tmp);
        }
        strcat(hexdump, "  ");
        for (int i = 0; i < 8; i++) {
          char a = read_u8(curOffs+i);
          if (a < 0x20) a = '.';
          if (a > 125) a = '.';
          sprintf(tmp, "%c", a);
          strcat(hexdump, tmp);
        }
        eadk_display_draw_string(hexdump, (eadk_point_t){0, 100 + 14*i}, false, eadk_color_black, eadk_color_white);
      }
      
    }
    execute_cpu();
    if (slow) eadk_timing_msleep(500);
  }
}
/*
void loop(Emu* emu) {
  bool quitting = false;
  int frame = 0;
  const float ticksPerFrame = 10738580 / 60;
  while (!quitting) {
    int currentFrameTicks = 0;
    
    u64 time = eadk_timing_millis();
    eadk_keyboard_state_t keys = eadk_keyboard_scan();
    set_input(keys);
    while (currentFrameTicks < ticksPerFrame) {
      char message[60];
     
      int cpuTicks = execute_cpu(&emu->cpu);
      float vdpCycles = 
      int machineTicks = cpuTicks * 3;
      currentFrameTicks += machineTicks;
    }
    
    s32 elapsed = (s32)(eadk_timing_millis() - time);
    while (elapsed < 17) elapsed = eadk_timing_millis() - time;
    frame++;
  }
}

void check_interrupts(Emu* emu) {
  
}*/