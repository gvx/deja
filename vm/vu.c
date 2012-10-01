#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stack.h"
#include "lib.h"
#include "std.h"
#include "utf8.h"

bool reraise;

void run(V file_name, Stack *S)
{
	V global = new_global_scope();
	open_std_lib(&toScope(global)->hm);
	Error e = Nothing;
	V file = load_file(file_name, global);
	if (file == NULL)
	{
		handle_error(IllegalFile, NULL);
		return;
	}
	Stack *scope = new_stack();
	Stack *save_scopes = new_stack();
	Scope *sc;
	push(scope, new_file_scope(file));
	//loading the dva part of the standard library
	push(scope, new_file_scope(load_std(global)));
	while (e == Nothing)
	{
		sc = toScope(get_head(scope));
		sc->pc++;
		reraise = false;
		e = do_instruction(&toFile(sc->file)->header, S, scope);
		if (e != Nothing && e != Exit)
		{
			if (!reraise)
			{
				while (stack_size(save_scopes) > 0)
				{
					clear_ref(pop(save_scopes));
				}

			}
			do
			{
				push(save_scopes, pop(scope));
				sc = toScope(get_head(save_scopes));
			}
			while (stack_size(scope) > 0 && !sc->is_error_handler);
			if (stack_size(scope) > 0)
			{ //Let error be handled by code
				pushS(add_ref(error_to_ident(e)));
				e = Nothing;
			}
			else
			{ //Error slips away, uncaught
				while (stack_size(save_scopes) > 0)
				{
					push(scope, pop(save_scopes));
				}
			}
		}
	}
	if (e == Exit)
	{
		int i;
		if (stack_size(S))
		{
			puts("Result:");
			for (i = stack_size(S) - 1; i >= 0; i--)
			{
				print_value(S->nodes[i], 0);
				putchar(10);
			}
		}
	}
	else //uh oh
	{
		handle_error(e, scope);
	}
	//clean-up
	clear_stack(scope);
	clear_stack(save_scopes);
	clear_ref(file);
}

int main(int argc, char *argv[])
{
	int i;
	init_path();
	init_errors();
	if (argc > 1)
	{
		Stack *S = new_stack();

		for (i = argc - 1; i > 1; i--)
		{
			if (!valid_utf8(strlen(argv[i]), argv[i]))
			{
				handle_error(UnicodeError, NULL);
				clear_stack(S);
				return 1;
			}
			pushS(a_to_value(argv[i]));
		}

		run(find_file(get_ident(argv[1])), S);
		clear_stack(S);
	}
	return 0;
}
