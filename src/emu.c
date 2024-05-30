#include "emu.h"
#include "cpu.h"
#include <time.h>
#include <stdio.h>
#include "io.h"
#include <stdlib.h>
#include "vdp.h"
#include "platform.h"
#include <string.h>


bool halted = false;
void handle_interrupts(Emu *emu, bool reset)
{
  if (emu->cpu.FF2 && reset)
  {
    emu->cpu.FF1 = false;
    emu->cpu.SP -= 2;
    u16 pc = emu->cpu.PC;
    if (halted)
      pc += 1;
    write_u16(emu->cpu.SP, pc);
    emu->cpu.PC = 0x66;
    halted = false;
  }
  else if (emu->cpu.FF1 && vdp_is_interrupt())
  {
    emu->cpu.SP -= 2;
    u16 pc = emu->cpu.PC;
    if (halted)
      pc += 1;
    write_u16(emu->cpu.SP, pc);
    emu->cpu.PC = 0x38;
    emu->cpu.FF1 = false;
    emu->cpu.FF2 = false;
    halted = false;
  }
}

int step(Emu *emu)
{
  int cpuTicks = execute_cpu(&halted);
  int machineTicks = 20 * 3;
  float vdpCycles = machineTicks / 2;
  handle_interrupts(emu, false);
  vdp_update(vdpCycles);
  return machineTicks;
}

#ifndef TARGET_LINUX
u8 get_input()
{
  u8 current_keys = 0;
  eadk_keyboard_state_t keyboardState = eadk_keyboard_scan();
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_up);
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_down) << 1;
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_left) << 2;
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_right) << 3;
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_ok) << 4;
  current_keys |= eadk_keyboard_key_down(keyboardState, eadk_key_back) << 5;
  return ~current_keys;
}

void emu_loop(Emu *emu)
{
  bool quitting = false;
  int frame = 0;
  const float ticksPerFrame = 10738580 / 60;
  while (!quitting)
  {
    int currentFrameTicks = 0;

    set_input(get_input());
    while (currentFrameTicks < ticksPerFrame)
    {
      currentFrameTicks += step(emu);
    }
    quitting = eadk_keyboard_key_down(eadk_keyboard_scan(), eadk_key_back);
    frame++;
  }
}
#else

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#undef RAYGUI_IMPLEMENTATION
#include <style_cyber.h>
u8 get_input()
{
  u8 current_keys = 0;
  current_keys |= IsKeyDown(KEY_UP);
  current_keys |= IsKeyDown(KEY_DOWN) << 1;
  current_keys |= IsKeyDown(KEY_LEFT) << 2;
  current_keys |= IsKeyDown(KEY_RIGHT) << 3;
  current_keys |= IsKeyDown(KEY_W) << 4;
  current_keys |= IsKeyDown(KEY_X) << 5;
  return ~current_keys;
}

void mem_dump()
{
  FILE *file = fopen("mem.bin", "wb");

  char memory[0x10000];
  for (int i = 0; i < 0x10000; i++)
    memory[i] = read_u8(i);

  fwrite(memory, 0x4000, 4, file);
  fclose(file);
}

