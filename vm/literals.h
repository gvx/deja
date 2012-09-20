#ifndef LIT_DEF
#define LIT_DEF

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "header.h"
#include "error.h"

#define TYPE_IDENT '\x00'
#define TYPE_STR '\x01'
#define TYPE_NUM '\x02'
#define TYPE_LIST '\x03'
#define TYPE_DICT '\x05'
#define TYPE_PAIR '\x06'
#define TYPE_FRAC '\x07'
// not a type, a flag for
// short variants of other
// types
#define TYPE_SHORT '\x80'

#if __BYTE_ORDER == __BIG_ENDIAN
#define ntohll(x) (x)
#else
#define ntohll(x) ntohll_(x)
#endif
#define htonll ntohll

union double_or_uint64_t {
	double d;
	uint64_t i;
};

uint64_t ntohll_(uint64_t);

bool read_literals(char*, size_t, Header*);
V get_literal(Header*, uint32_t);

#endif
