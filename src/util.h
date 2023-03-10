#include <stdio.h>

#include "misc.h"

#ifndef UTIL_H
#define UTIL_H

#define TOCHAR(d)             ((d) + '0')
#define TODIGIT(c)            ((c) - '0')
#define MAX(a, b)             ((a) > (b) ? (a) : (b))
#define MIN(a, b)             ((a) < (b) ? (a) : (b))
#define MAXTOKEN              100

int numInArr(int *array, int length, int num);
int numLength(int num);
int BSF(uint64_t num);
int PopCnt(uint64_t num);
int getToken(FILE* file);
void printError(int errorCode);

#endif
