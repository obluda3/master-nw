#include "tester.h"
#include <string.h>
#include "cpu.h"
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define RESET "\x1B[0m"

void unit_test(Emu *emu, char *filename)
{
    FILE *f = fopen(filename, "r");

    if (!f)
    {
        printf("File not found: %s\n", filename);
        return;
    }

    u16 count;
    fread(&count, 2, 1, f);
    for (int i = 0; i < count; i++)
    {
        printf("Test %d / %d: \n", i+1, count);
        test_case(emu, f);
    }
    printf("ok\n");
}

char *bool_to_string(bool val)
{
    return val ? GRN "OK" RESET : RED "NO" RESET;
}

void test_case(Emu *emu, FILE *f)
{
    state_t initial;
    fread(&initial, sizeof(state_t), 1, f);
    emu->cpu.PC = initial.pc;
    emu->cpu.SP = initial.sp;
    emu->cpu.main.singles.A = initial.a;
    emu->cpu.main.singles.B = initial.b;
    emu->cpu.main.singles.C = initial.c;
    emu->cpu.main.singles.D = initial.d;
    emu->cpu.main.singles.E = initial.e;
    *(u8 *)&emu->cpu.main.singles.F = initial.f;
    emu->cpu.main.singles.H = initial.h;
    emu->cpu.main.singles.L = initial.l;
    emu->cpu.I = initial.i;
    emu->cpu.R = initial.r;
    emu->cpu.IX = initial.ix;
    emu->cpu.IY = initial.iy;
    emu->cpu.alt.pairs.AF = initial.af_;
    emu->cpu.alt.pairs.BC = initial.bc_;
    emu->cpu.alt.pairs.DE = initial.de_;
    emu->cpu.alt.pairs.HL = initial.hl_;
    emu->cpu.FF1 = initial.iff1;
    emu->cpu.FF2 = initial.iff2;

    int ram_cnt;
    u16 address;
    u8 data;
    fread(&ram_cnt, 2, 1, f);
    for (int i = 0; i < ram_cnt; i++)
    {
        fread(&address, 2, 1, f);
        fread(&data, 1, 1, f);
        emu->mem.internal_memory[address] = data;
    }

    state_t final;
    fread(&final, sizeof(state_t), 1, f);
    char cpu_message[1024];
    char message[128];
    cpu_message[0] = '\0';
    bool halted = false;
    execute_cpu(&halted);

    sprintf(message, "PC:\t%s%d\t%s%d%s\n", RED, emu->cpu.PC, GRN, final.pc, RESET);
    if (emu->cpu.PC != final.pc)
        strcat(cpu_message, message);

    sprintf(message, "SP:\t%s%d\t%s%d%s\n", RED, emu->cpu.SP, GRN, final.sp, RESET);
    if (emu->cpu.SP != final.sp) strcat(cpu_message, message);

    sprintf(message, "A:\t%s%d\t%s%d%s\n", RED, emu->cpu.main.singles.A, GRN, final.a, RESET);
    if (emu->cpu.main.singles.A != final.a) strcat(cpu_message, message);

    sprintf(message, "B:\t%s%d\t%s%d%s\n", RED, emu->cpu.main.singles.B, GRN, final.b, RESET);
    if (emu->cpu.main.singles.B != final.b) strcat(cpu_message, message);

    sprintf(message, "C:\t%s%d\t%s%d%s\n", RED, emu->cpu.main.singles.C, GRN, final.c, RESET);
    if (emu->cpu.main.singles.C != final.c) strcat(cpu_message, message);

    sprintf(message, "D:\t%s%d\t%s%d%s\n", RED, emu->cpu.main.singles.D, GRN, final.d, RESET);
    if (emu->cpu.main.singles.D != final.d) strcat(cpu_message, message);
    
    sprintf(message, "E:\t%s%d\t%s%d%s\n", RED, emu->cpu.main.singles.E, GRN, final.e, RESET);
    if (emu->cpu.main.singles.E != final.e) strcat(cpu_message, message);

    sprintf(message, "F:\t%s%d\t%s%d%s\n", RED, *(u8*)&emu->cpu.main.singles.F, GRN, final.f, RESET);
    if (*(u8*)&emu->cpu.main.singles.F != final.f) strcat(cpu_message, message);

    sprintf(message, "H:\t%s%d\t%s%d%s\n", RED, emu->cpu.main.singles.H, GRN, final.h, RESET);
    if (emu->cpu.main.singles.H != final.h) strcat(cpu_message, message);

    sprintf(message, "L:\t%s%d\t%s%d%s\n", RED, emu->cpu.main.singles.L, GRN, final.l, RESET);
    if (emu->cpu.main.singles.L != final.l) strcat(cpu_message, message);

    sprintf(message, "I:\t%s%d\t%s%d%s\n", RED, emu->cpu.I, GRN, final.i, RESET);
    if (emu->cpu.I != final.i) strcat(cpu_message, message);

    sprintf(message, "R:\t%s%d\t%s%d%s\n", RED, emu->cpu.R, GRN, final.r, RESET);
    if (emu->cpu.R != final.r) strcat(cpu_message, message);

    sprintf(message, "IX:\t%s%d\t%s%d%s\n", RED, emu->cpu.IX, GRN, final.ix, RESET);
    if (emu->cpu.IX != final.ix) strcat(cpu_message, message);

    sprintf(message, "IY:\t%s%d\t%s%d%s\n", RED, emu->cpu.IY, GRN, final.iy, RESET);
    if (emu->cpu.IY != final.iy) strcat(cpu_message, message);

    sprintf(message, "AF_:\t%s%d\t%s%d%s\n", RED, emu->cpu.alt.pairs.AF, GRN, final.af_, RESET);
    if (emu->cpu.alt.pairs.AF != final.af_) strcat(cpu_message, message);

    sprintf(message, "BC_:\t%s%d\t%s%d%s\n", RED, emu->cpu.alt.pairs.BC, GRN, final.bc_, RESET);
    if (emu->cpu.alt.pairs.BC != final.bc_) strcat(cpu_message, message);

    sprintf(message, "DE_:\t%s%d\t%s%d%s\n", RED, emu->cpu.alt.pairs.DE, GRN, final.de_, RESET);
    if (emu->cpu.alt.pairs.DE != final.de_) strcat(cpu_message, message);

    sprintf(message, "HL_:\t%s%d\t%s%d%s\n", RED, emu->cpu.alt.pairs.HL, GRN, final.hl_, RESET);
    if (emu->cpu.alt.pairs.HL != final.hl_) strcat(cpu_message, message);

    sprintf(message, "IFF1:\t%s%d\t%s%d%s\n", RED, emu->cpu.FF1, GRN, final.iff1, RESET);
    if (emu->cpu.FF1 != final.iff1) strcat(cpu_message, message);

    sprintf(message, "IFF2:\t%s%d\t%s%d%s\n", RED, emu->cpu.FF2, GRN, final.iff2, RESET);
    if (emu->cpu.FF2 != final.iff2) strcat(cpu_message, message);

    char ram_message[1024];

    fread(&ram_cnt, 2, 1, f);
    for (int i = 0; i < ram_cnt; i++)
    {
        fread(&address, 2, 1, f);
        fread(&data, 1, 1, f);
        sprintf(message, "RAM %d:\t%s%d\t%s%d%s\n", address, RED, emu->mem.internal_memory[address], GRN, data, RESET);
        if (emu->mem.internal_memory[address] != data) strcat(ram_message, message);
    }
    
    if (strlen(cpu_message) || strlen(ram_message))
    {
        printf(RED "NOT OK\n" RESET);
        printf("%s", cpu_message);
        printf("%s", ram_message);
        getchar();
    }
    else
        printf(GRN "OK\n" RESET);
}