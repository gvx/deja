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

Error random_choose(Stack *S, Stack *scope_arr)
{
	require(1);
	V collection = popS();
	if (getType(collection) != T_LIST)
	{
		clear_ref(collection);
		return TypeError;
	}
	State size = stack_size(toStack(collection));
	pushS(add_ref(toStack(collection)->nodes[extract_ranged(size)]));

	clear_ref(collection);
	return Nothing;
}

Error random_chance(Stack *S, Stack *scope_arr)
{
	require(1);
	V p = popS();
	uint64_t threshold;
	if (getType(p) == T_NUM)
	{
		threshold = toNumber(p) * UINT32_MAX;
	}
	else if (getType(p) == T_FRAC)
	{
		threshold = toNumerator(p) * UINT32_MAX / toDenominator(p);
	}
	else
	{
		clear_ref(p);
		return TypeError;
	}
	if (threshold < 0 || threshold > UINT32_MAX)
	{
		clear_ref(p);
		set_error_msg("probability should be between 0 and 1");
		return ValueError;
	}

	pushS(add_ref(extract_number() < threshold ? v_true : v_false));
	clear_ref(p);
	return Nothing;
}
