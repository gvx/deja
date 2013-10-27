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
	UnicodeError,
	Interrupt,
	UserError,
	UnknownError,
} Error;

V error_names[UnknownError];
char *error_msg;

#define set_error_msg_ref(s) do { free(error_msg); error_msg = (s);} while (0)
#define set_error_msg(s) set_error_msg_ref(strdup(s))

void init_errors();

V error_to_ident(Error e);
Error ident_to_error(V e);

void handle_error(Error, Stack*);

#endif
