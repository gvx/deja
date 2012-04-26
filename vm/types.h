#ifndef TYPE_DEF
#define TYPE_DEF

// Section 0x0*: visible types
#define T_IDENT 0x00
#define T_STR 0x01
#define T_NUM 0x02
#define T_STACK 0x03
#define T_FUNC 0x04
#define T_DICT 0x05
#define T_PAIR 0x06
// Section 0x1*: internal types
#define T_SCOPE 0x10
#define T_FILE 0x11
#define T_CFUNC 0x12
// Section 0xF*: implicit types
#define T_NIL 0xFF

#endif
