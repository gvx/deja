#include "error.h"
#include "strings.h"

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
	error_names[7] = get_ident("unicode-error");
	error_names[8] = get_ident("interrupt");
	error_names[9] = get_ident("error");
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
		case UnicodeError:
			return "Error decoding string";
		case Interrupt:
			return "Process interrupted";
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
	fputs(error_name(e), stderr);
	if (error_msg)
	{
		fputs(": ", stderr);
		fputs(error_msg, stderr);
	}

	if (lastCall)
	{
		ITreeNode *s = toIdent(lastCall);
		fprintf(stderr, " in %*s", s->length, s->data);
	}
	if (scope_arr == NULL)
	{
		fputs(".\n", stderr);
		return;
	}
	fputs(":\n", stderr);
	Scope *sc;
	int n;
	for (n = 0; n < scope_arr->used; n++)
	{
		sc = toScope(scope_arr->nodes[n]);
		if (n == scope_arr->used - 1 || toScope(scope_arr->nodes[n + 1])->is_func_scope)
		{
			NewString *s = toFile(sc->file)->source != NULL ? toNewString(toFile(sc->file)->source) : toNewString(toFile(sc->file)->name);
			if (sc->callname && getType(sc->callname) == T_IDENT)
			{
				if (sc->linenr > 0)
					fprintf(stderr, "%*s:%d in %*s\n", (int)s->size, s->text, sc->linenr, toIdent(sc->callname)->length, toIdent(sc->callname)->data);
				else
					fprintf(stderr, "%*s in %*s\n", (int)s->size, s->text, toIdent(sc->callname)->length, toIdent(sc->callname)->data);
			}
			else
			{
				if (sc->linenr > 0)
					fprintf(stderr, "%*s:%d\n", (int)s->size, s->text, sc->linenr);
				else
					fprintf(stderr, "%*s\n", (int)s->size, s->text);
			}
		}
	}
}
