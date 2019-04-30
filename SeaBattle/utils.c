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
	int range = abs(max - min);
	return min + rand() % range;
}
