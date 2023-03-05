#include <ctype.h>
#include <stdlib.h>

#include "util.h"

char token[MAXTOKEN];
int tokenLength, tokenType, numToken;


int numInArr(int *array, int length, int num)
{
	while (--length >= 0)
		if (array[length] == num)
			return 1;
	return 0;
}


int numLength(int num)
{
	// even 0 is still 1 character long
	int length = 1;
	while ((num /= 10) != 0)
		length++;
	return length;
}


int BSF(uint64_t x)
{
	if (x == 0)
		return -1;
	// isolates smallest bit
	x = x & -x;

	int count = 0;
	if ((num & 0xffff0000) != 0)
		count += 16;
	if ((num & 0xff00ff00) != 0)
		count += 8;
	if ((num & 0xf0f0f0f0) != 0)
		count += 4;
	if ((num & 0xcccccccc) != 0)
		count += 2;
	if ((num & 0xaaaaaaaa) != 0)
		count += 1;
	return count;
}


int getNumTimeRanges(Player *player)
{
	// TODO: implement
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


// returns 0 if unsuccessful, the range otherwise
int getNextRange(uint64_t *p1Times, uint64_t *p2Times, float *startTime, float *endTime)
{
	// TODO:
	// bits: 000010111111111001110111111111
	// bits: 001111111110001101111111111000
	// ^ get the length and start pos of each of
	// the intersecting bits, one by one
	uint64_t islands;
	uint64_t boundaries;
	uint64_t start;
	uint64_t end;

	for (int hour = (int)*startTime; hour < HOURS_IN_DAY; hour++) {
		if (!p1Times[hour] || !p2Times[hour])
			continue;
		islands = p1Times[hour] & p2Times[hour];
		// TODO: implement a time range continuing from the previous hour
		boundaries = islands ^ (islands << 1);
		boundaries -= (start = BSF(boundaries));
		boundaries -= (end = BSF(boundaries));
		// if they're both non-0 (so both boundaries fit within 1 hour), that is one full range
		if (start && end) {
			*startTime = (float)hour + (float)getMagnitude(start) / 60.0;
			*endTime += (float)hour + (float)getMagnitude(end) / 60.0;
			return getMagnitude(end) - getMagnitude(start);
		}
		if (!start && end)
			printError(UNREACHABLE_CODE);
		if (start) {
			*startTime += (float)getMagnitude(start) / 60.0;
		}
	}
	return 0;
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

		fseek(file, -1, SEEK_CUR);
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

		fseek(file, -1, SEEK_CUR);
		return tokenType = NUMBER;
	}

	return tokenType = UNDEFINED;
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

		case UNREACHABLE_CODE:
			fprintf(stderr, "ERROR %d: Unreachable code", errorCode);
			exit(errorCode);

		default:
			fprintf(stderr, "Unknown error code \"%d\"\n", errorCode);
			exit(errorCode);
	}
}

