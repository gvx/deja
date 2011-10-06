#ifndef HASHMAP_DEF
#define HASHMAP_DEF

#include "value.h"

typedef struct Bucket
{
	int keysize;
	char *key;
	V value;
	struct Bucket *next;
} Bucket;

typedef struct HashMap
{
	int used;
	int size;
	Bucket** map;
} HashMap;

#include "scope.h"

HashMap* new_hashmap(int);
void hashmap_from_scope(V, int);
V get_hashmap(HashMap*, V);
void set_hashmap(HashMap*, V, V);
void grow_hashmap(HashMap*);

#endif