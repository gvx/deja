#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "utf8.h"
#include "run.h"
#include "error.h"
#include "module.h"

extern bool vm_silent;
extern bool vm_debug;
extern bool vm_persist;

int main(int argc, char *argv[])
{
	const struct option options[] = {
		{"help", no_argument, NULL, 'h'},
		{"debug", no_argument, NULL, 'd'},
		{"version", no_argument, NULL, 'v'},
		{"silent", no_argument, NULL, 's'},
		{"persist", no_argument, NULL, 'p'},
		{0, 0, 0, 0},
	};
	char opt;
	int i;
	while ((opt = getopt_long(argc, argv, "+hdvsp", options, NULL)) != -1)
	{
		switch (opt)
		{
		case 'h':
			puts("Usage: vu [OPTIONS] module [STACK]\n"
			     "  -h, --help     Show this help message and exit\n"
			     "  -v, --version  Show the VM version and exit\n"
			     "  -d, --debug    Enable debugging\n"
			     "  -s, --silent   Do not print the stack after running\n"
			     "  -p, --persist  Use standard input and output to persist the stack\n"
			     "                 This option is intended for internal use; implies --silent");
			return 0;
		case 'v':
			printf("vu virtual machine 0.1\nbyte code protocol %d.%d\n", VERSION >> 4, VERSION & 15);
			return 0;
		case 's':
			vm_silent = true;
			break;
		case 'd':
			vm_debug = true;
			break;
		case 'p':
			vm_persist = true;
			vm_silent = true;
			break;
		}
	}
	if (argc - optind > 0)
	{
		init_path();
		init_errors();
		Stack *S = new_stack();

		for (i = argc - 1; i > optind; i--)
		{
			if (!valid_utf8(strlen(argv[i]), argv[i]))
			{
				handle_error(UnicodeError, NULL);
				clear_stack(S);
				return 1;
			}
			pushS(a_to_value(argv[i]));
		}

		run(find_file(get_ident(argv[optind])), S);
		clear_stack(S);
	}
	return 0;
}
