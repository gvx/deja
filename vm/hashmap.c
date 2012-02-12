#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "gc.h"
#include "hashmap.h"
#include "types.h"

HashMap* new_hashmap(int initialsize)
{
	HashMap* hm = malloc(sizeof(HashMap));
	hm->used = 0;
	hm->size = initialsize;
	hm->map = NULL;
	return hm;
}

void hashmap_from_value(V v, int initialsize)
{
	HashMap* hm = toHashMap(v);
	hm->used = 0;
	hm->size = initialsize;
	hm->map = NULL;
}

void hashmap_from_scope(V v_scope, int initialsize)
{
	Scope* scope = toScope(v_scope);
	scope->hm.used = 0;
	scope->hm.size = initialsize;
	scope->hm.map = NULL;
}

V get_hashmap(HashMap* hm, V key)
{
	if (hm->map == NULL)
	{
		return NULL;
	}
	String* s = toString(key);
	Bucket* b = hm->map[s->hash % hm->size];
	while (b != NULL)
	{
		if (s->length == b->keysize)
		{
			if (!memcmp(b->key, toCharArr(s), s->length))
			{
				return b->value;
			}
		}
		b = b->next;
	}
	return NULL;
}

Bucket* new_bucket(String* s, V value)
{
	Bucket* b = malloc(sizeof(Bucket));
	b->keysize = s->length;
	b->key = malloc(s->length + 1);
	memcpy(b->key, toCharArr(s), s->length + 1);
	b->value = add_ref(value);
	b->next = NULL;
	return b;
}

bool set_to_bucket(Bucket* b, String* s, V value)
{
	if (s->length == b->keysize)
	{
		if (!memcmp(b->key, toCharArr(s), s->length))
		{
			clear_ref(b->value);
			b->value = add_ref(value);
			return false;
		}
	}
	if (b->next == NULL)
	{
		b->next = new_bucket(s, value);
		return true;
	}
	else
	{
		return set_to_bucket(b->next, s, value);	
	}
}

void set_hashmap(HashMap* hm, V key, V value)
{
	if (hm->map == NULL)
	{
		Bucket **bl = calloc(hm->size, sizeof(Bucket*));
		hm->map = bl;
	}
	String* s = toString(key);
	uint32_t hash = s->hash % hm->size; 
	Bucket* b = hm->map[hash];
	if (b == NULL)
	{
		hm->map[hash] = new_bucket(s, value);
		hm->used++;
	}
	else
	{
		if (set_to_bucket(b, s, value))
		{
			hm->used++;
		}
	}
	if (hm->used > hm->size)
	{
		grow_hashmap(hm);
	}
}

bool change_bucket(Bucket* b, String* s, V value)
{
	if (s->length == b->keysize)
	{
		if (!memcmp(b->key, toCharArr(s), s->length))
		{
			clear_ref(b->value);
			b->value = add_ref(value);
			return true;
		}
	}
	if (b->next == NULL)
	{
		return false;
	}
	else
	{
		return change_bucket(b->next, s, value);	
	}
}

bool change_hashmap(HashMap* hm, V key, V value)
{
	if (hm->map == NULL)
	{
		return false;
	}
	String* s = toString(key);
	Bucket* b = hm->map[s->hash % hm->size];
	if (b == NULL)
	{
		return false;
	}
	else
	{
		return change_bucket(b, s, value);
	}
}

void grow_hashmap(HashMap* hm)
{
	Bucket** bl = calloc(hm->size * 2, sizeof(Bucket*));
	int i;
	int h;
	Bucket* bb;
	for (i = 0; i < hm->size; i++)
	{ //rehash!
		Bucket *b = hm->map[i];
		while (b)
		{
			h = string_hash(b->keysize, b->key) % (hm->size * 2);
			bb = b->next;
			b->next = bl[h];
			bl[h] = b;
			b = bb;
		}
	}
	free(hm->map);
	hm->map = bl;
	hm->size *= 2;
}
