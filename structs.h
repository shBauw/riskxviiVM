#include <stdint.h>

struct line {
    uint32_t bits;
};
struct R {
    char func7[7];
    char rs2[5];
    char rs1[5];
    char func3[3];
    char rd[5];
    char opcode[7];
    char break1;
};
struct I {
    char imm[12];
    char rs1[5];
    char func3[3];
    char rd[5];
    char opcode[7];
    char break1;
};
struct S {
    char imm2[7];
    char rs2[5];
    char rs1[5];
    char func3[3];
    char imm1[5];
    char opcode[7];
    char break1;
};
struct SB {
    char imm2[7];
    char rs2[5];
    char rs1[5];
    char func3[3];
    char imm1[5];
    char opcode[7];
    char break1;
};
struct U {
    char imm[20];
    char rd[5];
    char opcode[7];
    char break1;
};
struct UJ {
    char imm[20];
    char rd[5];
    char opcode[7];
    char break1;
};
struct opCode {
    char other[25];
    char opcode[7];
    char break1;
};
struct system {
    int reg[32];
    char inst_mem[32 * 8][33];
    char data_mem[1024 * 8 + 1];
    char heap_banks[128][64 * 8 + 1];
};