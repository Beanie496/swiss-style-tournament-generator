#include <stdint.h>

#ifndef MISC_H
#define MISC_H

#define MINUTES_IN_HOUR       60
#define HOURS_IN_DAY          24
#define DAYS_IN_WEEK          7
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
	UNREACHABLE_CODE,
};

#endif
