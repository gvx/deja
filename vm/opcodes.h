#ifndef OPCODES_DEF
#define OPCODES_DEF

#include "header.h"
#include "stack.h"
#include "file.h"

#define OP_PUSH_LITERAL   0x00
#define OP_PUSH_INTEGER   0x01
#define OP_PUSH_WORD      0x02
#define OP_SET            0x03
#define OP_SET_LOCAL      0x04
#define OP_SET_GLOBAL     0x05
#define OP_GET            0x06
#define OP_GET_GLOBAL     0x07
#define OP_JMP            0x10
#define OP_JMPZ           0x11
#define OP_RETURN         0x12
#define OP_RECURSE        0x13
#define OP_JMPEQ          0x14
#define OP_JMPNE          0x15
#define OP_LABDA          0x20
#define OP_ENTER_SCOPE    0x21
#define OP_LEAVE_SCOPE    0x22
#define OP_NEW_LIST       0x30
#define OP_POP_FROM       0x31
#define OP_PUSH_TO        0x32
#define OP_PUSH_THROUGH   0x33
#define OP_DROP           0x40
#define OP_DUP            0x41
#define OP_SWAP           0x42
#define OP_ROT            0x43
#define OP_OVER           0x44
#define OP_LINE_NUMBER    0x50
#define OP_SOURCE_FILE    0x52
#define OP_ENTER_ERRHAND  0x60
#define OP_LEAVE_ERRHAND  0x61
#define OP_RAISE          0x62
#define OP_RERAISE        0x63
#define OP_NEW_DICT       0x70
#define OP_HAS_DICT       0x71
#define OP_GET_DICT       0x72
#define OP_SET_DICT       0x73
#define OP_CALL           0x80

Error inline do_instruction(Header*, Stack*, Stack*);

#endif
