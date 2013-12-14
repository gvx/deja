#include <stdint.h>
#include <stdlib.h>

#include "mersenne.h"

// Mersenne twister code based on pseudocode from Wikipedia
// https://en.wikipedia.org/wiki/Mersenne_twister#Pseudocode

#define LCGReps 3

typedef uint32_t State;

State MTState[624];
State MTIndex = 0;

inline State LCG(State in)
{
	// LCG values from Numerical Recipes
	return 1664525L * in + 1013904223L;
}

void init_random(int s)
{
	int i;
	State seed = s;
	MTIndex = 0;
	for (i = 0; i < LCGReps; i++)
	{
		seed = LCG(seed);
	}
	MTState[0] = seed;
	for (i = 1; i < 624; i++)
	{
		// loop over each other element
		MTState[i] = 0x6c078965L * (MTState[i-1] ^ (MTState[i-1] >> 30)) + i;
	}
}

void generate_numbers()
{
 	int i;
	for (i = 0; i < 624; i++)
	{
		State y = (MTState[i] & 0x80000000L) // bit 31 (32nd bit) of MTState[i]
						+ (MTState[(i+1) % 624] & 0x7fffffffL); // bits 0-30 (first 31 bits) of MTState[...]
		MTState[i] = MTState[(i + 397) % 624] ^ (y >> 1);
		if (y & 1)
		{
			MTState[i] = MTState[i] ^ (0x9908b0dfL);
		}
	}
}

State extract_number()
{
	if (MTIndex == 0)
	{
		generate_numbers();
	}

	int y = MTState[MTIndex];
	MTIndex = (MTIndex + 1) % 624;
	return y;
}

Error random_int(Stack *S, Stack *scope_arr)
{
	pushS(int_to_value(extract_number()));

	return Nothing;
}

State extract_ranged(State range)
{
	// If you can show this introduces
	// a bias, please report it to me
	// and I'll write a proper version.
	//return extract_number() % range;
	State rnd, limit = UINT32_MAX - UINT32_MAX % range;
	do
	{
		rnd = extract_number();
	}
	while (rnd >= limit);
	return rnd % range;
}

Error random_range(Stack *S, Stack *scope_arr)
{
	require(2);
	V a = popS();
	V b = popS();
	if (getType(a) != T_NUM || getType(b) != T_NUM)
	{
		clear_ref(a);
		clear_ref(b);
		return TypeError;
	}
	State min = toNumber(a);
	State max = toNumber(b);
	if (min >= max)
	{
		clear_ref(a);
		clear_ref(b);
		return ValueError;
	}
	pushS(int_to_value(min + extract_ranged(max - min)));

	clear_ref(a);
	clear_ref(b);
	return Nothing;
}
