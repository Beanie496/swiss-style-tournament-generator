#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "files.h"
#include "util.h"

FILE *fp;


void readInPlayers()
{
	fp = fopen("Players.txt", "r");
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

	do
		getDayTime(times, day);
	while (getToken(fp) == COMMA);
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
