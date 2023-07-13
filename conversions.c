#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "errors.h"

long decimalConvert(char binary[], int len, int sign) {
    long decimal = 0;
    int num;

    for (int i = 0; i < len; i++) {
        if (binary[i] == '1') {
            num = 1;
            for (int k = 0; k < len - i - 1; k++) num = num * 2;
            decimal += num;
        }
    }

    if (sign == 0 && binary[0] == '1') {
        num = 1;
        for (int k = 0; k < len; k++) num = num * 2;
        decimal = decimal - num;
    }

    return decimal;
}

char* binaryConvert(uint32_t decimal, int len) {
    char *binary = malloc(len + 1);
    for (int i = len - 1; i > -1; i = i - 1) {
        if (decimal > 0) {
            if (decimal % 2 == 0) {
                binary[i] = '0';
            } else binary[i] = '1';
            decimal = decimal / 2;
        } else binary[i] = '0';
    }

    return binary;
} 

int hexadecimal(uint32_t decimal, int len_in) {
    char hexadec[9] = {0};
    int len = len_in;
    int num;
    if (len_in == 0) len = 8;

    for (int i = len - 1; i > -1; i = i - 1) {
        num = decimal % 16;
        if (decimal > 0) {
            if (num > 9) hexadec[i] = (char) (num - 10) + 97;
            else hexadec[i] = (char) num + 48;

            decimal = decimal / 16;
        } else {
            hexadec[i] = '0';
        }
    }
    
    int j = 0;
    if (len_in == 0) while (hexadec[j] == '0' && j < len) j++;
    if (j != 8) for (int i = j; i < len; i++) printf("%c", hexadec[i]);
    else printf("0");
    
    return 0;
}