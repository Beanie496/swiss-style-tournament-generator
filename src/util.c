#include "util.h"


int numLength(int num)
{
	// even 0 is still 1 character long
	int length = 1;
	while ((num /= 10) != 0)
		length++;
	return length;
}
