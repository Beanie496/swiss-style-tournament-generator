#include <stdint.h>

#include "misc.h"

#ifndef FILE_H
#define FILE_H

void readInPlayers(void);
void getID(int playerIdx);
void getName(int playerIdx);
void getPrevPairedPlayers(int playerIdx);
void getScore(int playerIdx);
void getTimes(int playerIdx);
void getDayTimes(uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY], int day);
void getDayTime(uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY], int day);
void updateFile();
void writeLine(FILE *updatedPlayers, Player *player, int mostPairedPlayers, int mostTimeRanges);
int writePrevPairedIDs(FILE *updatedPlayers, Player *player, int mostPairedPlayers);
void writeAllTimes(FILE *updatedPlayers, Player *player, int mostTimeRanges);
void printError(int errorCode);

extern Player *players;
extern int totalPlayers, longestName, longestPlayerID;

#endif
