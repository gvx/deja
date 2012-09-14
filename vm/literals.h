#ifndef LIT_DEF
#define LIT_DEF

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "header.h"
#include "error.h"

bool read_literals(char*, size_t, Header*);
V get_literal(Header*, uint32_t);

#endif
