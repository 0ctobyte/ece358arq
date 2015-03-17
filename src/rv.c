#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "rv.h"

uint32_t rv_seed() {
	uint32_t seed = (uint32_t)time(0);
	srand(seed);
	return seed;
}

double rv_uniform() {
	return (double)rand()/((uint64_t)RAND_MAX+1);
}

