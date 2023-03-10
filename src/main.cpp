/* swissmatchup - a program to create swiss-style pairings from a file called "Players.txt",
 * where each line has the form:
 * "[Player ID] [Player name] [Previously fought players] [Score] [Time range available]".
 * Comments can be denoted with a '#' at the start of the line.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "misc.h"
#include "util.h"
#include "files.h"

#define SLOTS                 MINUTES_IN_HOUR

typedef struct {
	// player 1, player 2
	Player *p1, *p2;
	float time;
	unsigned int isEarliest : 1;
} Pairing;

enum daysOfWeek {
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,
	SUNDAY,
};

/* Using this many global variables is not something I take lightly.
 * However, there are a _lot_ of shared variables that many functions use,
 * and I don't want to use about 9 parameters for some functions
 */
Player *players;
int totalPlayers, longestName, longestPlayerID;
// inclusive maximum point difference between opponents that can be paired
float maxPointDif;
// the earliest time a match can take place
float earliestTime;
// the minimum gap that's between matches, in minutes
int minTimeDif;
// 0: monday, 6: sunday
int dayOfWeek;
int unpairedPlayers, longestName;
int isVisual;

void handleArgs(int argc, char *argv[]);
void pairPlayers(void);
Pairing *matchPlayer(Pairing *pairings, int *size, int p1Idx);
// returns 1 if successful; 1 otherwise
int pairPlayer(Pairing *pairings, int *size, int p1Idx, int search, float startTime, float endTime, float minHourDif);
void addPairedPlayer(Player *player1, Player *player2);
void printPlayers(void);
void printTimes(Player player);
void printPairings(Pairing *pairings, int size);
void printTime(FILE *stream, int hour, int minute);
void sortPlayers(void);
// Time (as a float) TO Token
char *ttot(float timeFloat);
void swap(Player *player1, Player *player2);
void freePlayerList(void);
int haveFought(Player p1, Player p2);


int main(int argc, char *argv[])
{
}
