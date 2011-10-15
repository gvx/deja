#ifndef OPCODES_DEF
#define OPCODES_DEF

#include "error.h"
#include "header.h"
#include "stack.h"

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
#define OP_LABDA          0x20
#define OP_ENTER_SCOPE    0x21
#define OP_LEAVE_SCOPE    0x22
#define OP_NEW_LIST       0x30
#define OP_DROP           0x40
#define OP_DUP            0x41
#define OP_LINE_NUMBER    0x50
#define OP_LINE_TEXT      0x51
#define OP_SOURCE_FILE    0x52

Error do_instruction(Header*, Stack*, Stack*);

#endif
