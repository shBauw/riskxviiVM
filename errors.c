#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "conversions.h"

void reg_dump(int program_counter, int reg[]) {
    printf("PC = 0x");
    hexadecimal(program_counter * 4, 8);
    for (int k = 0; k < 32; k++) {
        printf(";\nR[%d] = ", k);
        printf("0x");
        hexadecimal(reg[k], 8);
    }
    printf(";\n");
}
void error(char operation[], int pc, int reg[]) {
    hexadecimal(decimalConvert(operation, 32, 1), 8);
    printf("\n");
    reg_dump(pc, reg);
}
void illegalOperation(char operation[], int pc, int reg[]) {
    printf("Illegal Operation: 0x");
    error(operation, pc, reg);
}
void notImplemented(char operation[], int pc, int reg[]) {
    printf("Instruction Not Implemented: 0x");
    error(operation, pc, reg);
}