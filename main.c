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

#include "util.h"

#define MINUTES_IN_HOUR       60
#define HOURS_IN_DAY          24
#define DAYS_IN_WEEK          7
#define SLOTS                 MINUTES_IN_HOUR
#define MAXTOKEN              100
#define MAXLINE               1000

typedef struct {
    int id;
	char* name;
	int prevPlayedNum;
	int *prevPlayed;
	float score;
	// this represents the times they're available for each minute of the day.
	// 1 bit is 1 minute
	uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY];
	unsigned int paired : 1;
	char *comment;
} Player;

typedef struct {
	// player 1, player 2
	Player *p1, *p2;
	float time;
	unsigned int isEarliest : 1;
} Pairing;

enum tokens {
	C_START_BRACKET,
	C_END_BRACKET,
	COLON,
	COMMA,
	DASH,
	DOT,
	HASHTAG,
	NUMBER,
	STRING,
	UNDEFINED,
};

enum errors {
	// starting at 1 because an axit code of 0 is no error
	EXPECTED_COLON = 01,
	EXPECTED_COMMA,
	EXPECTED_CURLY_BRACKET,
	EXPECTED_DASH,
	EXPECTED_DECIMAL,
	EXPECTED_DOT,
	EXPECTED_HALF,
	EXPECTED_NUMBER,
	EXPECTED_SINGLE_DIGIT,
	EXPECTED_STRING,
	TOO_MANY_DAYS,
};

enum daysOfWeek {
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,
	SUNDAY,
};

// Using this many global variables is not something I take lightly.
// However, there are a _lot_ of shared variables that many functions use,
// and I don't want to use about 9 parameters for some functions
char token[MAXTOKEN];
int tokenLength, tokenType, numToken;
// inclusive maximum point difference between opponents that can be paired
float maxPointDif;
// the earliest time a match can take place
float earliestTime;
// the minimum gap that's between matches, in minutes
int minTimeDif;
// 0: monday, 6: sunday
int dayOfWeek;
int unpairedPlayers, totalPlayers, longestName, longestPlayerID;
int isVisual;
Player *players;
FILE *fp;

void handleArgs(int argc, char *argv[]);
void readInPlayers(void);
void pairPlayers(void);
Pairing *matchPlayer(Pairing *pairings, int *size, int p1Idx);
// returns 0 if succesful; 1 otherwise
int getNextRange(uint64_t *p1times, uint64_t *p2Times, float *startTime, float *endTime);
int canPairPlayer(Pairing *pairings, int *size, int p1Idx, int search, float startTime, float endTime, float minHourDif);
void getID(int playerIdx);
void getName(int playerIdx);
void getPrevPairedPlayers(int playerIdx);
void getScore(int playerIdx);
void getTimes(int playerIdx);
void getDayTimes(uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY], int day);
void getDayTime(uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY], int day);
void setMinuteBits(uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY], int day, int startHour, int endHour, int startMinute, int endMinute);
void addPairedPlayer(Player *player1, Player *player2);
void printPlayers(void);
void printVisualTimes(Player *player);
void updateFile();
void printLine(FILE *updatedPlayers, Player *player, int mostPairedPlayers);
void printPairings(Pairing *pairings, int size);
void printError(int errorCode);
void sortPlayers(void);
// Time (as a float) TO Token
char *ttot(float timeFloat);
int getToken(FILE* file);
void swap(Player *player1, Player *player2);
void freePlayerList(void);
int haveFought(Player p1, Player p2);


int main(int argc, char *argv[])
{
	totalPlayers = 0;
	longestName = 0;
	fp = fopen("Players.txt", "r");

	// defaults
	dayOfWeek = SATURDAY;
	maxPointDif = 1.0;
	earliestTime = 12.0;
	minTimeDif = 30;
	isVisual = 0;

	handleArgs(argc, argv);

	readInPlayers();
	sortPlayers();
	printPlayers();
	// a gap looks nice
	printf("\n");
	pairPlayers();
	updateFile();
	freePlayerList();

	return 0;
}


