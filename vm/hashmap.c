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

uint32_t get_hash(V v)
{
	int t = getType(v);
	if (t == T_STR)
	{
		return toString(v)->hash;
	}
	else if (t == T_NUM)
	{
		return (uint32_t)toNumber(v);
	}
	else if (t == T_PAIR)
	{
		return get_hash(toFirst(v)) + get_hash(toSecond(v));
	}
	else
	{
		return (unsigned long)v;
	}
}

V get_hashmap(HashMap* hm, V key)
{
	if (hm->map == NULL)
	{
		return NULL;
	}
	Bucket* b = hm->map[get_hash(key) % hm->size];
	while (b != NULL)
	{
		if (equal(key, b->key))
		{
			return b->value;
		}
		b = b->next;
	}
	return NULL;
}

Bucket* new_bucket(V key, V value)
{
	Bucket* b = malloc(sizeof(Bucket));
	b->key = add_ref(key);
	b->value = add_ref(value);
	b->next = NULL;
	return b;
}

bool set_to_bucket(Bucket* b, V key, V value)
{
	if (equal(b->key, key))
	{
		V tmp = b->value;
		b->value = add_ref(value);
		clear_ref(tmp);
		return false;
	}
	if (b->next == NULL)
	{
		b->next = new_bucket(key, value);
		return true;
	}
	else
	{
		return set_to_bucket(b->next, key, value);
	}
}

void set_hashmap(HashMap* hm, V key, V value)
{
	if (hm->map == NULL)
	{
		Bucket **bl = calloc(hm->size, sizeof(Bucket*));
		hm->map = bl;
	}
	uint32_t hash = get_hash(key) % hm->size;
	Bucket* b = hm->map[hash];
	if (b == NULL)
	{
		hm->map[hash] = new_bucket(key, value);
		hm->used++;
	}
	else
	{
		if (set_to_bucket(b, key, value))
		{
			hm->used++;
		}
	}
	if (hm->used > hm->size)
	{
		resize_hashmap(hm, hm->size * 2);
	}
}

bool change_bucket(Bucket* b, V key, V value)
{
	if (equal(b->key, key))
	{
		V tmp = b->value;
		b->value = add_ref(value);
		clear_ref(tmp);
		return true;
	}
	if (b->next == NULL)
	{
		return false;
	}
	else
	{
		return change_bucket(b->next, key, value);
	}
}

bool change_hashmap(HashMap* hm, V key, V value)
{
	if (hm->map == NULL)
	{
		return false;
	}

	Bucket* b = hm->map[get_hash(key) % hm->size];
	if (b == NULL)
	{
		return false;
	}
	else
	{
		return change_bucket(b, key, value);
	}
}

void resize_hashmap(HashMap* hm, int newsize)
{
	Bucket** bl = calloc(newsize, sizeof(Bucket*));
	int i;
	int h;
	Bucket* bb;
	for (i = 0; i < hm->size; i++)
	{ //rehash!
		Bucket *b = hm->map[i];
		while (b)
		{
			h = get_hash(b->key) % newsize;
			bb = b->next;
			b->next = bl[h];
			bl[h] = b;
			b = bb;
		}
	}
	free(hm->map);
	hm->map = bl;
	hm->size = newsize;
}

bool delete_hashmap(HashMap *hm, V key)
{
	if (hm->map == NULL)
	{
		return false;
	}
	Bucket **bprev = &hm->map[get_hash(key) % hm->size];
	Bucket *b = *bprev;
	while (b != NULL)
	{
		if (equal(key, b->key))
		{
			*bprev = b->next;
			clear_ref(b->key);
			clear_ref(b->value);
			free(b);
			hm->used--;
			if ((hm->used < hm->size / 2) && (hm->size > 16))
			{
				resize_hashmap(hm, hm->size / 2);
			}
			return true;
		}
		bprev = &(*bprev)->next;
		b = b->next;
	}
	return false;
}
