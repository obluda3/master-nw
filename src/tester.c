#include "tester.h"
#include <string.h>
#include "cpu.h"
#define DBG_RED "\x1B[31m"
#define DBG_GRN "\x1B[32m"
#define DBG_YEL "\x1B[33m"
#define DBG_BLU "\x1B[34m"
#define DBG_RESET "\x1B[0m"

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
        printf("Test %d / %d: \n", i + 1, count);
        test_case(emu, f);
    }
    printf("ok\n");
}

char *bool_to_string(bool val)
{
    return val ? DBG_GRN "OK" DBG_RESET : DBG_RED "NO" DBG_RESET;
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

    sprintf(message, "PC:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.PC, DBG_GRN, final.pc, DBG_RESET);
    if (emu->cpu.PC != final.pc)
        strcat(cpu_message, message);

    sprintf(message, "SP:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.SP, DBG_GRN, final.sp, DBG_RESET);
    if (emu->cpu.SP != final.sp)
        strcat(cpu_message, message);

    sprintf(message, "A:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.main.singles.A, DBG_GRN, final.a, DBG_RESET);
    if (emu->cpu.main.singles.A != final.a)
        strcat(cpu_message, message);

    sprintf(message, "B:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.main.singles.B, DBG_GRN, final.b, DBG_RESET);
    if (emu->cpu.main.singles.B != final.b)
        strcat(cpu_message, message);

    sprintf(message, "C:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.main.singles.C, DBG_GRN, final.c, DBG_RESET);
    if (emu->cpu.main.singles.C != final.c)
        strcat(cpu_message, message);

    sprintf(message, "D:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.main.singles.D, DBG_GRN, final.d, DBG_RESET);
    if (emu->cpu.main.singles.D != final.d)
        strcat(cpu_message, message);

    sprintf(message, "E:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.main.singles.E, DBG_GRN, final.e, DBG_RESET);
    if (emu->cpu.main.singles.E != final.e)
        strcat(cpu_message, message);

    sprintf(message, "F:\t%s%d\t%s%d%s\n", DBG_RED, *(u8 *)&emu->cpu.main.singles.F, DBG_GRN, final.f, DBG_RESET);
    if (*(u8 *)&emu->cpu.main.singles.F != final.f)
        strcat(cpu_message, message);

    sprintf(message, "H:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.main.singles.H, DBG_GRN, final.h, DBG_RESET);
    if (emu->cpu.main.singles.H != final.h)
        strcat(cpu_message, message);

    sprintf(message, "L:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.main.singles.L, DBG_GRN, final.l, DBG_RESET);
    if (emu->cpu.main.singles.L != final.l)
        strcat(cpu_message, message);

    sprintf(message, "I:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.I, DBG_GRN, final.i, DBG_RESET);
    if (emu->cpu.I != final.i)
        strcat(cpu_message, message);

    sprintf(message, "R:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.R, DBG_GRN, final.r, DBG_RESET);
    if (emu->cpu.R != final.r)
        strcat(cpu_message, message);

    sprintf(message, "IX:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.IX, DBG_GRN, final.ix, DBG_RESET);
    if (emu->cpu.IX != final.ix)
        strcat(cpu_message, message);

    sprintf(message, "IY:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.IY, DBG_GRN, final.iy, DBG_RESET);
    if (emu->cpu.IY != final.iy)
        strcat(cpu_message, message);

    sprintf(message, "AF_:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.alt.pairs.AF, DBG_GRN, final.af_, DBG_RESET);
    if (emu->cpu.alt.pairs.AF != final.af_)
        strcat(cpu_message, message);

    sprintf(message, "BC_:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.alt.pairs.BC, DBG_GRN, final.bc_, DBG_RESET);
    if (emu->cpu.alt.pairs.BC != final.bc_)
        strcat(cpu_message, message);

    sprintf(message, "DE_:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.alt.pairs.DE, DBG_GRN, final.de_, DBG_RESET);
    if (emu->cpu.alt.pairs.DE != final.de_)
        strcat(cpu_message, message);

    sprintf(message, "HL_:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.alt.pairs.HL, DBG_GRN, final.hl_, DBG_RESET);
    if (emu->cpu.alt.pairs.HL != final.hl_)
        strcat(cpu_message, message);

    sprintf(message, "IFF1:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.FF1, DBG_GRN, final.iff1, DBG_RESET);
    if (emu->cpu.FF1 != final.iff1)
        strcat(cpu_message, message);

    sprintf(message, "IFF2:\t%s%d\t%s%d%s\n", DBG_RED, emu->cpu.FF2, DBG_GRN, final.iff2, DBG_RESET);
    if (emu->cpu.FF2 != final.iff2)
        strcat(cpu_message, message);

    char ram_message[1024];

    fread(&ram_cnt, 2, 1, f);
    for (int i = 0; i < ram_cnt; i++)
    {
        fread(&address, 2, 1, f);
        fread(&data, 1, 1, f);
        sprintf(message, "RAM %d:\t%s%d\t%s%d%s\n", address, DBG_RED, emu->mem.internal_memory[address], DBG_GRN, data, DBG_RESET);
        if (emu->mem.internal_memory[address] != data)
            strcat(ram_message, message);
    }

    if (strlen(cpu_message) || strlen(ram_message))
    {
        printf(DBG_RED "NOT OK\n" DBG_RESET);
        printf("%s", cpu_message);
        printf("%s", ram_message);
        getchar();
    }
    else
        printf(DBG_GRN "OK\n" DBG_RESET);
}