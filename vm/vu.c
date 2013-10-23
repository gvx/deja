#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "utf8.h"
#include "run.h"
#include "error.h"
#include "strings.h"
#include "eva.h"

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
		init_module_path();
		init_errors();
		V global = new_global_scope();
		V v_eva = open_std_lib(&toScope(global)->hm);
		Stack *S = new_stack();
		init_argv(argc - optind, argv + optind, v_eva);
		run(global, S);
		clear_stack(S);
	}
	return 0;
}
