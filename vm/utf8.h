#ifndef UTF8_DEF
#define UTF8_DEF

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef char utf8byte;
typedef utf8byte *utf8;
typedef size_t utf8index;
typedef uint32_t unichar;

unichar decode_codepoint(utf8, utf8index*);
bool valid_utf8(size_t, utf8);

#endif
