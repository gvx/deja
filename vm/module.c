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
	ITreeNode *id = toIdent(module_name);
	int l = id->length;
	char *mod_name = id->data;
	int i;
	for (i = 0; i < SEARCH_PATH_SIZE; i++)
	{
		if (!search_path[i])
		{
			break;
		}
		int length = strlen(search_path[i]) + l + 3; // ".vu"
		char *fname = malloc(length + 1);
		strncat(strcpy(fname, search_path[i]), mod_name, l + 1);
		if (!exists(fname))
		{
			strcat(fname, ".vu");
		}
		if (exists(fname))
		{
			set_hashmap(loaded, module_name, int_to_value(1));
			clear_ref(module_name);
			V s = str_to_string(length, fname);
			free(fname);
			return s;
		}
		free(fname);
	}
	set_error_msg_ref(malloc(23 + l));
	sprintf(error_msg, "could not find module %*s", l, mod_name);
	clear_ref(module_name);
	return NULL;
}

void init_path()
{
	loaded = new_hashmap(32);
	search_path[0] = ""; //absolute path
	int i = 1;
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