void handleArgs(int argc, char *argv[])
{
	if (argc == 2 && !strcmp(argv[1], "help")) {
		printf("Use the argument \"-h\" for a list of the arguments\n");
		exit(0);
	}

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			fprintf(stderr, "Unknown argument \"%s\"\n", argv[i]);
			exit(0);
		}
		if (argv[i][2] != '\0') {
			fprintf(stderr, "Unknown argument \"%s\"\n", argv[i]);
			exit(0);
		}
		switch (argv[i][1]) {
			// day of week
			case 'd':
				if (i >= argc - 1)
					break;

				i++;
				if (argv[i][1] != '\0' || TODIGIT(argv[i][0]) > 6) {
					fprintf(stderr, "Day of week given outside range\n");
					exit(0);
				}
				dayOfWeek = TODIGIT(argv[i][0]);
				break;

			// max point difference
			case 'p':
				if (i >= argc - 1)
					break;
				
				sscanf(argv[++i], "%f", &maxPointDif);
				break;
				
			// earliest time
			case 'e':
				if (i >= argc - 1)
					break;
				
				sscanf(argv[++i], "%f", &earliestTime);
				break;
				
			// min time difference
			case 't':
				if (i >= argc - 1)
					break;
				
				sscanf(argv[++i], "%d", &minTimeDif);
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
				fprintf(stderr, "unknown argument \"%s\"", argv[i]);
				exit(0);
		}
	}
}

void readInPlayers()
{
	int playerIdx = 0;
	int isEOF = 0;
	int i;
	char temp;
	char tempStr[MAXLINE];
	players = malloc(0);


	while (!isEOF) {
		// this skips over comments
		while (getToken(fp) == HASHTAG)
			while (fgetc(fp) != '\n')
				;
		if (tokenType == EOF)
			break;


		players = realloc(players, sizeof(Player) * (playerIdx + 1));

		getID(playerIdx);
		getName(playerIdx);
		getPrevPairedPlayers(playerIdx);
		getScore(playerIdx);
		getTimes(playerIdx);

		// saves the rest of the line as a comment
		for (i = 0; (temp = fgetc(fp)) != '\n'; i++)
			tempStr[i] = temp;
		tempStr[i++] = '\0';
		players[playerIdx].comment = malloc(i * sizeof (char));
		strcpy(players[playerIdx].comment, tempStr);

		playerIdx++;

		if (temp == EOF)
			isEOF = 1;
		else
			fseek(fp, -1, SEEK_CUR);
	}
	fclose(fp);

	// the highest ID is one fewer than the number of players
	longestPlayerID = numLength(playerIdx - 1);
	totalPlayers = playerIdx;
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
	if (unpairedPlayers == 0)
		printf("None");
	else
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
	float startTime, endTime;
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

		while (!getNextRange(players[p1Idx].times[dayOfWeek],
					players[search].times[dayOfWeek],
					&startTime, &endTime))
			if (canPairPlayer(pairings, size, p1Idx, search, startTime, endTime, minHourDif))
				break;
	}

	return pairings;
}


// returns 0 if succesful; 1 otherwise
int getNextRange(uint64_t *p1times, uint64_t *p2Times, float *startTime, float *endTime)
{
	// TODO:
	// bits: 000010111111111001110111111111
	// bits: 001111111110001101111111111000
	// ^ get the length and start pos of each of
	// the intersecting bits, one by one

	for (int hour = 0; hour < HOURS_IN_DAY; hour++) {
		
	}
	return 0;
}


