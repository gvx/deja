#ifndef HASHMAP_DEF
#define HASHMAP_DEF

#include <stdint.h>

#include "value.h"

typedef struct Bucket
{
	V key;
	V value;
	struct Bucket *next;
} Bucket;

typedef struct HashMap
{
	int used;
	int size;
	Bucket** map;
} HashMap;

HashMap* new_hashmap(int);
void hashmap_from_value(V, int);
void hashmap_from_scope(V, int);
V get_hashmap(HashMap*, V);
V get_dict(HashMap*, V);
bool delete_hashmap(HashMap*, V);
void set_hashmap(HashMap*, V, V);
bool change_hashmap(HashMap*, V, V);
void resize_hashmap(HashMap*, int);
void copy_hashmap(HashMap*, HashMap*);

#endif
