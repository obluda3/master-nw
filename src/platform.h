#pragma once
#include "types.h"
#ifdef TARGET_LINUX
#include <raylib.h>
#else
#include <eadk.h>
#endif

void draw_line(int y, u8* pixels);
u8 get_input();