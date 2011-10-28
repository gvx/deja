#ifndef VALUE_DEF
#define VALUE_DEF

#include <stdbool.h>
#include <stdint.h>

#define toFile(x) ((File*)x->data.object)
#define toScope(x) ((Scope*)x->data.object)
#define toFunc(x) ((Func*)x->data.object)
#define toStack(x) ((Stack*)x->data.object)
#define toString(x) ((String*)x->data.object)
#define toNumber(x) (x->data.number)
#define toCFunc(x) ((Error (*)(Header*, Stack*, Stack*))x->data.object)

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
	int type;
	GCColor color;
	bool buffered;
	unsigned int refs;
	union
	{
		double number;
		void* object;
	} data;
} Value;

typedef Value* V;

typedef struct String
{
	uint32_t length;
	uint32_t hash;
	char* data;
} String;

uint32_t string_hash(int, const char*);

V int_to_value(int);
V double_to_value(double);
V a_to_value(char*);
V str_to_value(int, char*);
V get_ident(const char*);
V new_list();

bool truthy(V);

#endif
