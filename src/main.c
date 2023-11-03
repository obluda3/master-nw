#include "eadk.h"
#include <stdlib.h>
#include "emu.h"
#include "cpu.h"
#include "vdp.h"
#include <string.h>
#include <stdio.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "App";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

int main(int argc, char * argv[]) {
  Emu emu;
  eadk_display_push_rect_uniform((eadk_rect_t){0, 0, 320, 240}, eadk_color_white);
  init_cpu(&emu.cpu);
  init_mem(&emu.mem, eadk_external_data);
  init_vdp(&emu.vdp);
  emu_loop(&emu);
}
