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
int unpairedPlayers;
int isVisual;

void initGlobalVars();
void handleArgs(int argc, char *argv[]);
void handleArg(char *arg, char *nextArg);
void handleOption(char *arg, char *nextArg);
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
	initGlobalVars();
	handleArgs(argc, argv);
	readInPlayers();
	sortPlayers();
	printPlayers();
	pairPlayers();
	updateFile();
	freePlayerList();

	return 0;
}


void initGlobalVars()
{
	totalPlayers = 0;
	longestName = 0;

	// defaults
	dayOfWeek = SATURDAY;
	maxPointDif = 1.0;
	earliestTime = 12.0;
	minTimeDif = 30;
	isVisual = 0;
}


void handleArgs(int argc, char *argv[])
{
	if (argc == 2 && !strcmp(argv[1], "help")) {
		printf("Use the argument \"-h\" for a list of the arguments\n");
		exit(0);
	}

	for (int i = 1; i < argc; i++)
		handleArg(argv[i], i < argc - 1 ? argv[i + 1] : NULL);
}


void handleArg(char *arg, char *nextArg)
{
	if (arg[0] != '-') {
		fprintf(stderr, "Unknown argument \"%s\"\n", arg);
		exit(0);
	}
	if (arg[2] != '\0') {
		fprintf(stderr, "Unknown argument \"%s\"\n", arg);
		exit(0);
	}
	handleOption(arg, nextArg);
}


void handleOption(char *arg, char *nextArg)
{
	switch (arg[1]) {
		// day of week
		case 'd':
			if (nextArg == NULL)
				break;

			if (nextArg[1] != '\0' || TODIGIT(nextArg[0]) > 6) {
				fprintf(stderr,
					"Day of week given outside range\n");
				exit(0);
			}
			dayOfWeek = TODIGIT(nextArg[0]);
			break;

		// max point difference
		case 'p':
			if (nextArg == NULL)
				break;

			sscanf(nextArg, "%f", &maxPointDif);
			break;

		// earliest time
		case 'e':
			if (nextArg == NULL)
				break;

			sscanf(nextArg, "%f", &earliestTime);
			break;

		// min time difference
		case 't':
			if (nextArg == NULL)
				break;

			sscanf(nextArg, "%d", &minTimeDif);
			break;

		// print visual times
		case 'v':
			isVisual = 1;
			break;
			
		// print help
		case 'h':
			printf("Usage: swissmatchup [options]\n"
			       "Options:\n"
			       "  -d <day of week>      Set day of week: Mon-Sun = 0-6. Default %d.\n"
			       "  -p <point difference> Set maximum point difference. Default %.1f.\n"
			       "  -e <time>             Set earliest time, as a float. 12.5 is 12:30, for example. Default %.1f.\n"
			       "  -t <time difference>  Set the minimum gap between matchups. Default %d.\n"
			       "  -v                    Print the times visually.\n",
					dayOfWeek, maxPointDif, earliestTime, minTimeDif);
			exit(0);

		default:
			fprintf(stderr, "unknown argument \"%s\"", arg);
			exit(0);
	}
}


void pairPlayers()
{
	int size = 0;
	Pairing *pairings = malloc(size);


	for (int player = 0; player < totalPlayers - 1; player++) {
		pairings = matchPlayer(pairings, &size, player);
		if (players[player].paired == 0)
			unpairedPlayers++;
	}
	// the last player isn't checked in the for loop, so this covers that edge case
	if (players[totalPlayers - 1].paired == 0)
		unpairedPlayers++;

	printPairings(pairings, size);
	free(pairings);
	
	printf("Unpaired players: ");
	if (unpairedPlayers == 0) {
		printf("None\n");
		return;
	}
	for (int i = 0; i < totalPlayers; i++)
		if (players[i].paired == 0) {
			printf("%s (id: %d)", players[i].name, players[i].id);
			if (--unpairedPlayers > 0)
				printf(", ");
		}
	printf("\n");
	
	return;
}


Pairing *matchPlayer(Pairing *pairings, int *size, int p1Idx)
{
	float startTime = 0.0, endTime = 0.0;
	const float minHourDif = (float)minTimeDif / (float)MINUTES_IN_HOUR;

	for (int search = p1Idx + 1; search < totalPlayers; search++) {
		
		/* if:
		 * - both players aren't paired
		 * - the score gap is small enough
		 * - the players haven't fought before
		 * pair the players
		 */
		if ((players[p1Idx].paired | players[search].paired)
				// they're ordered by score so p1 will have a higher or equal to score than p2
				|| players[p1Idx].score - maxPointDif > players[search].score
				|| haveFought(players[p1Idx], players[search]))
			continue;

		while (getNextRange(players[p1Idx].times[dayOfWeek],
					players[search].times[dayOfWeek],
					&startTime, &endTime))
			if (pairPlayer(pairings, size, p1Idx, search, startTime, endTime, minHourDif))
				break;
	}

	return pairings;
}


