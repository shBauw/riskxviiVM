#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// My commands
#include "structs.h"
#include "conversions.h"
#include "errors.h"

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;

    FILE *file = NULL;
    file = fopen(argv[1], "rb");
    if (file == NULL) {
        printf("File not found\n");
        return 1;
    }

    // Initialise system:
    struct system sys;
    sys = (const struct system){0};
    // Use '-' to show unallocated space
    for (int j = 0; j < 128; j++) {
        for (int k = 0; k < 64 * 8; k++) {
            sys.heap_banks[j][k] = '-';
        }
    }

    int counter = 0;
    struct line current;
    while (fread(&current, 1, sizeof(struct line), file) > 0) {
        char *binaryNum;
        binaryNum = binaryConvert(current.bits, 32);
        if (counter < 32 * 8) {
            strncpy(sys.inst_mem[counter], binaryNum, 32);
        } else {
            char temp[33];
            int offset = 32;
            for (int k = 0; k < 32; k++) {
                if (k % 8 == 0) offset = offset - 8;
                temp[k] = binaryNum[offset + (k%8)];
            }
            strncpy(&sys.data_mem[(counter - (32 * 8)) * 32], temp, 32);
        }

        counter++;
        free(binaryNum);
    }
    fclose(file);

    struct opCode opcode;
    struct R R;
    struct I I;
    struct S S;
    struct SB SB;
    struct U U;
    struct UJ UJ;

    // Program Counter
    int i;
    int notImp = 0;
    for (i = 0; i < 32 * 8; ) {
        opcode = (const struct opCode){0};
        R = (const struct R){0};
        I = (const struct I){0};
        S = (const struct S){0};
        SB = (const struct SB){0};
        U = (const struct U){0};
        UJ = (const struct UJ){0};
        
        char inst[33] = {0};
        strncpy(inst, sys.inst_mem[i], 32);
        sscanf(inst, "%s%s", opcode.other, opcode.opcode);

        if (strncmp("0110011", opcode.opcode, 7) == 0) {// Logic
            sscanf(inst, "%s%s%s%s%s%s", R.func7, R.rs2, R.rs1, R.func3, R.rd, R.opcode);
            int rd = decimalConvert(R.rd, 5, 1);
            int rs1 = decimalConvert(R.rs1, 5, 1);
            int rs2 = decimalConvert(R.rs2, 5, 1);

            if (rd > 31 || rs1 > 31 || rs2 > 31) {
                illegalOperation(inst, i, sys.reg);
                return 1;
            }
            if (rd == 0) continue;

            char* string1 = binaryConvert(sys.reg[rs1], 32);
            char* string2 = binaryConvert(sys.reg[rs2], 32);
            int shift = sys.reg[rs2];
            char temp[32];
            int np;

            if (strncmp(R.func3,"000", 3) == 0) {
                if (strncmp(R.func7,"0000000", 7) == 0) {// add
                    sys.reg[rd] = sys.reg[rs1] + sys.reg[rs2];
                } else if (strncmp(R.func7, "0100000", 7) == 0) {// sub
                    sys.reg[rd] = sys.reg[rs1] - sys.reg[rs2];
                } else notImp = 1;
            } else if (strncmp(R.func3,"001", 3) == 0 && strncmp(R.func7,"0000000", 7) == 0) {// sll
                for (int k = 0; k < 32; k++) {
                    np = k + shift;
                    if (np > 31) temp[k] = '0';
                    else temp[k] = string1[np];
                }
                sys.reg[rd] = decimalConvert(temp, 32, 0);
            } else if (strncmp(R.func3,"100", 3) == 0 && strncmp(R.func7,"0000000", 7) == 0) {// xor
                for (int k = 0; k < 32; k++) {
                    if (string1[k] == string2[k]) {
                        temp[k] = '0';
                    } else temp[k] = '1';
                }
                sys.reg[rd] = decimalConvert(temp, 32, 0);
            } else if (strncmp(R.func3,"101", 3) == 0) {
                if (strncmp(R.func7,"0000000", 7) == 0) {// srl
                    for (int k = 0; k < 32; k++) {
                        np = k - shift;
                        if (np < 0) temp[k] = '0';
                        else temp[k] = string1[np];
                    }
                    sys.reg[rd] = decimalConvert(temp, 32, 0);
                } else if (strncmp(R.func7,"0100000", 7) == 0) {// sra
                    for (int k = 0; k < 32; k++) {
                        np = k - shift;
                        temp[k] = string1[np % 32];
                    }
                    sys.reg[rd] = decimalConvert(temp, 32, 0);
                } else notImp = 1;
            } else if (strncmp(R.func3,"110", 3) == 0 && strncmp(R.func7,"0000000", 7) == 0) {// or
                for (int k = 0; k < 32; k++) {
                    if (string1[k] == '1' || string2[k] == '1') {
                        temp[k] = '1';
                    } else temp[k] = '0';
                }
                sys.reg[rd] = decimalConvert(temp, 32, 0);
            } else if (strncmp(R.func3,"111", 3) == 0 && strncmp(R.func7,"0000000", 7) == 0) {// and
                for (int k = 0; k < 32; k++) {
                    if (string1[k] == '1' && string2[k] == '1') {
                        temp[k] = '1';
                    } else temp[k] = '0';
                }
                sys.reg[rd] = decimalConvert(temp, 32, 0);
            } else if (strncmp(R.func3,"010", 3) == 0 && strncmp(R.func7,"0000000", 7) == 0) {// slt
                if (sys.reg[rs1] < sys.reg[rs2]) sys.reg[rd] = 1;
                else sys.reg[rd] = 0;
            } else if (strncmp(R.func3,"011", 3) == 0 && strncmp(R.func7,"0000000", 7) == 0) {// sltu
                if ((uint32_t) sys.reg[rs1] < (uint32_t) sys.reg[rs2]) sys.reg[rd] = 1;
                else sys.reg[rd] = 0;
            } else notImp = 1;

            free(string1);
            free(string2);
        } else if (strncmp("0010011", opcode.opcode, 7) == 0) {// Immediate logic
            sscanf(inst, "%s%s%s%s%s", I.imm, I.rs1, I.func3, I.rd, I.opcode);
            int rs1 = decimalConvert(I.rs1, 5, 1);
            int rd = decimalConvert(I.rd, 5, 1);
            if (rs1 > 31 || rd > 31) {
                illegalOperation(inst, i, sys.reg);
                return 1;
            }
            if (rd == 0) continue;

            char* string1 = binaryConvert(sys.reg[decimalConvert(I.rs1, 5, 1)], 32);
            char temp[32];
            for (int k = 0; k < 32; k++) {
                if (k < 20) temp[k] = I.imm[0];
                else temp[k] = I.imm[k - 20];
            }

            if (strncmp(I.func3,"000", 3) == 0) {// addi
                sys.reg[rd] = sys.reg[rs1] + decimalConvert(I.imm, 12, 0);
            } else if (strncmp(I.func3,"100", 3) == 0) {// xori
                for (int k = 0; k < 32; k++) {
                    if (temp[k] == string1[k]) temp[k] = '0';
                    else temp[k] = '1';
                }

                sys.reg[rd] = decimalConvert(temp, 32, 0);
            } else if (strncmp(I.func3,"110", 3) == 0) {// ori
                for (int k = 0; k < 32; k++) {
                    if (string1[k] == '1' || temp[k] == '1') temp[k] = '1';
                    else temp[k] = '0';
                }

                sys.reg[rd] = decimalConvert(temp, 32, 0);
            } else if (strncmp(I.func3,"111", 3) == 0) {// andi
                for (int k = 0; k < 32; k++) {
                    if (string1[k] == '1' && temp[k] == '1') temp[k] = '1';
                    else temp[k] = '0';
                }

                sys.reg[rd] = decimalConvert(temp, 32, 0);
            } else if (strncmp(I.func3,"010", 3) == 0) {// slti
                if (sys.reg[rs1] < decimalConvert(I.imm, 12, 0)) sys.reg[rd] = 1;
                else sys.reg[rd] = 0;
            } else if (strncmp(I.func3,"011", 3) == 0) {// sltiu
                if ((uint32_t) sys.reg[rs1] < decimalConvert(temp, 32, 1)) sys.reg[rd] = 1;
                else sys.reg[rd] = 0;
            } else notImp = 1;
            free(string1);
        } else if (strncmp("0110111", opcode.opcode, 7) == 0) {// lui
            sscanf(inst, "%s%s%s", U.imm, U.rd, U.opcode);
            int rd = decimalConvert(U.rd, 5, 1);
            if (rd > 31) {
                illegalOperation(inst, i, sys.reg);
                return 1;
            }
            if (rd == 0) continue;

            char string[33] = {0};
            for (int k = 0; k < 32; k++) {
                if (k < 20) string[k] = U.imm[k];
                else string[k] = '0';
            }
            sys.reg[rd] = decimalConvert(string, 32, 0);
        } else if (strncmp("0000011", opcode.opcode, 7) == 0) {// Loading
            sscanf(inst, "%s%s%s%s%s", I.imm, I.rs1, I.func3, I.rd, I.opcode);
            int rs1 = decimalConvert(I.rs1, 5, 1);
            int rd = decimalConvert(I.rd, 5, 1);
            if (rs1 > 31 || rd > 31) {
                illegalOperation(inst, i, sys.reg);
                return 1;
            }

            int addr = sys.reg[rs1] + decimalConvert(I.imm, 12, 0);
            if (addr < 0) {
                illegalOperation(inst, i, sys.reg);
                return 1;
            }

            if (rd == 0) continue;
            
            int len;
            int sig;
            if (strncmp(I.func3,"000", 3) == 0) {// lb
                len = 8;
                sig = 0;
            } else if (strncmp(I.func3,"001", 3) == 0) {// lh
                len = 16;
                sig = 0;
            } else if (strncmp(I.func3,"010", 3) == 0) {// lw
                len = 32;
                sig = 0;
            } else if (strncmp(I.func3,"100", 3) == 0) {//lbu
                len = 8;
                sig = 1;
            } else if (strncmp(I.func3,"101", 3) == 0) {//lhu   
                len = 16;
                sig = 1;
            } else {
                notImplemented(inst, i, sys.reg);
                return 1;
            }

            if (addr > 46847) {
                addr = addr - 46848;
                addr = addr * 8;
                char string[33] = {0};

                char temp[33] = {0};
                for (int k = 0; k < len; k++) {
                    if (sys.heap_banks[(addr + k) / (64 * 8)][(addr + k) % (64 * 8)] != '-') {
                        temp[k] = sys.heap_banks[(addr + k) / (64 * 8)][(addr + k) % (64 * 8)];
                    } else {
                        illegalOperation(inst, i, sys.reg);
                        return 1;
                    }
                }

                int offset = -1;
                for (int k = 0; k < len; k++) {
                    if (k % 8 == 0) offset += 8;
                    string[offset - (k%8)] = temp[k];
                }

                sys.reg[rd] = decimalConvert(string, len, sig);
            } else if (addr > 2047) { // Virtual routines (load)
                addr = addr - 2048;

                if (addr == 18) { // Read character
                    int readChar = getchar();
                    sys.reg[rd] = readChar;
                } else if (addr == 22) { // Read number
                    char readNum[33] = {0};
                    fgets(readNum, 32, stdin);
                    sys.reg[rd] = atoi(readNum);
                } else {
                    illegalOperation(inst, i, sys.reg);
                    return 1;
                }
            } else {
                char m_num[33] = {0};

                if (addr > 1023) {
                    addr = addr - 1024;
                    addr = addr * 8;

                    for (int k = 0; k < len; k++) {
                        m_num[k] = sys.data_mem[addr + k];
                        if (m_num[k] != '0' && m_num[k] != '1') m_num[k] = '0';
                    }
                } else {
                    char temp[33] = {0};

                    addr = addr * 8;
                    for (int k = 0; k < len; k++) {
                        temp[k] = sys.inst_mem[(addr + k) / 32][31 - ((addr + k) % 32)];
                        if (temp[k] != '0' && temp[k] != '1') temp[k] = '0';
                    }
                    int offset = -1;
                    for (int k = 0; k < len; k++) {
                        if (k % 8 == 0) offset += 8;
                        m_num[k] = temp[offset - (k%8)];
                    }
                }
                

                sys.reg[rd] = decimalConvert(m_num, len, sig);
            }
        } else if (strncmp("0100011", opcode.opcode, 7) == 0) {// Storing
            sscanf(inst, "%s%s%s%s%s%s", S.imm2, S.rs2, S.rs1, S.func3, S.imm1, S.opcode);
            int rs1 = decimalConvert(S.rs1, 5, 1);
            int rs2 = decimalConvert(S.rs2, 5, 1);
            if (rs1 > 31 || rs2 > 31) {
                illegalOperation(inst, i, sys.reg);
                return 1;
            }
                            
            char combined[13] = {0};
            strncpy(combined, S.imm2, 7);
            strncat(combined, S.imm1, 5);
        
            int addr = sys.reg[rs1] + decimalConvert(combined, 12, 0);
            if (addr < 0) {
                illegalOperation(inst, i, sys.reg);
                return 1;
            }
            
            
            int len;
            if (strncmp(S.func3,"000", 3) == 0) {// sb
                len = 8;
            } else if (strncmp(S.func3,"001", 3) == 0) {// sh
                len = 16;
            } else if (strncmp(S.func3,"010", 3) == 0) {// sw
                len = 32;
            } else {
                notImplemented(inst, i, sys.reg);
                return 1;
            }

            if (addr > 46847) {
                addr = addr - 46848;
                addr = addr * 8;

                char *string = binaryConvert(sys.reg[rs2], 32);

                char temp[33] = {0};
                int offset = -1;
                for (int k = 0; k < len; k++) {
                    if (k % 8 == 0) offset += 8;
                    temp[k] = string[offset - (k%8)];
                }


                for (int k = 0; k < len; k++) {
                    if (sys.heap_banks[(addr + k) / (64 * 8)][(addr + k) % (64 * 8)] != '-') {
                        sys.heap_banks[(addr + k) / (64 * 8)][(addr + k) % (64 * 8)] = temp[k];
                    } else {
                        free(string);
                        illegalOperation(inst, i, sys.reg);
                        return 1;
                    }
                }

                free(string);
            } else if (addr > 2047) {// Virtual Routines (Store)
                addr = addr - 2048;

                if (addr == 0) {// Print character
                    putchar(sys.reg[rs2]);
                } else if (addr == 4) {// Print number
                    printf("%d", sys.reg[rs2]);
                } else if (addr == 8) {// Print unsigned int as hexadecimal
                    hexadecimal(sys.reg[rs2], 0);
                } else if (addr == 12) {// CPU Halt
                    printf("CPU Halt Requested\n");
                    return 1;
                } else if (addr == 32) {// Print PC
                    hexadecimal(i * 4, 8);
                } else if (addr == 36) {// Print reg dump
                    reg_dump(i, sys.reg);
                } else if (addr == 40) {// Print hexadeximal number from memory (unsigned)
                    char tempNum[33] = {0};
                    int v = sys.reg[rs2];
                    if (v > 1024) {
                        v = v - 1024;
                        v = v * 8;

                        for (int k = 0; k < len; k++) {
                            tempNum[k] = sys.data_mem[v + len - k - 1];
                            if (tempNum[k] != '0' && tempNum[k] != '1') tempNum[k] = '0';
                        }
                    } else {
                        char temp[33] = {0};
                        v = v * 8;
                        for (int k = 0; k < 32; k++) {
                            temp[k] = sys.inst_mem[(v + k) / 32][31 - ((v + k) % 32)];
                            if (temp[k] != '0' && temp[k] != '1') temp[k] = '0';
                        }
                        int offset = -1;
                        for (int k = 0; k < len; k++) {
                            if (k % 8 == 0) offset += 8;
                            tempNum[k] = temp[offset - (k%8)];
                        }
                    }
                    
                    hexadecimal(decimalConvert(tempNum, 32, 1), len / 4);
                } else if (addr == 48) {// Malloc 
                    int found = 1;
                    int padding = 0;
                    if (sys.reg[rs2] % 64 != 0) padding = 1;
                    len = ((sys.reg[rs2] / 64) + padding) * 64 * 8;
                    sys.reg[28] = 0;
                    for (int k = 0; k < 128; k++) {
                        for (int j = 0; j < (64 * 8); j++) {
                            if (sys.heap_banks[k][j] == '-') {
                                if (k + ((j + len) / (64 * 8)) > 128) {
                                    found = 0;
                                    break;
                                }

                                int broken = 0;
                                for (int z = 0; z < len; z++) {
                                    if (sys.heap_banks[k + (j + z) / (64 * 8)][(j+z) % (64 * 8)] != '-') broken = 1;
                                }
                                if (broken == 0) {
                                    for (int z = 0; z < len; z++) {
                                        sys.heap_banks[k + (j + z) / (64 * 8)][(j+z) % (64 * 8)] = '0';
                                    }
                                    sys.reg[28] = 46848 + (64 * k) + (j / 8);
                                    found = 0;
                                }
                            }
                            if (found == 0) break;
                        }
                        if (found == 0) break;
                    }
                } else if (addr == 52) {// Free
                    int bank = (sys.reg[rs2] - 46848) / 64;
                    for (int k = 0; k < 64 * 8; k++) {
                        if (sys.heap_banks[bank][k] == '-') {
                            illegalOperation(inst, i, sys.reg);
                            return 1;
                        } else {
                            sys.heap_banks[bank][k] = '-';
                        }
                    }    
                } else {
                    illegalOperation(inst, i, sys.reg);
                    return 1;
                }
            } else {
                char *string = binaryConvert(sys.reg[rs2], 32);

                if (addr > 1023) {
                    addr = addr - 1024;
                    addr = addr * 8;

                    for (int k = 0; k < len; k++) {
                        sys.data_mem[addr + k] = string[k];
                    }
                } else {
                    free(string);
                    illegalOperation(inst, i, sys.reg);
                    return 1;
                }

                free(string);
            }
        } else if (strncmp("1100011", opcode.opcode, 7) == 0) {// Branching
            sscanf(inst, "%s%s%s%s%s%s", SB.imm2, SB.rs2, SB.rs1, SB.func3, SB.imm1, SB.opcode);
            int rs1 = decimalConvert(SB.rs1, 5, 1);
            int rs2 = decimalConvert(SB.rs2, 5, 1);
            if (rs1 > 31 || rs2 > 31) {
                illegalOperation(inst, i, sys.reg);
                return 1;
            }
            
            char combined[14] = {0};
            combined[0] = SB.imm2[0];
            combined[1] = SB.imm1[4];
            for (int k = 0; k < 6; k++) {
                combined[2+k] = SB.imm2[1+k];
            }
            for (int k = 0; k < 4; k++) {
                combined[8+k] = SB.imm1[k];
            }
            combined[12] = '0';

            int move = decimalConvert(combined, 13, 0) / 4;

            rs1 = sys.reg[rs1];
            rs2 = sys.reg[rs2];

            int flag = 1;
            if (strncmp(SB.func3,"000", 3) == 0) {// beq
                if (rs1 == rs2) flag = 0;
            } else if (strncmp(SB.func3,"001", 3) == 0) {// bne
                if (rs1 != rs2) flag = 0;
            } else if (strncmp(SB.func3,"100", 3) == 0) {// blt
                if (rs1 < rs2) flag = 0;
            } else if (strncmp(SB.func3,"110", 3) == 0) {// bltu
                if ((uint32_t) rs1 < (uint32_t) rs2) flag = 0;
            } else if (strncmp(SB.func3,"101", 3) == 0) {// bge
                if (rs1 >= rs2) flag = 0;
            } else if (strncmp(SB.func3,"111", 3) == 0) {//bgeu
                if ((uint32_t) rs1 >= (uint32_t) rs2) flag = 0;
            } else notImp = 1;

            if (flag == 0) {
                i = i + move;
                continue;
            }
        } else if (strncmp("1101111", opcode.opcode, 7) == 0) {// jal
            sscanf(inst, "%s%s%s", UJ.imm, UJ.rd, UJ.opcode);
            int rd = decimalConvert(UJ.rd, 5, 1);
            if (rd > 31) {
                illegalOperation(inst, i, sys.reg);
                return 1;
            }

            // Store pointer only if not 0 reg.
            if (rd != 0) sys.reg[rd] = (i + 1) * 4;

            char string[22] = {0};
            string[0] = UJ.imm[0];
            for (int k = 0; k < 10; k++) {
                string[10+k] = UJ.imm[1+k];
            }
            string[9] = UJ.imm[11];
            for (int k = 0; k < 8; k++) {
                string[1+k] = UJ.imm[12+k];
            }       
            string[20] = '0';

            i = i + (decimalConvert(string, 21, 0) / 4);

            continue;
        } else if (strncmp("1100111", opcode.opcode, 7) == 0) {// jalr
            sscanf(inst, "%s%s%s%s%s", I.imm, I.rs1, I.func3, I.rd, I.opcode);
            int rs1 = decimalConvert(I.rs1, 5, 1);
            int rd = decimalConvert(I.rd, 5, 1);
            if (rs1 > 31 || rd > 31) {
                illegalOperation(inst, i, sys.reg);
                return 1;
            }

            if (strncmp(I.func3,"000", 3) == 0) {
                if (rd != 0) sys.reg[rd] = (i + 1) * 4;
                i = (sys.reg[rs1] + decimalConvert(I.imm, 12, 0)) / 4;   
                continue;
            } else notImp = 1;
        } else notImp = 1;
        
        if (notImp == 1) {
            notImplemented(inst, i, sys.reg);
            return 1;
        }
        i++;
    }

    printf("\n");
    return 0;
}