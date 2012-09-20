#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "hashmap.h"
#include "types.h"
#include "value.h"
#include "stack.h"
#include "literals.h"

bool persist_collect_(V original, HashMap *hm)
{
	int type = getType(original);
	HashMap *hmv;
	Bucket *b;
	int i;
	if (type == T_SCOPE || type == T_FUNC || type == T_CFUNC)
	{
		return false;
	}
	if (get_hashmap(hm, original) != NULL)
	{
		return true;
	}
	set_hashmap(hm, original, intToV(-pair_ordinal(original)-1));
	switch(type)
	{
		case T_STR:
		case T_IDENT:
		case T_NUM:
		case T_FRAC:
			break;
		case T_STACK:
			for (i = 0; i < toStack(original)->used; i++)
			{
				if (!persist_collect_(toStack(original)->nodes[i], hm))
				{
					return false;
				}
			}
			break;
		case T_DICT:
			hmv = toHashMap(original);
			if (hmv->map != NULL)
			{
				for (i = 0; i < hmv->size; i++)
				{
					b = hmv->map[i];
					while(b != NULL)
					{
						if (!persist_collect_(b->key, hm) ||!persist_collect_(b->value, hm))
						{
							return false;
						}
						b = b->next;
					}
				}
			}
			break;
		case T_PAIR:
			if (!persist_collect_(toFirst(original), hm) || !persist_collect_(toSecond(original), hm))
				return false;
			break;
	}
	return true;
}

HashMap *persist_collect(V original)
{
	HashMap *hm = new_hashmap(64);

	if (persist_collect_(original, hm))
		return hm;
	return NULL;
}

int persist_order_objects(HashMap *hm)
{
	int level;
	int i;
	Bucket *b;
	long int index = 0;
	bool found = true;
	for (level = -1; found; level--)
	{
		found = false;
		for (i = 0; i < hm->size; i++)
		{
			b = hm->map[i];
			while (b != NULL)
			{
				if (toInt(b->value) == level)
				{
					b->value = intToV(index++);
					found = true;
				}
				b = b->next;
			}
		}
	}
	return index;
}

void write_object(FILE *file, V obj, HashMap *hm)
{
	int t = getType(obj);
	char type;
	switch (t)
	{
		case T_NUM:
			type = TYPE_NUM;
			fwrite(&type, 1, 1, file);
			union double_or_uint64_t num;
			num.d = toNumber(obj);
			num.i = htonll(num.i);
			fwrite(&num, 8, 1, file);
			break;
	}
}

bool persist(char *fname, V obj)
{
	int i;
	HashMap *hm;
	int maxm;
	FILE *file;
	long int obj_encoded;
	Bucket *b;

	hm = persist_collect(obj);
	if (hm)
	{
		maxm = persist_order_objects(hm);
		file = fopen(fname, "w");
		fwrite("\x07DV\x03\0\0\0\x01", 8, 1, file);
		obj_encoded = htonll(toInt(get_hashmap(hm, obj)));
		fwrite(&obj_encoded, sizeof obj_encoded, 1, file);

		V reverse_lookup[maxm];

		for (i = 0; i < hm->size; i++)
		{
			b = hm->map[i];
			while (b != NULL)
			{
				reverse_lookup[toInt(b->value)] = b->key;
				b = b->next;
			}
		}

		for (i = 0; i < maxm; i++)
		{
			write_object(file, reverse_lookup[i], hm);
		}

		fclose(file);
		return true;
	}
	else
	{
		return false;
	}
}
