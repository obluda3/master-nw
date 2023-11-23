#include <stdlib.h>
#include "emu.h"
#include "cpu.h"
#include "vdp.h"
#include "platform.h"
#include <string.h>
#include <stdio.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "master-nw";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

int main(int argc, char * argv[]) {
  Emu emu;
  char* rom;
  #ifndef TARGET_LINUX
  rom = eadk_external_data;
  eadk_display_push_rect_uniform((eadk_rect_t){0, 0, 320, 240}, eadk_color_white);
  #else
  InitWindow(512, 392, "raylib [core] example - basic window");
  SetTargetFPS(60);
  BeginDrawing();
  FILE *file = fopen("/home/houza/Bureau/master-nw/rom.SMS", "rb");
  fseek(file, 0, SEEK_END);
	unsigned long len = ftell(file);
	fseek(file, 0, SEEK_SET);
  rom=(char *)malloc(len+1);
  fread(rom, len, 1, file);
	fclose(file);

  #endif

  init_cpu(&emu.cpu);
  init_mem(&emu.mem, rom);
  init_vdp(&emu.vdp);
  emu_loop(&emu);
}