int canPairPlayer(Pairing *pairings, int *size, int p1Idx, int search, float startTime, float endTime, float minHourDif)
{
	// if the previous match's time is equal to the proposed start time of this one,
	// buffer it by the minimum amount of time a match needs
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


void getID(int playerIdx)
{
	if (tokenType != NUMBER) {
		fprintf(stderr, "Warning: Player ID not given. Defaulting to ID of %d.\n", playerIdx);
		players[playerIdx].id = playerIdx;
	} else {
		players[playerIdx].id = numToken;
	}
}


void getName(int playerIdx)
{
	if (getToken(fp) != STRING)
		printError(EXPECTED_STRING);
	players[playerIdx].name = malloc(sizeof(char) * tokenLength);
	if (tokenLength > longestName)
		longestName = tokenLength;
	strcpy(players[playerIdx].name, token);
}


void getPrevPairedPlayers(int playerIdx)
{
	int size = 0;
	if (getToken(fp) != C_START_BRACKET) {
		printError(EXPECTED_CURLY_BRACKET);
	}
	if (getToken(fp) != C_END_BRACKET) {
		while (tokenType != EOF) {
			if (tokenType == NUMBER) {
				if (size == 0)
					players[playerIdx].prevPlayed = malloc(++size);				
				else
					players[playerIdx].prevPlayed = realloc(players[playerIdx].prevPlayed, ++size);
				players[playerIdx].prevPlayed[size - 1] = numToken;
			} else {
				printError(EXPECTED_NUMBER);
			}
			if (getToken(fp) == C_END_BRACKET)
				break;
			if (tokenType != COMMA)
				printError(EXPECTED_COMMA);
			getToken(fp);
		}
	} else {
		players[playerIdx].prevPlayed = NULL;
	}
	players[playerIdx].prevPlayedNum = size;
}


void getScore(int playerIdx)
{
	if (getToken(fp) != NUMBER)
		printError(EXPECTED_NUMBER);
	players[playerIdx].score = (float)numToken;
	if (getToken(fp) != DOT)
		printError(EXPECTED_DOT);
	if (getToken(fp) != NUMBER)
		printError(EXPECTED_DECIMAL);
	if (tokenLength != 1)
		printError(EXPECTED_SINGLE_DIGIT);
	if (numToken != 5 && numToken != 0)
		printError(EXPECTED_HALF);
	players[playerIdx].score += (float)numToken / 10.0;
}


void getTimes(int playerIdx)
{
	int day = 0;

	for (int i = 0; i < DAYS_IN_WEEK; i++)
		for (int j = 0; j < HOURS_IN_DAY; j++)
			players[playerIdx].times[i][j] = 0;

	/* Loop through each day of the week:
	 *   Loop through each range of each day:
	 *     Set the relevant minutes
	 */

	if (getToken(fp) != C_START_BRACKET)
		printError(EXPECTED_CURLY_BRACKET);

	while (day < DAYS_IN_WEEK)
		getDayTimes(players[playerIdx].times, day++);

	if (getToken(fp) != C_END_BRACKET)
		printError(EXPECTED_CURLY_BRACKET);
}


void getDayTimes(uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY], int day)
{
	if (getToken(fp) != C_START_BRACKET)
		printError(EXPECTED_CURLY_BRACKET);

	// if the list of times is empty, there's nothing to be done
	if (getToken(fp) == C_END_BRACKET)
		return;

	do {
		getDayTime(times, day);
	} while (getToken(fp) == COMMA);
}


void getDayTime(uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY], int day)
{
	int startHour, endHour;
	int startMinute, endMinute;

	if (tokenType != NUMBER)
		printError(EXPECTED_NUMBER);
	startHour = numToken;

	if (getToken(fp) != COLON)
		printError(EXPECTED_NUMBER);

	if (getToken(fp) != NUMBER)
		printError(EXPECTED_NUMBER);
	startMinute = numToken;

	if (getToken(fp) != DASH)
		printError(EXPECTED_DASH);

	if (getToken(fp) != NUMBER)
		printError(EXPECTED_NUMBER);
	endHour = numToken;

	if (getToken(fp) != COLON)
		printError(EXPECTED_NUMBER);

	if (getToken(fp) != NUMBER)
		printError(EXPECTED_NUMBER);
	endMinute = numToken;

	setMinuteBits(times, day, startHour, endHour, startMinute, endMinute);
}


