#include "error.h"

void init_errors(void)
{
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
	String *s = toString(e);
	for (i = 0; i < UnknownError; i++)
	{
		if (toString(error_names[i])->length == s->length && !memcmp(toString(error_names[i])->data, s->data, s->length))
		{
			clear_ref(e);
			return i;
		}
	}
	clear_ref(e);
	return UnknownError;
}

void handle_error(Error e, Stack *scope_arr)
{
	printf("%s\n", error_name(e));
	if (scope_arr == NULL)
	{
		return;
	}
	StackArray *sa = scope_arr->head;
	Scope *sc;
	bool show_next = true;
	int itotal = 1;
	while (sa != NULL)
	{
		int i;
		for (i = 0; i < sa->numitems; i++)
		{
			sc = toScope(sa->items[i]);
			if (show_next)
			{
				String *s = toFile(sc->file)->source != NULL ? toString(toFile(sc->file)->source) : toString(toFile(sc->file)->name);
				printf("%s:%d\n", s->data, sc->linenr);
			}
			itotal++;
			show_next = sc->is_func_scope || (itotal == stack_size(scope_arr));
		}
		sa = sa->next;
	}
}