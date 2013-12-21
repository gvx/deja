#ifndef MERSENNE_DEF
#define MERSENNE_DEF

#include "lib.h"

void init_random(int);
Error random_int(Stack*, Stack*);
Error random_range(Stack*, Stack*);
Error random_choose(Stack*, Stack*);
Error random_chance(Stack*, Stack*);

#endif
