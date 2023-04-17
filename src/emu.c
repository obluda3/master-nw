#include "emu.h"
#include <eadk.h>
#include <stdio.h>

void emu_loop(Emu* emu) {
  bool quitting = false;
  int frame = 0;
  const float ticksPerFrame = 10738580 / 60;
  while (!quitting) {
    int currentFrameTicks = 0;
    eadk_keyboard_state_t keys;
    u64 time = eadk_timing_millis();

    while (currentFrameTicks < ticksPerFrame) {
      char message[60];
      quitting = eadk_keyboard_key_down( eadk_keyboard_scan(), eadk_key_back);

      int cpuTicks = execute_cpu();
      int machineTicks = cpuTicks * 3;
      currentFrameTicks += machineTicks;
    }
    
    s32 elapsed = (s32)(eadk_timing_millis() - time);
    while (elapsed < 17) elapsed = eadk_timing_millis() - time;
    frame++;
  }
}