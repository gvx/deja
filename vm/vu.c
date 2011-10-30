#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "opcodes.h"
#include "stack.h"
#include "value.h"
#include "header.h"
#include "error.h"
#include "file.h"
#include "scope.h"
#include "lib.h"
#include "module.h"

void run(V file_name)
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
	Stack *S = new_stack();
	Stack *scope = new_stack();
	Scope *sc;
	push(scope, new_file_scope(file));
	while (e == Nothing)
	{
		sc = toScope(get_head(scope));
		sc->pc++;
		e = do_instruction(&toFile(sc->file)->header, S, scope);
	}
	if (e != Exit) //uh oh
	{
		handle_error(e, scope);
	}
	//clean-up
	clear_stack(S);
	clear_stack(scope);
	clear_ref(file);
}

int main(int argc, char *argv[])
{
	init_path();
	init_errors();
	if (argc > 1)
	{
		run(find_file(get_ident(argv[1])));
	}
	return 0;
}
