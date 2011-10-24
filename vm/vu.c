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
//#include "env.h"

Error run_file(V global, V file_name)
{
	Error e = Nothing;
	V file = load_file(file_name, global);
	if (file == NULL)
	{
		return IllegalFile;
	}
	Stack *S = new_stack();
	Stack *scope = new_stack();
	push(scope, new_file_scope(file));
	while (e == Nothing)
	{
		e = do_instruction(&toFile(file)->header, S, scope);
		toScope(get_head(scope))->pc++;
	}
	if (e != Exit) //uh oh
	{
		printf("An error occurred.\n");
		//at this point we can use the scope stack to produce a traceback
	}
	//clean-up
	clear_stack(S);
	clear_stack(scope);
	clear_ref(file);
	return e;
}

void run(V file_name)
{
	V global = new_global_scope();
	open_std_lib(&toScope(global)->hm);
	Error e = run_file(global, file_name);
	//Do something with e
}

bool exists(char* fname)
{
	FILE *f = fopen(fname, "r");
	if (f == NULL)
	{
		return false;
	}
	else
	{
		fclose(f);
		return true;
	}
}

int main(int argc, char *argv[])
{
	if (argc > 1 && exists(argv[1]))
	{
		run(a_to_value(argv[1]));
	}
	return 0;
}
