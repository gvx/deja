#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utf8.h"
#include "run.h"

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
