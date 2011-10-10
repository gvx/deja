#ifndef VALUE_DEF
#define VALUE_DEF

#include <stdbool.h>

// Déjà Vu utilises the synchronous cycle collection algorithm
// described by David F. Bacon and V.T. Rajan (2001)
typedef enum GCColor {
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
		struct String* string;
		void* object;
	} data;
} Value;

typedef Value* V;

typedef struct String
{
	unsigned int length;
	char* data;
} String;

V int_to_value(int);
V double_to_value(double);
V a_to_value(char*);
V str_to_value(int, char*);
V newlist();

bool truthy(V);

#endif
