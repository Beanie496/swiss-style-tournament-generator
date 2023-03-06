#include <stdio.h>

#include "misc.h"
#include "files.h"
#include "util.h"


void updateFile()
{
	int mostPairedPlayers = 0;
	int numTimeRanges = 0;
	FILE *updatedPlayers = fopen("newPlayerList.txt", "w+");

	// this is to align nicely the data entries that come
	// after the previously paired players list
	for (int i = 0; i < totalPlayers; i++) {
		int playerTimeRanges = getNumTimeRanges(&players[i], dayOfWeek);
		if (players[i].prevPlayedNum > mostPairedPlayers)
			mostPairedPlayers = players[i].prevPlayedNum;
		if (playerTimeRanges > numTimeRanges)
			numTimeRanges = playerTimeRanges;
	}

	for (int i = 0; i < totalPlayers; i++)
		writeLine(updatedPlayers, &players[i], mostPairedPlayers, numTimeRanges);

	fclose(updatedPlayers);
}


void writeLine(FILE *updatedPlayers, Player *player, int mostPairedPlayers, int mostTimeRanges)
{
	int spaces;

	// ID and name
	fprintf(updatedPlayers, "%-3d %*s", player->id, -longestName, player->name);
	spaces = writePrevPairedIDs(updatedPlayers, player, mostPairedPlayers);
	// player score
	fprintf(updatedPlayers, "%*.1f", spaces, player->score);
	writeAllTimes(updatedPlayers, player, mostTimeRanges);
	fprintf(updatedPlayers, "   %s\n", player->comment);
}


int writePrevPairedIDs(FILE *updatedPlayers, Player *player, int mostPairedPlayers)
{
	int spaces;
	// for alignment
	int currentPairedPlayers = 0;

	fprintf(updatedPlayers, " {");
	for (int i = 0; i < player->prevPlayedNum; i++) {
		currentPairedPlayers++;
		fprintf(updatedPlayers, "%*d", -longestPlayerID, player->prevPlayed[i]);
		if (i != players[i].prevPlayedNum - 1)
			fprintf(updatedPlayers, ", ");
	}
	spaces = (mostPairedPlayers - currentPairedPlayers) * longestPlayerID;
	// this accounts for the commas
	spaces += (mostPairedPlayers - currentPairedPlayers) * 2;
	// 0 and 1 paired players both have 0 commas
	if (currentPairedPlayers == 0)
		spaces--;
	// 2 for at least 2 spaces; 3 to align the player's score
	spaces += 2 + 3;
	fprintf(updatedPlayers, "}");

	return spaces;
}


void writeAllTimes(FILE *updatedPlayers, Player *player, int mostTimeRanges)
{
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
}
