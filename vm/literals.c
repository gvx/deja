#include "literals.h"
#include "gc.h"
#include "types.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>

#define TYPE_IDENT '\x00'
#define TYPE_STR '\x01'
#define TYPE_NUM '\x02'

#if __BYTE_ORDER == __BIG_ENDIAN
#define ntohll(x) (x)
#else
uint64_t ntohll(uint64_t i)
{
	return ntohl(i >> 32) | ((uint64_t)ntohl(i & (((uint64_t)1 << 32) - 1)) << 32);
}
#endif

double unpack754(uint64_t i)
{
    double result;
    long long shift;

    if (i == 0)
    {
    	return 0.0;
    }

    result = (i & ((1LL << 52) - 1));
    result /= (1LL << 52);
    result += 1.0;

    shift = ((i >> 52) & ((1LL << 11) - 1)) - (1 << 10) + 1;
    while(shift-- > 0)
    {
	    result *= 2.0;
	}
    while(++shift < 0)
    {
	    result /= 2.0;
	}

	if (i & (1LL >> 63))
	{
		result = -result;
	}

    return result;
}

void read_literals(FILE* f, Header* h)
{
	long oldpos = ftell(f);
	long startpos;
	int i;
	int n = 0;
	char type;
	uint32_t str_length; 
	fseek(f, h->size * 4, SEEK_CUR);
	startpos = ftell(f);
	while (!feof(f))
	{
		fread(&type, sizeof(type), 1, f);
		if (feof(f))
		{
			break;
		}
		n++;
		if (type == TYPE_NUM)
		{
			fseek(f, 8, SEEK_CUR);
		}
		else if (type == TYPE_STR || type == TYPE_IDENT)
		{
			fread(&str_length, sizeof(str_length), 1, f);
			fseek(f, ntohl(str_length), SEEK_CUR);
		}
	}
	V* arr = calloc(n, sizeof(V));
	V t;
	String* s;
	fseek(f, startpos, SEEK_SET);
	for (i = 0; i < n; i++)
	{
		fread(&type, sizeof(type), 1, f);
		if (type == TYPE_NUM)
		{
			uint64_t d;
			fread(&d, sizeof(d), 1, f);
			t = double_to_value(unpack754(ntohll(d));
		}
		else //if (type == TYPE_STR || type == TYPE_IDENT)
		{
			fread(&str_length, sizeof(str_length), 1, f);
			s = malloc(sizeof(String));
			s->length = ntohl(str_length);
			s->data = malloc(s->length);
			fread(s->data, s->length, 1, f);
			t = new_value(type == TYPE_STR ? T_STR : T_IDENT);
			t->data.object = s;
			t->color = Green;
		}
		arr[i] = t;
	}
	h->n_literals = n;
	h->literals = arr;
	fseek(f, oldpos, SEEK_SET);
}

V get_literal(Header* h, uint32_t index)
{
	return h->literals[index];
}