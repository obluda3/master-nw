#include "eadk.h"
#include <stdlib.h>
#include "emu.h"
#include "cpu.h"
#include <string.h>
#include <stdio.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "App";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

int main(int argc, char * argv[]) {
  Emu emu;
  init_cpu(&emu.cpu);
  emu_loop(&emu);
}
