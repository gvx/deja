#ifndef LIT_DEF
#define LIT_DEF

#include <stdint.h>
#include <stdio.h>

#include "header.h"
#include "value.h"

void read_literals(FILE*, Header*);
V get_literal(Header*, uint32_t);

#endif