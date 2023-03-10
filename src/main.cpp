/* swissmatchup - a program to create swiss-style pairings from a file called "Players.txt",
 * where each line has the form:
 * "[Player ID] [Player name] [Previously fought players] [Score] [Time range available]".
 * Comments can be denoted with a '#' at the start of the line.
 */
#include "misc.h"
#include "util.h"
#include "files.h"

#define SLOTS                 MINUTES_IN_HOUR

class Pairing {
	private:
		// player 1, player 2
		Player *p1, *p2;
		float time;
		bool isEarliest;

		Pairing *matchPlayer(Pairing *pairings, int *size, int p1Idx);
		int pairPlayer(Pairing *pairings, int *size, int p1Idx, int search, float startTime, float endTime, float minHourDif);
		void printPairings(Pairing *pairings, int size);
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

Players players;
Tokeniser tokeniser;

void handleArgs(int argc, char *argv[]);
// TODO: this isn't needed anymore as I know now that printf can pad numbers with 0's too
void printTime(FILE *stream, int hour, int minute);
// Time (as a float) TO Token
std::string ttot(float timeFloat);


int main(int argc, char *argv[])
{
}
