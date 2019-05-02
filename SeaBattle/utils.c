#include "utils.h"
#include <stdlib.h>
#include <time.h>

void init_randomizer()
{
	static int already_rand = 0;
	if (!already_rand)
	{
		srand(time(NULL));
		already_rand = 1;
	}
}

int random_int(int min, int max)
{
	int range = abs(max - min) + 1;
	return min + rand() % range;
}

int random_bool()
{
	return random_int(0, 1);
}
