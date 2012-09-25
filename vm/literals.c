#include "literals.h"
#include "idents.h"
#include "utf8.h"

#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>

#define eofreached ((unsigned)(8 + curpos - oldpos) >= size)

uint64_t ntohll_(uint64_t i)
{
	return ntohl(i >> 32) | ((uint64_t)ntohl(i & (((uint64_t)1 << 32) - 1)) << 32);
}

bool read_literals(char *oldpos, size_t size, Header* h)
{
	int i, j;
	int n = 0;
	char type;
	uint32_t str_length;
	uint32_t ref;
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
		switch (type)
		{
			case TYPE_NUM:
				curpos += 8;
				break;
			case TYPE_STR:
			case TYPE_IDENT:
				memcpy(&str_length, curpos, 4);
				curpos += 4 + ntohl(str_length);
				break;
			case TYPE_STR | TYPE_SHORT:
			case TYPE_IDENT | TYPE_SHORT:
				str_length = *curpos++;
				curpos += str_length;
				break;
			case TYPE_PAIR:
				curpos += 6;
				break;
			case TYPE_FRAC:
				curpos += 16;
				break;
			case TYPE_FRAC | TYPE_SHORT:
				curpos += 2;
				break;
			case TYPE_LIST:
				memcpy(&str_length, curpos, 4);
				curpos += 4 + 3 * ntohl(str_length);
				break;
			case TYPE_DICT:
				memcpy(&str_length, curpos, 4);
				curpos += 4 + 6 * ntohl(str_length);
				break;
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
			union double_or_uint64_t d;
			memcpy(&d, curpos, 8);
			curpos += 8;
			d.i = ntohll(d.i);
			t = double_to_value(d.d);
		}
		else if (type == TYPE_STR)
		{
			memcpy(&str_length, curpos, 4);
			curpos += 4;
			str_length = ntohl(str_length);
			if (!valid_utf8(str_length, curpos))
			{
				error_msg = "Wrong encoding for string literal, should be UTF-8.";
				return false;
			}
			t = str_to_value(str_length, curpos);
			curpos += str_length;
		}
		else if (type == TYPE_IDENT)
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
		else if (type == (TYPE_STR | TYPE_SHORT))
		{
			str_length = *curpos++;
			if (!valid_utf8(str_length, curpos))
			{
				error_msg = "Wrong encoding for string literal, should be UTF-8.";
				return false;
			}
			t = str_to_value(str_length, curpos);
			curpos += str_length;
		}
		else if (type == (TYPE_IDENT | TYPE_SHORT))
		{
			str_length = *curpos++;
			char data[str_length + 1];
			memcpy(&data, curpos, str_length);
			data[str_length] = '\0';
			t = lookup_ident(str_length, data);
			curpos += str_length;
		}
		else if (type == TYPE_PAIR)
		{
			ref = 0;
			memcpy(((char*)&ref) + 1, curpos, 3);
			str_length = ntohl(ref);
			if (ref >= i)
			{
				error_msg = "Illegal pair detected.";
				return false;
			}
			V v1 = arr[ref];

			memcpy(((char*)&ref) + 1, curpos + 3, 3);
			ref = ntohl(ref);
			if (ref >= i)
			{
				error_msg = "Illegal pair detected.";
				return false;
			}
			V v2 = arr[ref];

			t = new_pair(v1, v2);
			curpos += 6;
		}
		else if (type == TYPE_FRAC)
		{
			int64_t numer;
			int64_t denom;
			memcpy(&numer, curpos, 8);
			numer = ntohll(numer);
			memcpy(&denom, curpos + 8, 8);
			denom = ntohll(denom);
			t = new_frac(numer, denom);
			curpos += 16;
		}
		else if (type == (TYPE_FRAC | TYPE_SHORT))
		{
			int8_t numer;
			uint8_t denom;
			numer = *curpos++;
			denom = *curpos++;
			t = new_frac(numer, denom);
		}
		else if (type == TYPE_LIST)
		{
			memcpy(&str_length, curpos, 4);
			str_length = ntohl(str_length);
			t = new_list();
			curpos += 4;
			if (str_length > 0)
			{
				uint32_t size = 64;
				while (size < str_length) size <<= 1;
				toStack(t)->size = size;
				toStack(t)->used = str_length;
				toStack(t)->nodes = calloc(size, sizeof(V));
				for (j = 0; j < str_length; j++)
				{
					ref = 0;
					memcpy(((char*)&ref) + 1, curpos, 3);
					ref = ntohl(ref);
					toStack(t)->nodes[j] = intToV((uint64_t)ref);
					curpos += 3;
				}
			}
		}
		else if (type == TYPE_DICT)
		{
			memcpy(&str_length, curpos, 4);
			curpos += 4;
			str_length = ntohl(str_length);
			t = new_dict();
			if (str_length > 0)
			{
				uint32_t size = 16;
				while (size < str_length) size <<= 1;
				toHashMap(t)->size = size;
				toHashMap(t)->used = str_length;
				toHashMap(t)->map = (Bucket**)curpos;
			}
			curpos += 6 * str_length;
		}
		else
		{
			error_msg = "Unknown literal type.";
			return false;
		}
		arr[i] = t;
	}

	for (i = 0; i < n; i++)
	{
		t = arr[i];
		switch(getType(t))
		{
			case TYPE_LIST:
				for (j = 0; j < toStack(t)->used; j++)
				{
					toStack(t)->nodes[j] = arr[toInt(toStack(t)->nodes[j])];
				}
				break;
			case TYPE_DICT:
				if (toHashMap(t)->map)
				{
					curpos = ((char*)toHashMap(t)->map);

					toHashMap(t)->map = NULL;
					str_length = toHashMap(t)->used; //worst abuse of variable name ever Y/Y?
					toHashMap(t)->used = 0;
					for (j = 0; j < str_length; j++)
					{
						ref = 0;
						memcpy(((char*)&ref) + 1, curpos, 3);
						ref = ntohl(ref);
						V key = arr[ref];

						ref = 0;
						memcpy(((char*)&ref) + 1, curpos + 3, 3);
						ref = ntohl(ref);
						V value = arr[ref];

						set_hashmap(toHashMap(t), key, value);

						curpos += 6;
					}
				}
				break;
		}
	}

	h->n_literals = n;
	h->literals = arr;
	return true;
}

V get_literal(Header* h, uint32_t index)
{
	if (index >= h->n_literals)
	{
		return NULL;
	}
	return h->literals[index];
}
