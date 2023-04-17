#pragma once
#include "emu.h"

void init_cpu(Z80* CPU);
int execute_cpu(); // return cycle number
