#ifndef FILE_DEF
#define FILE_DEF

#include "header.h"
#include "literals.h"
#include "value.h"
#include "types.h"
#include "gc.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
	V name;
	V global;
	Header header;
	uint32_t *code;
} File;

V load_file(V, V);

#endif