#ifndef HEADER_DEF
#define HEADER_DEF

#define MAGIC "\aDV"
#define VERSION '\x03'

#include <netinet/in.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "value.h"

typedef struct
{
	char magic[3];
	char version;
	uint32_t size;
	V* literals;
	uint32_t n_literals;
} Header;

Header read_header(char*, size_t);
bool header_correct(Header*);

#endif
