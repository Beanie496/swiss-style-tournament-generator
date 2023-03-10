#include <string>
#include <vector>

#ifndef MISC_H
#define MISC_H

#define MINUTES_IN_HOUR       60
#define HOURS_IN_DAY          24
#define DAYS_IN_WEEK          7
#define MAXLINE               1000

class Player {
	private:
		int id;
		std::string name;
		int prevPlayedNum;
		std::vector<int> prevPlayed;
		float score;
		// this represents the times they're available for each minute of the day.
		// 1 bit is 1 minute
		std::vector<bool> times[DAYS_IN_WEEK][HOURS_IN_DAY];
		bool paired;
		std::string comment;
};

class Players {
	public:
		void pairPlayers(void);
		void printPlayers(void);
		void printTimes(Player player);

	private:
		std::vector<Player> players;
		int totalPlayers, longestName, longestPlayerID;
		int unpairedPlayers;
		// inclusive maximum point difference between opponents that can be paired
		float maxPointDif;
		// the earliest time a match can take place
		float earliestTime;
		// the minimum gap that's between matches, in minutes
		int minTimeDif;
		// 0: monday, 6: sunday
		int dayOfWeek;
		bool isVisual;
		
		void addPairedPlayer(Player *player1, Player *player2);
		bool haveFought(Player p1, Player p2);
		int getNumTimeRanges(Player *player, int day);
		void setMinuteBits(uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY], int day, int startHour, int endHour, int startMinute, int endMinute);
		int getNextRange(uint64_t *p1times, uint64_t *p2Times, float *startTime, float *endTime);
		void getID(int playerIdx);
		void getName(int playerIdx);
		void getPrevPairedPlayers(int playerIdx);
		void getScore(int playerIdx);
		void getTimes(int playerIdx);
		void getDayTimes(uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY], int day);
		void getDayTime(uint64_t times[DAYS_IN_WEEK][HOURS_IN_DAY], int day);
		void sortPlayers(void);
		void swap(Player *player1, Player *player2);
		void freePlayerList(void);
};

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
