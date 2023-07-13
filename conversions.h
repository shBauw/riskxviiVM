#ifndef CONVERSIONS_INCLUDED
#define CONVERSIONS_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "errors.h"

long decimalConvert(char binary[], int len, int sign);
char* binaryConvert(uint32_t decimal, int len);
int hexadecimal(uint32_t decimal, int len_in);
#endif