#pragma once
#include "emu.h"

void init_cpu(Z80* cpu);
int execute_cpu(bool* halted); // return cycle number

