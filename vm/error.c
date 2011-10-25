#include "error.h"

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
			return "File not found";
		case UserError:
			return "Error";
		default:
			return "Unknown error occurred";
	}
}

void handle_error(Error e, Stack *scope_arr)
{
	printf("%s:\n", error_name(e));
	Node *n = scope_arr->head;
	while (n != NULL)
	{
		//printf("%d\n", toScope(n->data)->linenr);
		n = n->next;
	}
}