#ifndef VALUE_DEF
#define VALUE_DEF

#include <stdbool.h>
#include <stdint.h>

#define isInt(x) ((long int)x & 1)
#define canBeInt(x) (x > INTPTR_MIN >> 1 && x < INTPTR_MAX >> 1)
#define toInt(x) ((long int)x>>1)
#define intToV(x) ((V)(((x) << 1) + 1))
#define isCFunc(x) ((long int)(x) & 2)
#define toCFunc(x) ((CFuncP)((long int)(x) & ~2))
#define cFuncToV(x) ((V)((long int)(x) | 2))

#define toFile(x) ((File*)(x + 1))
#define toScope(x) ((Scope*)(x + 1))
#define toFunc(x) ((Func*)(x + 1))
#define toStack(x) ((Stack*)(x + 1))
#define toIdent(x) ((ITreeNode*)(x))
#define toDouble(x) (*(double*)(x + 1))
#define toNumber(x) (isInt(x) ? (double)toInt(x) : toDouble(x))
#define toHashMap(x) ((HashMap*)(x + 1))
#define getType(x) (isInt(x) ? T_NUM : isCFunc(x) ? T_CFUNC : x->type)
#define toFirst(x) (*((V*)(x + 1)))
#define toSecond(x) (*((V*)(x + 2)))
#define toNumerator(x) (((Frac*)(x + 1))->numerator)
#define toDenominator(x) (((Frac*)(x + 1))->denominator)
#define getNumer(x) ((frac_long)toNumerator(x))
#define getDenom(x) ((frac_long)toDenominator(x))
#define toBlob(x) ((Blob*)(x + 1))

#define new_dict() new_sized_dict(16)

#define pushS(v) push(S, add_rooted(v))
#define popS() clear_rooted(pop(S))

// Déjà Vu utilises the synchronous cycle collection algorithm
// described by David F. Bacon and V.T. Rajan (2001)
typedef enum GCColor
{
	Black,	//In use or free
	Gray,	//Possible member of cycle
	White,	//Member of garbage cycle
	Purple,	//Possible root of cycle
	Green	//Acyclic
} GCColor;

typedef struct Value
{
	uint8_t type;
	uint8_t buffered : 1;
	uint8_t /*GCColor*/ color : 7;
	uint16_t baserefs;
	uint32_t refs;
} Value;

typedef Value* V;

typedef struct
{
	long int numerator;
	long int denominator;
} Frac;

typedef struct TreeNode {
	uint8_t type;
	uint32_t length;
	struct TreeNode *left;
	struct TreeNode *right;
	char data[1]; // That length is a white lie.
} ITreeNode;

#ifdef __GNUC__
typedef __int128_t frac_long;
#endif

V int_to_value(long int);
V double_to_value(double);
V get_ident(const char*);
V new_list();
V new_sized_dict();
V new_pair(V, V);
V new_frac(frac_long, frac_long);

bool truthy(V);
bool equal(V, V);

long int pair_ordinal(V);

#endif
