#include "literals.h"
#include "gc.h"
#include "types.h"
#include "idents.h"

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

#define eofreached ((8 + curpos - oldpos) >= size)

void read_literals(char *oldpos, size_t size, Header* h)
{
	int i;
	int n = 0;
	char type;
	uint32_t str_length;
	char *startpos = oldpos + h->size * 4;
	char *curpos = startpos;
	while (!eofreached)
	{
		type = *curpos++;
		if (eofreached)
		{
			break;
		}
		n++;
		if (type == TYPE_NUM)
		{
			curpos += 8;
		}
		else if (type == TYPE_STR || type == TYPE_IDENT)
		{
			memcpy(&str_length, curpos, 4);
			curpos += 4 + ntohl(str_length);
		}
	}
	V* arr = calloc(n, sizeof(V));
	V t;
	curpos = startpos;
	for (i = 0; i < n; i++)
	{
		type = *curpos++;
		if (type == TYPE_NUM)
		{
			uint64_t d;
			memcpy(&d, curpos, 8);
			curpos += 8;
			d = ntohll(d);
			t = double_to_value(*(double*)&d);
		}
		else if (type == TYPE_STR)
		{
			memcpy(&str_length, curpos, 4);
			curpos += 4;
			str_length = ntohl(str_length);
			t = str_to_value(str_length, curpos);
			curpos += str_length;
			t->type = T_STR;
		}
		else // if (type == TYPE_IDENT)
		{
			memcpy(&str_length, curpos, 4);
			curpos += 4;
			str_length = ntohl(str_length);
			char data[str_length + 1];
			memcpy(&data, curpos, str_length);
			data[str_length] = '\0';
			t = lookup_ident(str_length, data);
			curpos += str_length;
		}
		arr[i] = t;
	}
	h->n_literals = n;
	h->literals = arr;
}

V get_literal(Header* h, uint32_t index)
{
	if (index >= h->n_literals)
	{
		return NULL;
	}
	return h->literals[index];
}