void setMinuteBits(uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY], int day, int startHour, int endHour, int startMinute, int endMinute)
{
	if (startHour < endHour) {
		// sets a row of bits from startMinute to MINUTES_IN_HOUR:
		// - gets a row of 1's
		// - left-shifts it by the difference in times
		// - NOTs it to get a row of 1's from 0 to the difference
		// - left-shifts it by startMinute to get a row of 1's from
		//   startMinute to MINUTES_IN_HOUR
		times[day][startHour] = ~(~(uint64_t)0 << (MINUTES_IN_HOUR - startMinute)) << startMinute;

		while (++startHour < endHour)
			// row of 1's from 0 to MINUTES_IN_HOUR
			times[day][startHour] = ~(~(uint64_t)0 << MINUTES_IN_HOUR);

		startMinute = 0;
	}

	times[day][endHour] = ~(~0 << (endMinute - startMinute)) << startMinute;
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
			printVisualTimes(&players[i]);
		printf("\n");
	}
}


void printVisualTimes(Player player[])
{
	// TODO: fix this function
	/*
	printf("   %4.1f - %4.1f   [", player->times[0], player->times[1]);
	for (float hour = 0.0; hour < (float)HOURS_IN_DAY; hour++) {
		for (float minute = 0.0; minute < 1.0; minute += 0.25) {
			if (hour + minute >= player->times[0] && hour + minute < player->times[1])
				printf("#");
			else
				printf("-");
		}

		// after hour 11 (at 12:00)
		if (hour == 11.0)
			printf("|");
		else if (hour != 23.0)
			printf(" ");
	}
	printf("]");
	*/
}


void updateFile()
{
	int mostPairedPlayers = 0;
	FILE *updatedPlayers = fopen("newPlayerList.txt", "w+");

	// this is to align nicely the data entries that come
	// after the previously paired players list
	for (int i = 0; i < totalPlayers; i++)
		if (players[i].prevPlayedNum > mostPairedPlayers)
			mostPairedPlayers = players[i].prevPlayedNum;

	for (int i = 0; i < totalPlayers; i++)
		printLine(updatedPlayers, &players[i], mostPairedPlayers);

	fclose(updatedPlayers);
}


// TODO: refactor
void printLine(FILE *updatedPlayers, Player *player, int mostPairedPlayers)
{
	int spaces;
	// this is also for alignment
	int currentPairedPlayers = 0;

	// ID and name
	fprintf(updatedPlayers, "%-3d %*s", player->id, -longestName, player->name);

	fprintf(updatedPlayers, " {");
	for (int i = 0; i < player->prevPlayedNum; i++) {
		currentPairedPlayers++;
		fprintf(updatedPlayers, "%*d", -longestPlayerID, player->prevPlayed[i]);
		if (i != players[i].prevPlayedNum - 1) {
			fprintf(updatedPlayers, ", ");
		}
	}

	spaces = (mostPairedPlayers - currentPairedPlayers) * longestPlayerID;
	// this accounts for the commas
	spaces += (mostPairedPlayers - currentPairedPlayers) * 2;
	// 0 and 1 paired players both have 0 commas
	if (currentPairedPlayers == 0)
		spaces--;
	
	// 2 for at least 2 spaces; 2 to align the player's score
	spaces += 2 + 2;

	fprintf(updatedPlayers, "} %*.1f", spaces, player->score);
	// TODO: make this print the times properly
	fprintf(updatedPlayers, "    {");
	for (int i = 0; i < DAYS_IN_WEEK; i++) {
		fprintf(updatedPlayers, " {");
		for (int j = 0; j < HOURS_IN_DAY; j++) {
			for (int k = 0; k < MINUTES_IN_HOUR; k++) {
			}

		}
		fprintf(updatedPlayers, "}");
		if (i != DAYS_IN_WEEK - 1)
			fprintf(updatedPlayers, ",");
	}
	//fprintf(updatedPlayers, "{{%s", ttot(players[i].times[0]));
	//fprintf(updatedPlayers, " - %s}}", ttot(players[i].times[1]));
	fprintf(updatedPlayers, "   %s\n", player->comment);
}


