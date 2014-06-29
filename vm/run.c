#include "run.h"
#include "lib.h"
#include "std.h"
#include "compiler.h"
#include "error.h"
#include "debug.h"
#include "persist.h"

Error print_stack(Stack*, Stack*);

bool reraise;
bool vm_silent = false;
bool vm_debug = false;
bool vm_persist = false;
bool vm_interrupt = false;
Error last_error = Nothing;

Stack *traceback;

void run(V global, Stack *S)
{
	Scope *sc;
	Error e = Nothing;
	Stack *scope = new_stack();
	traceback = new_stack();
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
	//loading compiler
	push(scope, add_rooted(new_file_scope(load_compiler(global))));
	//loading the dva part of the standard library
	push(scope, add_rooted(new_file_scope_env(load_std(global), add_ref(toScope(global)->env))));
	while (e == Nothing)
	{
		sc = toScope(get_head(scope));
		sc->pc++;
		reraise = false;
		e = do_instruction(&toFile(sc->file)->header, S, scope);
		if (vm_interrupt)
		{
			vm_interrupt = false;
			e = Interrupt;
		}
		if (e != Nothing && e != Exit)
		{
			DBG_PRINTF("Error %d %sraised", e, reraise ? "re" : "");
			if (!reraise)
			{
				while (stack_size(traceback) > 0)
				{
					clear_base_ref(pop(traceback));
				}
				last_error = e;
			}
			else
			{
				e = last_error;
			}
			do
			{
				push(traceback, pop(scope));
				sc = toScope(get_head(traceback));
			}
			while (stack_size(scope) > 0 && !sc->is_error_handler);
			if (stack_size(scope) > 0)
			{ //Let error be handled by code
				pushS(add_ref(error_to_ident(e)));
				e = Nothing;
			}
			else
			{ //Error slips away, uncaught
				while (stack_size(traceback) > 0)
				{
					push(scope, pop(traceback));
				}
			}
		}
	}
	if (e == Exit)
	{
		if (stack_size(S) && !vm_silent)
		{
			puts("Result:");
			print_stack(S, NULL);
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
	clear_stack(traceback);
}
