#ifndef ERRORS_INCLUDED
#define ERRORS_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "conversions.h"

void reg_dump(int program_counter, int reg[]);
void illegalOperation(char operation[], int pc, int reg[]);
void notImplemented(char operation[], int pc, int reg[]);
#endif