#ifndef ERR_DEF
#define ERR_DEF

#include "stack.h"
#include "scope.h"

#include <stdlib.h>
#include <stdio.h>

typedef enum
{
	Nothing,
	Exit,
	NameError,
	ValueError,
	TypeError,
	StackEmpty,
	IllegalFile,
	UserError
} Error;

void handle_error(Error, Stack*);

#endif