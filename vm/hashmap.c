#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "gc.h"
#include "hashmap.h"
#include "types.h"

#include <assert.h> //FIXME

int string_hash(int length, const char *key)
{
	unsigned int hash = 2166136261;
	int i;
	for (i = 0; i < length; i++)
	{
        hash = (16777619 * hash) ^ (*key);
        key++;
	}
	return hash;
};

HashMap* new_hashmap(int initialsize)
{
	HashMap* hm = malloc(sizeof(HashMap));
	hm->used = 0;
	hm->size = initialsize;
	Bucket** bl = malloc(sizeof(Bucket*) * initialsize);
	int i;
	for (i = 0; i < hm->size; i++)
	{
		bl[i] = NULL;
	}
	hm->map = bl;
	return hm;
}

void hashmap_from_scope(V v_scope, int initialsize)
{
	Scope* scope = (Scope*)v_scope->data.object;
	scope->hm.used = 0;
	scope->hm.size = initialsize;
	Bucket** bl = calloc(initialsize, sizeof(Bucket*));
	scope->hm.map = bl;
}

V get_from_bucket(Bucket* b, String* s)
{
	if (s->length == b->keysize)
	{
		int i;
		bool equal = true;
		for (i = 0; i < s->length; i++)
		{
			if (b->key[i] != s->data[i])
			{
				equal = false;
				break;
			}
		}
		if (equal)
		{
			return b->value;
		}
	}
	return b->next == NULL ? NULL : get_from_bucket(b->next, s);
}

V get_hashmap(HashMap* hm, V key)
{
	assert(key->type == T_IDENT); //FIXME: replace with exception system
	String* s = key->data.object;
	int hash = string_hash(s->length, s->data) % hm->size;
	Bucket* b = hm->map[hash];
	return get_from_bucket(b, s);
}

Bucket* new_bucket(String* s, V value)
{
	Bucket* b = malloc(sizeof(Bucket*));
	b->keysize = s->length;
	b->key = malloc(s->length);
	memcpy(b->key, s->data, s->length);
	b->value = add_ref(value);
	b->next = NULL;
	return b;
}

bool set_to_bucket(Bucket* b, String* s, V value)
{
	if (s->length == b->keysize)
	{
		int i;
		bool equal = true;
		for (i = 0; i < s->length; i++)
		{
			if (b->key[i] != s->data[i])
			{
				equal = false;
				break;
			}
		}
		if (equal)
		{
			clear_ref(b->value);
			b->value = add_ref(value);
			return true;
		}
	}
	if (b->next == NULL)
	{
		b->next = new_bucket(s, value);
		return false;
	}
	else
	{
		return set_to_bucket(b->next, s, value);	
	}
}

void set_hashmap(HashMap* hm, V key, V value)
{
	assert(key->type == T_IDENT); //FIXME: replace with exception system
	String* s = key->data.object;
	int hash = string_hash(s->length, s->data) % hm->size;
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

void grow_hashmap(HashMap* hm)
{
	Bucket** bl = calloc(sizeof(Bucket*), hm->size * 2);
	int i;
	for (i = 0; i < hm->size; i++)
	{ //rehash!
		Bucket* b = hm->map[i];
		do
		{
			bl[string_hash(b->keysize, b->key)] = b;
		}
		while(b = b->next);
	}
	free(hm->map);
	hm->map = bl;
	hm->size *= 2;
}
