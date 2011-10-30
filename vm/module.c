#include "module.h"

#define SEARCH_PATH_SIZE 32

static char *search_path[SEARCH_PATH_SIZE];
static HashMap *loaded;

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

V find_file(V module_name)
{
	V a = get_hashmap(loaded, module_name);
	if (a)
	{
		return a;
	}
	char *mod_name = toString(module_name)->data;
	int l = toString(module_name)->length;
	int i;
	for (i = 0; i < SEARCH_PATH_SIZE; i++)
	{
		if (!search_path[i])
		{
			break;
		}
		int length = strlen(search_path[i]) + l;
		char *fname = malloc(length + 1);
		strncat(strcpy(fname, search_path[i]), mod_name, l + 1);
		if (exists(fname))
		{
			set_hashmap(loaded, module_name, int_to_value(1));
			clear_ref(module_name);
			return str_to_value(length, fname);
			break;
		}
		free(fname);
	}
	clear_ref(module_name);
	return NULL;
}

void init_path()
{
	loaded = new_hashmap(32);
	int i = 0;
	char *env = getenv("DEJAVUPATH");
	if (env == NULL)
	{
		env = ".:/usr/local/share/deja";
	}
	char *start = env;
	while (i < SEARCH_PATH_SIZE - 1)
	{
		if (*env == ':' || *env == '\0')
		{
			size_t length = env - start;
			char *n = malloc(length + 2);
			strncpy(n, start, length);
			n[length] = '/';
			n[length + 1] = '\0';
			search_path[i++] = n;
			start = env + 1;
		}
		if (!*env)
		{
			break;
		}
		env++;
	}
}