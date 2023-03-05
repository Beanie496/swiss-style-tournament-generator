#include <stdio.h>

#include "misc.h"

#ifndef UTIL_H
#define UTIL_H

#define TOCHAR(d)             ((d) + '0')
#define TODIGIT(c)            ((c) - '0')
#define MAX(a, b)             ((a) > (b) ? (a) : (b))
#define MIN(a, b)             ((a) < (b) ? (a) : (b))
#define BSF(n)                ((n) & -(n))
#define MAXTOKEN              100

int numInArr(int *array, int length, int num);
int numLength(int num);
int getMagnitude(uint64_t num);
int getNumTimeRanges(Player *player);
void setMinuteBits(uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY], int day, int startHour, int endHour, int startMinute, int endMinute);
int getNextRange(uint64_t *p1times, uint64_t *p2Times, float *startTime, float *endTime);
int getToken(FILE* file);
void printError(int errorCode);

extern char token[];
extern int tokenLength, tokenType, numToken;

#endif