int pairPlayer(Pairing *pairings, int *size, int p1Idx, int search, float startTime, float endTime, float minHourDif)
{
	// if the previous match's time is equal to the proposed start time of this one,
	// buffer it by the minimum amount of time a match needs
	// TODO: this is outdated. Rework.
	if (pairings[*size - 1].time == startTime)
		startTime += minHourDif;
	
	if (startTime > endTime - minHourDif)
		return 0;

	// if the longest time that a match can last is big enough
	if (startTime <= endTime - minHourDif) {
		addPairedPlayer(&players[p1Idx], &players[search]);
		players[p1Idx].paired = players[search].paired = 1;

		pairings = realloc(pairings, ++*size * sizeof(Pairing));
		pairings[*size - 1].p1 = &players[p1Idx];
		pairings[*size - 1].p2 = &players[search];
		pairings[*size - 1].time = startTime;

		return 1;
	}

	return 0;
}


void addPairedPlayer(Player *player1, Player *player2)
{
	if (player1->prevPlayedNum == 0)
		player1->prevPlayed = malloc(++(player1->prevPlayedNum));
	else
		player1->prevPlayed = realloc(player1->prevPlayed, ++(player1->prevPlayedNum));
	player1->prevPlayed[player1->prevPlayedNum - 1] = player2->id;

	if (player2->prevPlayedNum == 0)
		player2->prevPlayed = malloc(++(player2->prevPlayedNum));
	else
		player2->prevPlayed = realloc(player2->prevPlayed, ++(player2->prevPlayedNum));
	player2->prevPlayed[player2->prevPlayedNum - 1] = player1->id;

	return;
}


void printPlayers()
{
	for (int i = 0; i < totalPlayers; i++) {
		printf("%*s   %.1f", -longestName, players[i].name, players[i].score);
		if (isVisual)
			printTimes(players[i]);
		printf("\n");
	}
	printf("\n");
}


void printTimes(Player player)
{
	// TODO: print the times graphically
	
	int count = 0;
	for (int hour = 0; hour < 24; hour++) {
		int minute;
		uint64_t boundaries = player.times[dayOfWeek][hour] & (player.times[dayOfWeek][hour] << 1);
		while ((minute = BSF(boundaries)) != -1) {
			count++;
			boundaries ^= 1ull << minute;
			if (count == 0)
				printf("   ");
			else if (count % 2 == 0)
				printf(", ");
			else
				printf(" - ");
			printTime(stdout, hour, minute);
		}
	}
}


void printPairings(Pairing *pairings, int size)
{
	int hours, minutes;

	for (int match = 0; match < size; match++) {
		hours = pairings[match].time;
		minutes = (pairings[match].time - (int)(pairings[match].time)) * 60.0;

		// it must have 2 digits, even if the first is 0
		printTime(stdout, hours, minutes);
		printf(": ");

		printf("%*s - %*s", -longestName, pairings[match].p1->name,
				-longestName, pairings[match].p2->name);
		// if the earliest time the match _can_ take place is earlier than
		// the time it's actually taking place, it means that there's a match
		// taking its slot - meaning if other match finishes early, this one
		// can be moved
		if (pairings[match].isEarliest)
			printf("[Can change]");
		printf("\n");
	}
	for (int match = 0; match < size; match++)
		printf("id: %-2d - id: %-2d\n", pairings[match].p1->id, pairings[match].p2->id);
}


void printTime(FILE *stream, int hour, int minute)
{
	if (hour < 10)
		fprintf(stream, "0");
	fprintf(stream, "%d:", hour);

	if (minute < 10)
		fprintf(stream, "0");
	fprintf(stream, "%d: ", minute);
}


void sortPlayers()
{
	// bubble sort for simplicity's sake
	for (int i = 1; i < totalPlayers; i++)
		for (int j = 0; j < totalPlayers - 1; j++)
			// sort by score
			if (players[j].score < players[j + 1].score) {
				swap(&players[j], &players[j + 1]);
			// if the scores are the same, sort by earliest start time first
			} else if (players[j].score == players[j + 1].score) {
				if (players[j].times[0] > players[j + 1].times[0])
					swap(&players[j], &players[j + 1]);
				// if earliest time is the same, sort by earliest finish time first
				if (players[j].times[0] == players[j + 1].times[0]
						&& players[j].times[1] > players[j + 1].times[1])
					swap(&players[j], &players[j + 1]);
			}
}


char *ttot(float timeFloat)
{
	int minutes = (timeFloat - (int)timeFloat) * 60;

	if (timeFloat < 10.0)
		token[0] = '0';
	else
		token[0] = TOCHAR((int)(timeFloat / 10.0));
	token[1] = TOCHAR((int)timeFloat % 10);
	token[2] = ':';
	if (minutes < 10)
		token[3] = '0';
	else
		token[3] = TOCHAR(minutes / 10);
	token[4] = TOCHAR(minutes % 10);
	token[5] = '\0';
	
	return token;
}


void swap(Player *player1, Player *player2)
{
	Player temp = *player1;
	*player1 = *player2;
	*player2 = temp;
}


void freePlayerList()
{
	for (int i = 0; i < totalPlayers; i++) {
		free(players[i].name);
		free(players[i].prevPlayed);
		free(players[i].comment);
	}
	free(players);
}


int haveFought(Player p1, Player p2)
{
	if (p1.prevPlayedNum == 0)
		return 0;

	// we only check one of the players because if p1 hasn't played p2,
	// p2 hasn't played p1 either
	for (int i = 0; i < p1.prevPlayedNum; i++)
		if (p1.prevPlayed[i] == p2.id)
			return 1;
	
	return 0;
}
