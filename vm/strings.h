#ifndef STR_DEF
#define STR_DEF

#include "utf8.h"
#include "value.h"
#include "lib.h"
#define toNewString(x) ((NewString*)(x+1))

typedef struct {
	size_t size;     //size in bytes
	size_t length;   //length in characters
	uint32_t hash;   //hashcode (initially 0)
	utf8byte text[1];
} NewString;

uint32_t need_hash(V);
uint32_t string_length(V);
V charat(utf8, utf8index);
V strslice(utf8, utf8index, utf8index);
V a_to_string(char*);
V str_to_string(size_t, char*);

Error new_ord(Stack*, Stack*);
Error new_chr(Stack*, Stack*);
Error new_chars(Stack*, Stack*);

#endif