void emu_loop(Emu *emu)
{
  bool quitting = false;
  int frame = 0;
  const float ticksPerFrame = 10738580 / 60;
  InitWindow(950, 600, "master-nw");
  SetTargetFPS(200);
  GuiLoadStyleCyber();
  Image screen = {
      .data = emu->vdp.framebuffer,
      .width = 256,
      .height = 192,
      .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
      .mipmaps = 1};
  Texture2D screenTex = LoadTextureFromImage(screen);

  char regAFText[128] = "";
  char regBCText[128] = "";
  char regDEText[128] = "";
  char regHLText[128] = "";
  char regAF2Text[128] = "";
  char regBC2Text[128] = "";
  char regDE2Text[128] = "";
  char regHL2Text[128] = "";
  char regIXText[128] = "";
  char regIYText[128] = "";
  char regSPText[128] = "";
  char regPCText[128] = "";

  bool paused = false;
  Rectangle ScrollPanel027ScrollView = {0, 0, 0, 0};
  Vector2 ScrollPanel027ScrollOffset = {0, 0};
  Vector2 ScrollPanel027BoundsOffset = {0, 0};
  bool buttonStepPressed = false;
  bool buttonPausePressed = false;
  bool fastForward = false;
  int currentFrameTicks = 0;
  while (!quitting)
  {
    if (paused) 
    {
      if (buttonStepPressed) 
      {
        currentFrameTicks += step(emu);
        if (currentFrameTicks >= ticksPerFrame) 
        {
          frame++;
          currentFrameTicks = 0;
          UpdateTexture(screenTex, emu->vdp.framebuffer);
        }
      }
      if (buttonStepPressed || buttonPausePressed) 
      {
        sprintf(regAFText, "%04X", emu->cpu.main.pairs.AF);
        sprintf(regBCText, "%04X", emu->cpu.main.pairs.BC);
        sprintf(regDEText, "%04X", emu->cpu.main.pairs.DE);
        sprintf(regHLText, "%04X", emu->cpu.main.pairs.HL);

        sprintf(regAF2Text, "%04X", emu->cpu.alt.pairs.AF);
        sprintf(regBC2Text, "%04X", emu->cpu.alt.pairs.BC);
        sprintf(regDE2Text, "%04X", emu->cpu.alt.pairs.DE);
        sprintf(regHL2Text, "%04X", emu->cpu.alt.pairs.HL);

        sprintf(regSPText, "%04X", emu->cpu.SP);
        sprintf(regPCText, "%04X", emu->cpu.PC);
        sprintf(regIXText, "%04X", emu->cpu.IX);
        sprintf(regIYText, "%04X", emu->cpu.IY);
      }
    }
    else if (!paused)
    {
      currentFrameTicks = 0;
      set_input(get_input());
      while (currentFrameTicks < ticksPerFrame)
      {
        currentFrameTicks += step(emu);
      }
      UpdateTexture(screenTex, emu->vdp.framebuffer);
      frame++;
    }

    if (IsKeyPressed(KEY_TAB)) {
      fastForward = !fastForward;
      int fps = fastForward ? 1000 : 200;
      SetTargetFPS(fps);
    }

    
    BeginDrawing();
    ClearBackground(RAYWHITE);

    GuiPanel((Rectangle){-1, -1, 952, 602}, NULL);
    GuiGroupBox((Rectangle){600, 50, 320, 500}, "INSTRUCTIONS");
    GuiPanel((Rectangle){50, 50, 514, 386}, NULL);
    GuiGroupBox((Rectangle){50, 448, 514, 100}, "CONTROL");

    GuiLabel((Rectangle){648, 66, 48, 22}, regAFText);
    GuiLabel((Rectangle){648, 98, 48, 22}, regBCText);
    GuiLabel((Rectangle){648, 130, 48, 22}, regDEText);
    GuiLabel((Rectangle){648, 162, 48, 22}, regHLText);
    GuiLabel((Rectangle){752, 66, 48, 22}, regAF2Text);
    GuiLabel((Rectangle){752, 98, 48, 22}, regBC2Text);
    GuiLabel((Rectangle){752, 130, 48, 22}, regDE2Text);
    GuiLabel((Rectangle){752, 162, 48, 22}, regHL2Text);
    GuiLabel((Rectangle){856, 66, 48, 22}, regIXText);
    GuiLabel((Rectangle){856, 98, 48, 22}, regIYText);
    GuiLabel((Rectangle){856, 130, 48, 22}, regSPText);
    GuiLabel((Rectangle){856, 162, 48, 22}, regPCText);
    GuiLabel((Rectangle){616, 66, 56, 22}, "AF");
    GuiLabel((Rectangle){616, 98, 56, 22}, "BC");
    GuiLabel((Rectangle){616, 130, 56, 22}, "DE");
    GuiLabel((Rectangle){616, 162, 56, 22}, "HL");
    GuiLabel((Rectangle){720, 66, 56, 22}, "AF'");
    GuiLabel((Rectangle){720, 98, 56, 22}, "BC'");
    GuiLabel((Rectangle){720, 130, 56, 22}, "DE'");
    GuiLabel((Rectangle){720, 162, 56, 22}, "HL'");
    GuiLabel((Rectangle){824, 66, 56, 22}, "IX");
    GuiLabel((Rectangle){824, 98, 56, 22}, "IY");
    GuiLabel((Rectangle){824, 130, 56, 22}, "SP");
    GuiLabel((Rectangle){824, 162, 56, 22}, "PC");

    buttonPausePressed = GuiButton((Rectangle){72, 476, 120, 24}, "PAUSE");
    if (buttonPausePressed) 
    {
      paused = true;
    }
    if (GuiButton((Rectangle){240, 476, 120, 24}, "RESUME")) 
    {
      paused = false;
    }
    buttonStepPressed = GuiButton((Rectangle){408, 476, 120, 24}, "STEP");

    if (GuiButton((Rectangle){240, 512, 120, 24}, "dump"))
      mem_dump(emu);

    

    DrawTextureEx(screenTex, (Vector2){51, 51}, 0, 2.0f, (Color){255, 255, 255, 255});
    DrawFPS(0,0);
    EndDrawing();
    quitting = WindowShouldClose();
  }
}
#endif
