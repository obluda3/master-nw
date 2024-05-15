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
    halted = false;
  }
}

int step(Emu *emu)
{
  int cpuTicks = execute_cpu(&halted);
  int machineTicks = cpuTicks * 3;
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
  return ~current_keys
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

void draw_instruction_panel()
{
  // décoder les instructions jusqu'à arriver à la position indiquée par  le scroll
  // a partir de là, enchainer les drawtext pour dessiner ttes les instructions
  // on peut utiliser le scissors au pire
}

void emu_loop(Emu *emu)
{
  bool quitting = false;
  int frame = 0;
  const float ticksPerFrame = 10738580 / 60;
  InitWindow(950, 600, "master-nw");

  Image screen = {
      .data = emu->vdp.framebuffer,
      .width = 256,
      .height = 192,
      .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
      .mipmaps = 1};
  Texture2D screenTex = LoadTextureFromImage(screen);

  bool textBoxRegAFEditMode = false;
  char textBoxRegAFText[128] = "";
  bool textBoxRegBCEditMode = false;
  char textBoxRegBCText[128] = "";
  bool textBoxRegDEEditMode = false;
  char textBoxRegDEText[128] = "";
  bool textBoxRegHLEditMode = false;
  char textBoxRegHLText[128] = "";
  bool textBoxRegAF2EditMode = false;
  char textBoxRegAF2Text[128] = "";
  bool textBoxRegBC2EditMode = false;
  char textBoxRegBC2Text[128] = "";
  bool textBoxRegDE2EditMode = false;
  char textBoxRegDE2Text[128] = "";
  bool textBoxRegHL2EditMode = false;
  char textBoxRegHL2Text[128] = "";
  bool textBoxRegIXEditMode = false;
  char textBoxRegIXText[128] = "";
  bool textBoxRegIYEditMode = false;
  char textBoxRegIYText[128] = "";
  bool textBoxRegSPEditMode = false;
  char textBoxRegSPText[128] = "";
  bool textBoxRegPCEditMode = false;
  char textBoxRegPCText[128] = "";
  Rectangle ScrollPanel027ScrollView = {0, 0, 0, 0};
  Vector2 ScrollPanel027ScrollOffset = {0, 0};
  Vector2 ScrollPanel027BoundsOffset = {0, 0};
  bool buttonPausePressed = false;
  bool buttonResumePressed = false;
  bool buttonStepPressed = false;

  bool paused = false;
  while (!quitting)
  {
    int currentFrameTicks = 0;

    set_input(get_input());
    while (currentFrameTicks < ticksPerFrame)
    {
      currentFrameTicks += step(emu);
    }
    UpdateTexture(screenTex, emu->vdp.framebuffer);

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawFPS(256, 0);

    GuiGroupBox((Rectangle){600, 50, 320, 500}, "INSTRUCTIONS");
    GuiPanel((Rectangle){50, 50, 514, 386}, NULL);
    if (GuiTextBox((Rectangle){648, 66, 48, 22}, textBoxRegAFText, 128, textBoxRegAFEditMode))
      textBoxRegAFEditMode = !textBoxRegAFEditMode;
    if (GuiTextBox((Rectangle){648, 98, 48, 22}, textBoxRegBCText, 128, textBoxRegBCEditMode))
      textBoxRegBCEditMode = !textBoxRegBCEditMode;
    if (GuiTextBox((Rectangle){648, 130, 48, 22}, textBoxRegDEText, 128, textBoxRegDEEditMode))
      textBoxRegDEEditMode = !textBoxRegDEEditMode;
    if (GuiTextBox((Rectangle){648, 162, 48, 22}, textBoxRegHLText, 128, textBoxRegHLEditMode))
      textBoxRegHLEditMode = !textBoxRegHLEditMode;
    if (GuiTextBox((Rectangle){752, 66, 48, 22}, textBoxRegAF2Text, 128, textBoxRegAF2EditMode))
      textBoxRegAF2EditMode = !textBoxRegAF2EditMode;
    if (GuiTextBox((Rectangle){752, 98, 48, 22}, textBoxRegBC2Text, 128, textBoxRegBC2EditMode))
      textBoxRegBC2EditMode = !textBoxRegBC2EditMode;
    if (GuiTextBox((Rectangle){752, 130, 48, 22}, textBoxRegDE2Text, 128, textBoxRegDE2EditMode))
      textBoxRegDE2EditMode = !textBoxRegDE2EditMode;
    if (GuiTextBox((Rectangle){752, 162, 48, 22}, textBoxRegHL2Text, 128, textBoxRegHL2EditMode))
      textBoxRegHL2EditMode = !textBoxRegHL2EditMode;
    if (GuiTextBox((Rectangle){856, 66, 48, 22}, textBoxRegIXText, 128, textBoxRegIXEditMode))
      textBoxRegIXEditMode = !textBoxRegIXEditMode;
    if (GuiTextBox((Rectangle){856, 98, 48, 22}, textBoxRegIYText, 128, textBoxRegIYEditMode))
      textBoxRegIYEditMode = !textBoxRegIYEditMode;
    if (GuiTextBox((Rectangle){856, 130, 48, 22}, textBoxRegSPText, 128, textBoxRegSPEditMode))
      textBoxRegSPEditMode = !textBoxRegSPEditMode;
    if (GuiTextBox((Rectangle){856, 162, 48, 22}, textBoxRegPCText, 128, textBoxRegPCEditMode))
      textBoxRegPCEditMode = !textBoxRegPCEditMode;
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
    GuiLabel((Rectangle){616, 210, 120, 24}, "MEMORY");
    GuiScrollPanel((Rectangle){616, 234, 288 - ScrollPanel027BoundsOffset.x, 302 - ScrollPanel027BoundsOffset.y}, NULL, (Rectangle){616, 234, 288, 302}, &ScrollPanel027ScrollOffset, &ScrollPanel027ScrollView);
    GuiGroupBox((Rectangle){48, 448, 512, 100}, "CONTROL");
    buttonPausePressed = GuiButton((Rectangle){72, 488, 120, 24}, "PAUSE");
    buttonResumePressed = GuiButton((Rectangle){240, 488, 120, 24}, "RESUME");
    buttonStepPressed = GuiButton((Rectangle){408, 488, 120, 24}, "STEP");

    DrawTextureEx(screenTex, (Vector2){51, 51}, 0, 2.0f, (Color){255, 255, 255, 255});

    EndDrawing();

    quitting = WindowShouldClose();
    frame++;
  }
}
#endif
