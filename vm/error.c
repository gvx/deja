#include "error.h"

V lastCall = NULL;

void init_errors(void)
{
	error_msg = NULL;
	error_names[0] = get_ident("nothing");
	error_names[1] = get_ident("exit");
	error_names[2] = get_ident("name-error");
	error_names[3] = get_ident("value-error");
	error_names[4] = get_ident("type-error");
	error_names[5] = get_ident("stack-empty");
	error_names[6] = get_ident("illegal-file");
	error_names[7] = get_ident("error");
}

char* error_name(Error e)
{
	switch (e)
	{
		case NameError:
			return "Name does not exist";
		case ValueError:
			return "Wrong value";
		case TypeError:
			return "Wrong type";
		case StackEmpty:
			return "The stack is empty";
		case IllegalFile:
			return "Not a valid program";
		case UserError:
			return "Error";
		default:
			return "Unknown error occurred";
	}
}

V error_to_ident(Error e)
{
	return error_names[(int)e];
}

Error ident_to_error(V e)
{
	int i;
	for (i = 0; i < UnknownError; i++)
	{
		if (error_names[i] == e)
		{
			return i;
		}
	}
	return UnknownError;
}

void handle_error(Error e, Stack *scope_arr)
{
	if (error_msg)
		puts(error_msg);
	else
		puts(error_name(e));
	if (lastCall)
	{
		String *s = toString(lastCall);
		printf("In %*s:\n", s->length, toCharArr(s));
	}
	if (scope_arr == NULL)
	{
		return;
	}
	Node *n = scope_arr->head;
	Scope *sc;
	bool show_next = true;
	while (n != NULL)
	{
		sc = toScope(n->data);
		if (show_next)
		{
			String *s = toFile(sc->file)->source != NULL ? toString(toFile(sc->file)->source) : toString(toFile(sc->file)->name);
			printf("%s:%d\n", toCharArr(s), sc->linenr);
		}
		n = n->next;
		show_next = sc->is_func_scope || (n && n->next == NULL);
	}
}
