#include "literals.h"
#include "gc.h"
#include "types.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <netinet/in.h>

#define TYPE_IDENT '\x00'
#define TYPE_STR '\x01'
#define TYPE_NUM '\x02'

union bo_double
{
	double d;
	unsigned char c[8];
};

bool little_endian(void)
{
	union bo_double test;
	test.d = 1.0;
	return test.c[0] != '\0';
}

void read_literals(FILE* f, Header* h)
{
	long oldpos = ftell(f);
	int i;
	int n = 0;
	char type;
	uint32_t str_length; 
	while (!feof(f))
	{
		n++;
		fread(&type, sizeof(type), 1, f);
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
	fseek(f, oldpos, SEEK_SET);
	for (i = 0; i < n; i++)
	{
		fread(&type, sizeof(type), 1, f);
		if (type == TYPE_NUM)
		{
			t = new_value(T_NUM);
			union bo_double d;
			fread(&d, sizeof(d), 1, f);
			if (little_endian())
			{
				union bo_double d2;
				int j;
				for (j = 0; j < 8; j++)
				{
					d2.c[8-j] = d.c[j];
				}
				d.d = d2.d;
			}
			t->data.number = d.d;
		}
		else if (type == TYPE_STR || type == TYPE_IDENT)
		{
			fread(&str_length, sizeof(str_length), 1, f);
			s = malloc(sizeof(String));
			s->length = ntohl(str_length);
			s->data = malloc(s->length);
			fread(s->data, s->length, 1, f);
			t = new_value(type == TYPE_STR ? T_STR : T_IDENT);
			t->data.object = s;
		}
		arr[i] = t;
	}
	h->literals = arr;
}

V get_literal(Header* h, uint32_t index)
{
	return h->literals[index];
}