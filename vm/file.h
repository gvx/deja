#ifndef FILE_DEF
#define FILE_DEF

#include "literals.h"
#include "types.h"
#include "gc.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
	V name;
	V source;
	V global;
	Header header;
	uint32_t *code;
} File;

V load_file(V, V);
V load_stdin(V);
V load_memfile(char*, size_t, V, V);

#endif
