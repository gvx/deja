#include "run.h"
#include "lib.h"
#include "std.h"
#include "error.h"
#include "debug.h"
#include "persist.h"

bool reraise;
bool vm_silent = false;
bool vm_debug = false;
bool vm_persist = false;

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
	push(scope, add_rooted(new_file_scope(file)));
	if (vm_persist)
	{
		V stdinfile = load_stdin(global);
		if (stdinfile == NULL)
		{
			handle_error(IllegalFile, NULL);
			return;
		}
		push(scope, add_rooted(new_file_scope(stdinfile)));
	}
	//loading the dva part of the standard library
	push(scope, add_rooted(new_file_scope(load_std(global))));
	while (e == Nothing)
	{
		sc = toScope(get_head(scope));
		sc->pc++;
		reraise = false;
		e = do_instruction(&toFile(sc->file)->header, S, scope);
		if (e != Nothing && e != Exit)
		{
			DBG_PRINTF("Error %d %sraised", e, reraise ? "re" : "");
			if (!reraise)
			{
				while (stack_size(save_scopes) > 0)
				{
					clear_base_ref(pop(save_scopes));
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
		if (stack_size(S) && !vm_silent)
		{
			puts("Result:");
			for (i = 0; i < stack_size(S); i++)
			{
				print_value(S->nodes[i], 0);
				putchar(10);
			}
		}
		if (vm_persist)
		{
			persist_all_file(stdout, S);
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
