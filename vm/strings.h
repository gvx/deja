#ifndef STR_DEF
#define STR_DEF

#include "utf8.h"
#include "value.h"
#include "lib.h"
#include "stack.h"
#include "error.h"

#define toNewString(x) ((NewString*)(x+1))

typedef struct {
	size_t size;     //size in bytes
	size_t length;   //length in characters
	uint32_t hash;   //hashcode (initially 0)
	utf8byte text[1];
} __attribute__((packed)) NewString;

uint32_t need_hash(V);
uint32_t string_length(V);
V charat(utf8, utf8index);
V strslice(utf8, utf8index, utf8index);
V a_to_string(char*);
V str_to_string(size_t, char*);
V empty_string_to_value(size_t, utf8*);

Error ord(Stack*, Stack*);
Error chr(Stack*, Stack*);
Error chars(Stack*, Stack*);
Error starts_with(Stack*, Stack*);
Error ends_with(Stack*, Stack*);
Error contains(Stack*, Stack*);
Error count(Stack*, Stack*);
Error find(Stack*, Stack*);
Error concat(Stack*, Stack*); /* concat( "a" "b" "c" ) */
Error concat_list(Stack*, Stack*); /* concat [ "a" "b" "c" ] */
Error join(Stack*, Stack*);
Error split(Stack*, Stack*);
Error slice(Stack*, Stack*);
Error split_any(Stack*, Stack*);

#endif
