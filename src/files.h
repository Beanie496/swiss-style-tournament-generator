#include <stdint.h>

#include "misc.h"

#ifndef FILE_H
#define FILE_H

// TODO: files work very differently in C++. Rework.
void readInPlayers(void);
void updateFile();
void writeLine(FILE *updatedPlayers, Player *player, int mostPairedPlayers, int mostTimeRanges);
int writePrevPairedIDs(FILE *updatedPlayers, Player *player, int mostPairedPlayers);
void writeAllTimes(FILE *updatedPlayers, Player *player, int mostTimeRanges);
void printError(int errorCode);

class Tokeniser {
    std::string token;
    int tokenLength, tokenType, numToken;
};

extern Players players;
extern Tokeniser tokeniser;
extern FILE *fp;

#endif
