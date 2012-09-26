#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

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

void write_ref(FILE *file, V obj, HashMap *hm)
{
	V n = get_hashmap(hm, obj);
	uint32_t ref = toInt(n);
	ref = htonl(ref);
	fwrite(((char*)&ref) + 1, 3, 1, file);
}

bool canBeSmallInt(V obj)
{
	return isInt(obj) && (toInt(obj) > -(1 << 23)) && (toInt(obj) < (1 << 23));
}

void write_object(FILE *file, V obj, HashMap *hm)
{
	int t = getType(obj);
	union double_or_uint64_t num;
	ITreeNode *id = NULL;
	String *s = NULL;
	Stack *st;
	HashMap *hmv;
	int8_t n8;
	uint8_t l8;
	int32_t n32;
	uint32_t l32;
	int64_t n64;
	uint64_t l64;
	int i;
	Bucket *b;

	char type = t;

	switch (t)
	{
		case T_NUM:
			if (canBeSmallInt(obj))
				type |= TYPE_SHORT;
			break;
		case T_IDENT:
			id = toIdent(obj);
			if (id->length < 256)
				type |= TYPE_SHORT;
			break;
		case T_STR:
			s = toString(obj);
			if (s->length < 256)
				type |= TYPE_SHORT;
			break;
		case T_FRAC:
			if (toNumerator(obj) < 128 && toNumerator(obj) >= -128 &&
				toDenominator(obj) < 256)
				type |= TYPE_SHORT;
			break;
	}

	fwrite(&type, 1, 1, file);

	switch (t)
	{
		case T_IDENT:
			if (type & TYPE_SHORT)
			{
				l8 = id->length;
				fwrite(&l8, 1, 1, file);
			}
			else
			{
				l32 = id->length;
				l32 = htonl(l32);
				fwrite(&l32, 4, 1, file);
			}
			fwrite(&id->data, id->length, 1, file);
			break;
		case T_STR:
			if (type & TYPE_SHORT)
			{
				l8 = s->length;
				fwrite(&l8, 1, 1, file);
			}
			else
			{
				l32 = s->length;
				l32 = htonl(l32);
				fwrite(&l32, 4, 1, file);
			}
			fwrite(toCharArr(s), s->length, 1, file);
			break;
		case T_NUM:
			if (type & TYPE_SHORT)
			{
				n32 = toInt(obj);
				n32 = htonl(n32);
				fwrite(((char*)&n32) + 1, 3, 1, file);
			}
			else
			{
				num.d = toNumber(obj);
				num.i = htonll(num.i);
				fwrite(&num, 8, 1, file);
			}
			break;
		case T_FRAC:
			if (type & TYPE_SHORT)
			{
				n8 = toNumerator(obj);
				fwrite(&n8, 1, 1, file);
				l8 = toDenominator(obj);
				fwrite(&l8, 1, 1, file);
			}
			else
			{
				n64 = toNumerator(obj);
				n64 = htonl(n64);
				fwrite(&n64, 8, 1, file);
				l64 = toDenominator(obj);
				l64 = htonl(l64);
				fwrite(&l64, 8, 1, file);
			}
			break;
		case T_PAIR:
			write_ref(file, toFirst(obj), hm);
			write_ref(file, toSecond(obj), hm);
			break;
		case T_STACK:
			st = toStack(obj);
			l32 = st->used;
			l32 = htonl(l32);
			fwrite(&l32, 4, 1, file);
			for (i = 0; i < st->used; i++)
			{
				write_ref(file, st->nodes[i], hm);
			}
			break;
		case T_DICT:
			hmv = toHashMap(obj);
			l32 = hmv->used;
			l32 = htonl(l32);
			fwrite(&l32, 4, 1, file);
			if (hmv->map != NULL)
			{
				for (i = 0; i < hmv->size; i++)
				{
					b = hmv->map[i];
					while(b != NULL)
					{
						write_ref(file, b->key, hm);
						write_ref(file, b->value, hm);
						b = b->next;
					}
				}
			}
			break;
	}
}

bool persist(char *fname, V obj)
{
	int i;
	HashMap *hm;
	int maxm;
	FILE *file;
	uint32_t obj_encoded;
	Bucket *b, *bb;

	hm = persist_collect(obj);
	if (hm)
	{
		maxm = persist_order_objects(hm);
		file = fopen(fname, "w");
		fwrite("\007DV\x03\0\0\0\x02", 8, 1, file);
		obj_encoded = htonl((uint32_t)toInt(get_hashmap(hm, obj)));
		fwrite(&obj_encoded, 4, 1, file);

		fwrite("\x12\0\0\0", 4, 1, file);

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

		if (hm->map != NULL)
		{
			for (i = 0; i < hm->size; i++)
			{
				b = hm->map[i];
				while(b != NULL)
				{
					bb = b;
					b = b->next;
					free(bb);
				}
			}
			free(hm->map);
		}
		free(hm);

		return true;
	}
	else
	{
		return false;
	}
}

bool persist_all(char *fname, Stack *objects)
{
	int i;
	HashMap *hm = NULL;
	int maxm;
	FILE *file;
	uint32_t obj_encoded;
	Bucket *b;
	uint32_t ssize;

	if (objects->used)
	{
		hm = persist_collect(objects->nodes[0]);

		if (!hm)
		{
			return false;
		}

		for (i = 1; i < objects->used; i++)
		{
			if (!persist_collect_(objects->nodes[i], hm))
			{
				return false;
			}
		}

		maxm = persist_order_objects(hm);
	}
	else
	{
		maxm = 0; // empty stack is valid too
	}

	file = fopen(fname, "w");

	fwrite("\007DV\x03", 4, 1, file);
	ssize = htonl(objects->used + 1);
	fwrite(&ssize, 4, 1, file);

	for (i = 0; i < objects->used; i++)
	{
		obj_encoded = htonl((uint32_t)toInt(get_hashmap(hm, objects->nodes[i])));
		fwrite(&obj_encoded, 4, 1, file);
	}

	fwrite("\x12\0\0\0", 4, 1, file);

	V reverse_lookup[maxm];

	if (hm)
	{
		for (i = 0; i < hm->size; i++)
		{
			b = hm->map[i];
			while (b != NULL)
			{
				reverse_lookup[toInt(b->value)] = b->key;
				b = b->next;
			}
		}
	}

	for (i = 0; i < maxm; i++)
	{
		write_object(file, reverse_lookup[i], hm);
	}

	fclose(file);

	return true;
}

bool valid_persist_name(V fname)
{
	String *s = toString(fname);
	return memchr(toCharArr(s), '/', s->length) == NULL;
}

void makedirs(char *path)
{
	char *first_slash = path;
	while ((first_slash = strchr(first_slash + 1, '/')))
	{
		*first_slash = '\0';
		mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		*first_slash = '/';
	}
}

char *make_persist_path(V fname)
{
	String *s = toString(fname);
	char *pathbase = getenv("XDG_DATA_HOME");
	size_t plen;
	char *home = NULL;
	if (pathbase == NULL)
	{
		home = getenv("HOME");
		plen = strlen(home) + strlen("/.local/share");
		pathbase = malloc(plen + 1);
		sprintf(pathbase, "%s/.local/share", home);
	}
	else
	{
		plen = strlen(pathbase);
	}
	char *data = malloc(plen + strlen("/deja/persist/") + s->length + 3 + 1);
	sprintf(data, "%s/deja/persist/%s.vu", pathbase, toCharArr(s));
	if (home)
	{ // if home is not NULL, that means we allocated pathbase
		free(pathbase);
	}
	makedirs(data);
	return data;
}
