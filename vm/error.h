#ifndef ERR_DEF
#define ERR_DEF

#include "stack.h"
#include "scope.h"
#include "file.h"

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
	UserError,
	UnknownError,
} Error;

V error_names[UnknownError];

void init_errors();

V error_to_ident(Error e);
Error ident_to_error(V e);

void handle_error(Error, Stack*);

#endif