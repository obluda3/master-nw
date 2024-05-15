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

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

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

#ifndef TARGET_LINUX
u8 get_input() {
  u8 current_keys = 0;
  eadk_keyboard_state_t keyboardState = eadk_keyboard_scan();
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_up);
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_down) << 1;
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_left) << 2;
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_right) << 3;
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_ok) << 4;
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_back) << 5;
  return ~current_keys
}


void emu_loop(Emu* emu) {
  bool quitting = false;
  int frame = 0;
  const float ticksPerFrame = 10738580 / 60;
  while (!quitting) {
    int currentFrameTicks = 0;
    
    set_input(get_input());
    while (currentFrameTicks < ticksPerFrame) {
      currentFrameTicks += step(emu);
    }
    quitting = eadk_keyboard_key_down(eadk_keyboard_scan(), eadk_key_back);
    frame++;
  }
}
#else
u8 get_input() {
  u8 current_keys = 0;
  current_keys |= IsKeyDown(KEY_UP);
  current_keys |= IsKeyDown(KEY_DOWN) << 1;
  current_keys |= IsKeyDown(KEY_LEFT) << 2;
  current_keys |= IsKeyDown(KEY_RIGHT) << 3;
  current_keys |= IsKeyDown(KEY_W) << 4;
  current_keys |= IsKeyDown(KEY_X) << 5;
  return ~current_keys;
}


void draw_instruction_panel() {
  // décoder les instructions jusqu'à arriver à la position indiquée par  le scroll
  // a partir de là, enchainer les drawtext pour dessiner ttes les instructions
  // on peut utiliser le scissors au pire
  
}

void emu_loop(Emu* emu) {
  bool quitting = false;
  int frame = 0;
  const float ticksPerFrame = 10738580 / 60;
  InitWindow(950, 600, "master-nw");

  Image screen = {
      .data = emu->vdp.framebuffer,
      .width = 256,
      .height = 192,
      .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
      .mipmaps = 1
    };
  Texture2D screenTex = LoadTextureFromImage(screen);

  bool textBoxSeekEditMode = false;
  char textBoxSeekText[128] = "0000";
  Rectangle instructionListScrollView = { 0, 0, 0, 0 };
  Vector2 instructionListScrollOffset = { 0, 0 };
  Vector2 instructionListBoundsOffset = { 0, 0 };

  while (!quitting) {
    int currentFrameTicks = 0;
    
    set_input(get_input());
    while (currentFrameTicks < ticksPerFrame) {
      currentFrameTicks += step(emu);
    }
    UpdateTexture(screenTex, emu->vdp.framebuffer);

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawFPS(256, 0);

    GuiGroupBox((Rectangle){ 600, 50, 300, 500 }, "INSTRUCTIONS");
    if (GuiTextBox((Rectangle){ 760, 70, 120, 24 }, textBoxSeekText, 128, textBoxSeekEditMode)) textBoxSeekEditMode = !textBoxSeekEditMode;
    GuiLabel((Rectangle){ 620, 70, 120, 24 }, "SEEK");
    GuiScrollPanel((Rectangle){ 620, 110, 260 - instructionListBoundsOffset.x, 392 - instructionListBoundsOffset.y }, NULL, (Rectangle){ 620, 110, 260, 500 }, &instructionListScrollOffset, &instructionListScrollView);
    GuiPanel((Rectangle){ 50, 50, 514, 386 }, NULL);

    DrawTextureEx(screenTex, (Vector2){51, 51},0, 2.0f, (Color){255,255,255,255});


    EndDrawing();

    quitting = WindowShouldClose();
    frame++;
  }
}
#endif
