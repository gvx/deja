#ifndef LIT_DEF
#define LIT_DEF

#include <stdint.h>
#include <stdio.h>

#include "header.h"

void read_literals(char*, size_t, Header*);
V get_literal(Header*, uint32_t);

#endif