void printPairings(Pairing *pairings, int size)
{
	int hours, minutes;

	for (int match = 0; match < size; match++) {
		hours = pairings[match].time;
		minutes = (pairings[match].time - (int)(pairings[match].time)) * 60.0;

		// it must have 2 digits, even if the first is 0
		if (hours < 10)
			printf("0");
		printf("%d:", hours);

		if (minutes < 10)
			printf("0");
		printf("%d: ", minutes);

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


void printError(int errorCode)
{
	switch (errorCode) {
		case EXPECTED_COLON:
			fprintf(stderr, "ERROR %d: Expected colon\n", errorCode);
			exit(errorCode);

		case EXPECTED_COMMA:
			fprintf(stderr, "ERROR %d: Expected comma\n", errorCode);
			exit(errorCode);

		case EXPECTED_CURLY_BRACKET:
			fprintf(stderr, "ERROR %d: Expected curly bracket\n", errorCode);
			exit(errorCode);

		case EXPECTED_DASH:
			fprintf(stderr, "ERROR %d: Expected dash\n", errorCode);
			exit(errorCode);
			
		case EXPECTED_DECIMAL:
			fprintf(stderr, "ERROR %d: Expected decimal\n", errorCode);
			exit(errorCode);
			
		case EXPECTED_DOT:
			fprintf(stderr, "ERROR %d: Expected dot\n", errorCode);
			exit(errorCode);
			
		case EXPECTED_HALF:
			fprintf(stderr, "ERROR %d: Expected .0 or .5\n", errorCode);
			exit(errorCode);
			
		case EXPECTED_NUMBER:
			fprintf(stderr, "ERROR %d: Expected number\n", errorCode);
			exit(errorCode);
			
		case EXPECTED_SINGLE_DIGIT:
			fprintf(stderr, "ERROR %d: Expected single digit\n", errorCode);
			exit(errorCode);
			
		case EXPECTED_STRING:
			fprintf(stderr, "ERROR %d: Expected string\n", errorCode);
			exit(errorCode);

		case TOO_MANY_DAYS:
			fprintf(stderr, "ERROR %d: Too many days of the week given\n", errorCode);
			exit(errorCode);

		default:
			fprintf(stderr, "Unknown error code \"%d\"\n", errorCode);
			exit(errorCode);
	}
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


int getToken(FILE *file)
{
	char c;
	while (isspace(c = fgetc(file)))
		;
	
	if (c == EOF)
		return tokenType = EOF;
	if (c == '{')
		return tokenType = C_START_BRACKET;
	if (c == '}')
		return tokenType = C_END_BRACKET;
	if (c == ':')
		return tokenType = COLON;
	if (c == ',')
		return tokenType = COMMA;
	if (c == '-')
		return tokenType = DASH;
	if (c == '.')
		return tokenType = DOT;
	if (c == '#')
		return tokenType = HASHTAG;
	// 'STRING' is a sequence of alphanumeric characters that doesn't start with a number
	if (isalpha(c)) {
		int i = 0;
		do {
			token[i++] = c;
			c = fgetc(file);
		} while (i < 100 && isalnum(c));
		
		if (i == 100) {
			fprintf(stderr, "ERROR: Maximum name length exceeded\n");
			token[i = 99] = '\0';
			while (isalpha(c = fgetc(file)))
				;
		} else {
			token[i] = '\0';
		}
		tokenLength = i + 1;

		fseek(fp, -1, SEEK_CUR);
		return tokenType = STRING;
	}
	if (isdigit(c)) {
		tokenLength = 0;
		numToken = 0;
		// convert the number in the file to an actual number
		do {
			numToken *= 10;
			numToken += TODIGIT(c);
			tokenLength++;
		} while (isdigit(c = fgetc(file)));

		fseek(fp, -1, SEEK_CUR);
		return tokenType = NUMBER;
	}

	return tokenType = UNDEFINED;
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
