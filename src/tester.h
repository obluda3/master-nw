#include "types.h"
#include <stdio.h>
#include "emu.h"

typedef struct {
    u16 pc;
    u16 sp;
    u8 a;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
    u8 f;
    u8 h;
    u8 l;
    u8 i;
    u8 r;
    u16 ix;
    u16 iy;
    u16 af_;
    u16 bc_;
    u16 de_;
    u16 hl_;
    u16 iff1;
    u16 iff2;
} state_t;

void unit_test(Emu* emu, char* filename);

void test_case(Emu* emu, FILE* f);
bool check_state(FILE* f);
void set_state(FILE* f);
