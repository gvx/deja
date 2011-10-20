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


int run_file(V file_name)
{
	Error e = Nothing;
	V global = new_global_scope();
	V file = load_file(file_name, global);
	Stack *S = newstack();
	Stack *scope = newstack();
	push(scope, new_file_scope(file));
	open_lib(&toScope(global)->hm);
	while (e == Nothing)
	{
		e = do_instruction(&toFile(file)->header, S, scope);
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
	return 0;
}


int main(int argc, char *argv[])
{
	return 0;
}
