#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "eva.h"
#include "blob.h"
#include "strings.h"
#include "literals.h"
#include "prompt.h"

Error decode(Stack *S, Stack *scope_arr)
{
	require(2);
	V encoding = popS();
	V raw_data = popS();
	if (getType(encoding) != T_IDENT || getType(raw_data) != T_BLOB)
	{
		clear_ref(encoding);
		clear_ref(raw_data);
		return TypeError;
	}
	//just assume UTF-8 for now
	if (!valid_utf8(toBlob(raw_data)->size, (utf8)toBlob(raw_data)->data))
	{
		clear_ref(encoding);
		clear_ref(raw_data);
		return UnicodeError;
	}
	V output = str_to_string(toBlob(raw_data)->size, (utf8)toBlob(raw_data)->data);
	pushS(output);
	clear_ref(encoding);
	clear_ref(raw_data);
	return Nothing;
}

#define ENSURE_BIG_ENOUGH(extrasize) while (index + (extrasize) >= toBlob(output)->size) { resize_blob(output, 2 * toBlob(output)->size); }

int quote_string(V output, int index, char *text, size_t size)
{
	int copysize;
	//under-estimation, needs to account for escape sequences
	ENSURE_BIG_ENOUGH(size + 2);
	setbyte_blob(output, index++, '"');
	int x, lastindex = 0;
	for (x = 0; x < size; x++)
	{
		unsigned char current = text[x];
		char *special = "\\\t\n\r\"";
		char *special_index;
		if (current && (special_index = strchr(special, current)))
		{
			copysize = x - lastindex;
			ENSURE_BIG_ENOUGH(copysize + 2);
			memcpy(toBlob(output)->data + index, text + lastindex, copysize);
			index += copysize;
			setbyte_blob(output, index++, '\\');
			setbyte_blob(output, index++, "\\tnrq"[special_index - special]);
			lastindex = x + 1;
		}
		else if (current < 32)
		{
			copysize = x - lastindex;
			ENSURE_BIG_ENOUGH(copysize + 5);
			memcpy(toBlob(output)->data + index, text + lastindex, copysize);
			index += copysize;
			setbyte_blob(output, index++, '\\');
			setbyte_blob(output, index++, '{');
			if (current > 9)
				setbyte_blob(output, index++, '0' + (current / 10));
			setbyte_blob(output, index++, '0' + (current % 10));
			setbyte_blob(output, index++, '}');
			lastindex = x + 1;
		}
	}
	copysize = x - lastindex;
	ENSURE_BIG_ENOUGH(copysize + 1);
	memcpy(toBlob(output)->data + index, text + lastindex, copysize);
	index += copysize;
	setbyte_blob(output, index++, '"');
	return index;
}

bool is_a_set(HashMap *obj)
{
	if (dictDefault(obj) != v_false)
	{
		return false;
	}
	if (obj->map != NULL)
	{
		int i;
		for (i = 0; i < obj->size; i++)
		{
			Bucket *b = obj->map[i];
			while (b)
			{
				if (b->value != v_true)
				{
					return false;
				}
				b = b->next;
			}
		}
	}
	return true;
}

int encode_quoted(V output, int index, V object, int level)
{
	ITreeNode* i;
	switch (getType(object))
	{
		case T_IDENT:
			i = toIdent(object);
			ENSURE_BIG_ENOUGH(i->length + 1);
			setbyte_blob(output, index++, ':');
			memcpy(toBlob(output)->data + index, i->data, i->length);
			index += i->length;
			break;
		case T_STR:
			index = quote_string(output, index, toNewString(object)->text, toNewString(object)->size);
			break;
		case T_NUM:
			if (object == v_true)
			{
				ENSURE_BIG_ENOUGH(4);
				memcpy(toBlob(output)->data + index, "true", 4);
				index += 4;
			}
			else if (object == v_false)
			{
				ENSURE_BIG_ENOUGH(5);
				memcpy(toBlob(output)->data + index, "false", 5);
				index += 5;
			}
			else
			{
				ENSURE_BIG_ENOUGH(34);
				index += sprintf((char*)toBlob(output)->data + index, "%.15g", toNumber(object));
			}
			break;
		case T_LIST:
			if (level < 4)
			{
				ENSURE_BIG_ENOUGH(3);
				setbyte_blob(output, index++, '[');
				setbyte_blob(output, index++, ' ');
				int i;
				for (i = 0; i < toStack(object)->used; i++)
				{
					index = encode_quoted(output, index, toStack(object)->nodes[i], level + 1);
					ENSURE_BIG_ENOUGH(1);
					setbyte_blob(output, index++, ' ');
				}
				ENSURE_BIG_ENOUGH(1);
				setbyte_blob(output, index++, ']');
			}
			else
			{
				ENSURE_BIG_ENOUGH(5);
				memcpy(toBlob(output)->data + index, "[...]", 5);
				index += 5;
			}
			break;
		case T_DICT:
			if (level < 4)
			{
				if (is_a_set(toHashMap(object)))
				{
					ENSURE_BIG_ENOUGH(5);
					memcpy(toBlob(output)->data + index, "set{ ", 5);
					index += 5;
					int i;
					HashMap *hm = toHashMap(object);
					if (hm->map != NULL)
					{
						for (i = 0; i < hm->size; i++)
						{
							Bucket *b = hm->map[i];
							while (b)
							{
								index = encode_quoted(output, index, b->key, level + 1);
								ENSURE_BIG_ENOUGH(1);
								setbyte_blob(output, index++, ' ');
								b = b->next;
							}
						}
					}
					ENSURE_BIG_ENOUGH(1);
					setbyte_blob(output, index++, '}');
				}
				else
				{
					ENSURE_BIG_ENOUGH(2);
					setbyte_blob(output, index++, '{');
					setbyte_blob(output, index++, ' ');
					int i;
					HashMap *hm = toHashMap(object);
					if (hm->map != NULL)
					{
						for (i = 0; i < hm->size; i++)
						{
							Bucket *b = hm->map[i];
							while (b)
							{
								index = encode_quoted(output, index, b->key, level + 1);
								ENSURE_BIG_ENOUGH(1);
								setbyte_blob(output, index++, ' ');
								index = encode_quoted(output, index, b->value, level + 1);
								ENSURE_BIG_ENOUGH(1);
								setbyte_blob(output, index++, ' ');
								b = b->next;
							}
						}
					}
					ENSURE_BIG_ENOUGH(1);
					setbyte_blob(output, index++, '}');
				}
			}
			else
			{
				ENSURE_BIG_ENOUGH(5);
				memcpy(toBlob(output)->data + index, "{...}", 5);
				index += 5;
			}
			break;
		case T_PAIR:
			// note: pairs are not cyclic, so no need to increase the level
			ENSURE_BIG_ENOUGH(2);
			memcpy(toBlob(output)->data + index, "& ", 2);
			index += 2;
			index = encode_quoted(output, index, toFirst(object), level);
			ENSURE_BIG_ENOUGH(1);
			setbyte_blob(output, index++, ' ');
			index = encode_quoted(output, index, toSecond(object), level);
			break;
		case T_FRAC:
			ENSURE_BIG_ENOUGH(42);
			index += sprintf((char*)toBlob(output)->data + index, "%ld/%ld", toNumerator(object), toDenominator(object));
			break;
		case T_CFUNC:
		case T_FUNC:
		case T_SCOPE:
			ENSURE_BIG_ENOUGH(6);
			memcpy(toBlob(output)->data + index, "(func)", 6);
			index += 6;
			break;
		case T_BLOB:
			ENSURE_BIG_ENOUGH(6);
			memcpy(toBlob(output)->data + index, "(blob:", 6);
			index += 6;
			int size = toBlob(object)->size > 31 ? 30 : toBlob(object)->size;
			int i;
			bool is_printable_ascii = true;
			for (i = 0; i < size; i++)
			{
				unsigned char c = toBlob(object)->data[i];
				if (c >= 128)
				{
					is_printable_ascii = false;
					break;
				}
			}
			if (is_printable_ascii)
			{
				index = quote_string(output, index, (char*)toBlob(object)->data, size);
			}
			else
			{
				ENSURE_BIG_ENOUGH(size * 2);
				for (i = 0; i < size; i++)
				{
					int x = toBlob(object)->data[i];
					setbyte_blob(output, index + i * 2, "0123456789abcdef"[x / 16]);
					setbyte_blob(output, index + i * 2 + 1, "0123456789abcdef"[x % 16]);
				}
				index += size * 2;
			}
			if (size < toBlob(object)->size)
			{
				ENSURE_BIG_ENOUGH(4);
				memcpy(toBlob(output)->data + index, "...)", 4);
				index += 4;
			}
			else
			{
				ENSURE_BIG_ENOUGH(1);
				setbyte_blob(output, index++, ')');
			}
			break;
	}
	return index;
}

Error encode(Stack *S, Stack *scope_arr)
{
	require(2);
	V encoding = popS();
	V cooked_data = popS();
	if (getType(encoding) != T_IDENT)
	{
		clear_ref(encoding);
		clear_ref(cooked_data);
		return TypeError;
	}
	if (encoding == get_ident("utf-8"))
	{
		if (getType(cooked_data) != T_STR)
		{
			clear_ref(encoding);
			clear_ref(cooked_data);
			return TypeError;
		}
		V output = new_blob(toNewString(cooked_data)->size);
		memcpy(toBlob(output)->data, toNewString(cooked_data)->text, toNewString(cooked_data)->size);
		pushS(output);
		clear_ref(encoding);
		clear_ref(cooked_data);
		return Nothing;
	}
	else if (encoding == get_ident("ieee-754"))
	{
		if (getType(cooked_data) != T_NUM)
		{
			clear_ref(encoding);
			clear_ref(cooked_data);
			return TypeError;
		}
		V output = new_blob(8);
		union double_or_uint64_t v;
		v.d = toNumber(cooked_data);
		v.i = htonll(v.i);
		memcpy(toBlob(output)->data, &v, 8);
		pushS(output);
		clear_ref(encoding);
		clear_ref(cooked_data);
		return Nothing;
	}
	else if (encoding == get_ident("quoted"))
	{
		V output = new_blob(256);
		resize_blob(output, encode_quoted(output, 0, cooked_data, 0));
		pushS(output);
		clear_ref(encoding);
		clear_ref(cooked_data);
		return Nothing;
	}
	else
	{
		clear_ref(encoding);
		clear_ref(cooked_data);
		set_error_msg("unrecognised encoding");
		return ValueError;
	}
}

Error print_stack(Stack *S, Stack *scope_arr)
{
	int i;
	V output = new_blob(256);
	putchar('[');
	for (i = S->used - 1; i >= 0; i--)
	{
		putchar(' ');
		fwrite(toBlob(output)->data, 1, encode_quoted(output, 0, S->nodes[i], 0), stdout);
	}
	if (S->used)
	{
		putchar(' ');
	}
	putchar(']');
	putchar('\n');
	clear_ref(output);
	return Nothing;
}

#define READ_BUFF_SIZE 1048576
Error read_file(Stack *S, Stack *scope_arr)
{
	require(1);
	V file_name = popS();
	if (getType(file_name) == T_IDENT && file_name == get_ident("stdin"))
	{
		V output = new_blob(READ_BUFF_SIZE);
		size_t extrasize = 0;
		size_t nread;
		while ((nread = fread(toBlob(output)->data + extrasize, 1, READ_BUFF_SIZE, stdin)) == READ_BUFF_SIZE)
		{
			extrasize += READ_BUFF_SIZE;
			resize_blob(output, extrasize + READ_BUFF_SIZE);
		}
		resize_blob(output, extrasize + nread);
		pushS(output);
	}
	else if (getType(file_name) == T_STR)
	{
		FILE *input = fopen(toNewString(file_name)->text, "rb");
		fseek(input, 0L, SEEK_END);
		size_t file_size = ftell(input);
		fseek(input, 0L, SEEK_SET);
		V output = new_blob(file_size);
		size_t nread = fread(toBlob(output)->data, 1, READ_BUFF_SIZE, input);
		assert(nread == file_size);
		fclose(input);
		pushS(output);
	}
	else
	{
		clear_ref(file_name);
		return TypeError;
	}
	clear_ref(file_name);
	return Nothing;
}

#define READ_LINE_BUFF_SIZE 256
Error read_line(Stack* S, Stack* scope_arr)
{
	require(1);
	V file_name = popS();
	if (getType(file_name) == T_IDENT && file_name == get_ident("stdin"))
	{
		V output = new_blob(READ_LINE_BUFF_SIZE);
		size_t extrasize = 0;
		while(fgets((char*)toBlob(output)->data + extrasize, READ_LINE_BUFF_SIZE, stdin))
		{
			char last = toBlob(output)->data[toBlob(output)->size - 1];
			if (!last || last == '\n')
			{
				break;
			}
			extrasize += READ_LINE_BUFF_SIZE;
			resize_blob(output, extrasize + READ_LINE_BUFF_SIZE);
		}
		size_t len = strlen((char*)toBlob(output)->data);
		if (len == 0)
		{
			pushS(get_ident("eof"));
			clear_ref(file_name);
			clear_ref(output);
			return Nothing;
		}
		if (toBlob(output)->data[len - 1] == '\n')
		{
			len--;
		}
		resize_blob(output, len);
		pushS(output);
	}
	else
	{
		clear_ref(file_name);
		return TypeError;
	}
	clear_ref(file_name);
	return Nothing;
}

Error read_prompt(Stack *S, Stack *scope_arr)
{
	require(1);
	V prompt_line = popS();
	if (getType(prompt_line) != T_STR)
	{
		clear_ref(prompt_line);
		return TypeError;
	}
	prompt_t strout;
	switch (prompt(toNewString(prompt_line)->text, strout))
	{
		case prompt_result_normal:
			if (!valid_utf8(strlen(strout), strout))
			{
				return UnicodeError;
			}
			pushS(a_to_string(strout));
			clear_ref(prompt_line);
			return Nothing;
		case prompt_result_interrupt:
			clear_ref(prompt_line);
			return Interrupt;
		case prompt_result_eof:
			pushS(get_ident("eof"));
			clear_ref(prompt_line);
			return Nothing;
	}
	return Nothing;
}

Error open_file(Stack *S, Stack *scope_arr)
{
	require(2);
	V mode = popS();
	V file_name = popS();
	if (getType(mode) != T_IDENT || getType(file_name) != T_STR)
	{
		clear_ref(mode);
		clear_ref(file_name);
		return TypeError;
	}
	char *m;
	if (mode == get_ident("read"))
	{
		m = "rb";
	}
	else if (mode == get_ident("write"))
	{
		m = "wb";
	}
	else if (mode == get_ident("append"))
	{
		m = "ab";
	}
	else if (mode == get_ident("read-write"))
	{
		m = "r+b";
	}
	else
	{
		set_error_msg("Invalid open mode, use one of :read, :write, :append or :read-write");
		clear_ref(mode);
		clear_ref(file_name);
		return ValueError;
	}
	FILE *file = fopen(toNewString(file_name)->text, m);
	V handle = new_sized_dict(1);
	V handle_int = int_to_value((long int)file);
	set_hashmap(toHashMap(handle), get_ident("handle"), handle_int);
	set_hashmap(toHashMap(handle), get_ident("file-name"), file_name);
	set_hashmap(toHashMap(handle), get_ident("mode"), mode);
	pushS(handle);
	clear_ref(handle_int);
	clear_ref(mode);
	clear_ref(file_name);
	return Nothing;
}

Error close_file(Stack *S, Stack *scope_arr)
{
	V file_obj = popS();
	if (getType(file_obj) != T_DICT)
	{
		clear_ref(file_obj);
		return TypeError;
	}
	V handle = get_dict(toHashMap(file_obj), get_ident("handle"));
	if (handle == NULL || getType(handle) != T_NUM)
	{
		clear_ref(file_obj);
		return TypeError;
	}
	FILE *file = (FILE*)(long int)toNumber(handle);
	fclose(file);
	return Nothing;
}

Error write_fragment(Stack *S, Stack *scope_arr)
{
	require(2);
	V file_obj = popS();
	V blob = popS();
	if (getType(blob) != T_BLOB)
	{
		clear_ref(file_obj);
		clear_ref(blob);
		return TypeError;
	}
	if (getType(file_obj) == T_IDENT)
	{
		if (file_obj == get_ident("stdout"))
		{
			fwrite(toBlob(blob)->data, 1, toBlob(blob)->size, stdout);
			clear_ref(file_obj);
			clear_ref(blob);
			return Nothing;
		}
		else if (file_obj == get_ident("stderr"))
		{
			fwrite(toBlob(blob)->data, 1, toBlob(blob)->size, stderr);
			clear_ref(file_obj);
			clear_ref(blob);
			return Nothing;
		}
	}
	if (getType(file_obj) != T_DICT)
	{
		clear_ref(file_obj);
		clear_ref(blob);
		return TypeError;
	}
	V handle = get_dict(toHashMap(file_obj), get_ident("handle"));
	if (handle == NULL || getType(handle) != T_NUM)
	{
		clear_ref(file_obj);
		clear_ref(blob);
		clear_ref(handle);
		return TypeError;
	}
	FILE *file = (FILE*)(long int)toNumber(handle);

	if (fwrite(toBlob(blob)->data, 1, toBlob(blob)->size, file) < toBlob(blob)->size)
	{
		//return IOError?
	}

	clear_ref(file_obj);
	clear_ref(blob);
	clear_ref(handle); //close open files?
	return Nothing;
}

#define SEARCH_PATH_SIZE 32

static char *search_path[SEARCH_PATH_SIZE];

Error find_module(Stack *S, Stack *scope_arr)
{
	require(1);
	V module_name = popS();
	if (getType(module_name) != T_IDENT)
	{
		clear_ref(module_name);
		return TypeError;
	}
	ITreeNode *id = toIdent(module_name);
	int l = id->length;
	char *mod_name = id->data;
	int i;
	struct stat deja_file;
	struct stat vu_file;
	for (i = 0; i < SEARCH_PATH_SIZE; i++)
	{
		if (!search_path[i])
		{
			break;
		}
		int length = strlen(search_path[i]) + l + 5; // longest of ".vu" and ".deja"
		char *fname = malloc(length + 1);
		strcat(strncat(strcpy(fname, search_path[i]), mod_name, l + 1), ".deja");
		int deja_exists = stat(fname, &deja_file) == 0;
		fname[length - 4] = '\0';
		strcat(fname, "vu");
		int vu_exists = stat(fname, &vu_file) == 0;
		if (vu_exists || deja_exists)
		{
			pushS(str_to_string(length - 2, fname));

			if (!vu_exists || difftime(deja_file.st_mtime, vu_file.st_mtime) > 0)
			{
				fname[length - 4] = '\0';
				strcat(fname, "deja");
				pushS(str_to_string(length, fname));
			}
			else
			{
				pushS(add_ref(v_false));
			}
			free(fname);
			clear_ref(module_name);
			return Nothing;
		}
		free(fname);
	}
	set_error_msg_ref(malloc(23 + l));
	sprintf(error_msg, "could not find module %*s", l, mod_name);
	clear_ref(module_name);
	return IllegalFile;
}

void init_module_path()
{
	int i = 0;
	char *env = getenv("DEJAVUPATH");
	if (env == NULL)
	{
		env = ".:/usr/local/lib/deja";
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

Error run_file(Stack *S, Stack *scope_arr)
{
	require(1);
	V fname = popS();
	if (getType(fname) != T_STR)
	{
		clear_ref(fname);
		return TypeError;
	}
	V file = load_file(fname, toFile(toScope(get_head(scope_arr))->file)->global);
	if (file == NULL)
	{
		clear_ref(fname);
		return IllegalFile;
	}
	push(scope_arr, add_rooted(new_file_scope(file)));
	clear_ref(fname);
	clear_ref(file);
	return Nothing;
}

Error run_file_in_env(Stack *S, Stack *scope_arr)
{
	require(2);
	V env = popS();
	V fname = popS();
	if (getType(fname) != T_STR)
	{
		clear_ref(env);
		clear_ref(fname);
		return TypeError;
	}
	V file = load_file(fname, toFile(toScope(get_head(scope_arr))->file)->global);
	if (file == NULL)
	{
		clear_ref(env);
		clear_ref(fname);
		return IllegalFile;
	}
	push(scope_arr, add_rooted(new_file_scope_env(file, env)));
	clear_ref(fname);
	return Nothing;
}

Error run_blob(Stack *S, Stack *scope_arr)
{
	require(1);
	V blob = popS();
	if (getType(blob) != T_BLOB)
	{
		clear_ref(blob);
		return TypeError;
	}
	V file = load_memfile((char*)toBlob(blob)->data, toBlob(blob)->size, a_to_string("(blob)"), toFile(toScope(get_head(scope_arr))->file)->global);
	if (file == NULL)
	{
		clear_ref(blob);
		return IllegalFile;
	}
	push(scope_arr, add_rooted(new_file_scope(file)));
	clear_ref(blob);
	clear_ref(file);
	return Nothing;
}

Error run_blob_in_env(Stack *S, Stack *scope_arr)
{
	require(2);
	V env = popS();
	V blob = popS();
	if (getType(blob) != T_BLOB || getType(env) != T_DICT)
	{
		clear_ref(env);
		clear_ref(blob);
		return TypeError;
	}
	V file = load_memfile((char*)toBlob(blob)->data, toBlob(blob)->size, a_to_string("(blob)"), toFile(toScope(get_head(scope_arr))->file)->global);
	if (file == NULL)
	{
		clear_ref(env);
		clear_ref(blob);
		return IllegalFile;
	}
	push(scope_arr, add_rooted(new_file_scope_env(file, env)));
	clear_ref(blob);
	return Nothing;
}

Error find_file_(Stack *S, Stack *scope_arr)
{
	require(1);
	V file_name = popS();
	if (getType(file_name) != T_STR)
	{
		clear_ref(file_name);
		return TypeError;
	}
	NewString *fn = toNewString(file_name);
	int l = fn->size;
	char *f_name = fn->text;
	struct stat deja_file;
	struct stat vu_file;
	int deja_exists;
	int vu_exists;
	if (l > 5 && !memcmp(f_name + l - 5, ".deja", 5))
	{
		// .deja
		// check if we need to invoke compiler
		deja_exists = stat(f_name, &deja_file) == 0;

		utf8 text;
		V compiled_name = empty_string_to_value(l - 2, &text);
		memcpy(text, f_name, l - 4);
		text[l - 4] = 'v';
		text[l - 3] = 'u';

		vu_exists = stat(text, &vu_file) == 0;

		if (!deja_exists && !vu_exists)
		{
			clear_ref(file_name);
			set_error_msg_ref(malloc(21 + l));
			sprintf(error_msg, "could not find file %*s", l, f_name);
			return IllegalFile;
		}

		pushS(compiled_name);

		if (!vu_exists || difftime(deja_file.st_mtime, vu_file.st_mtime) > 0)
		{
			pushS(file_name);
		}
		else
		{
			pushS(add_ref(v_false));
		}
	}
	else
	{
		// .vu
		// never invoke compiler
		vu_exists = stat(f_name, &vu_file) == 0;
		if (!vu_exists)
		{
			clear_ref(file_name);
			set_error_msg_ref(malloc(21 + l));
			sprintf(error_msg, "could not find file %*s", l, f_name);
			return IllegalFile;
		}
		pushS(file_name);
		pushS(add_ref(v_false));
	}
	return Nothing;
}

int init_argv(int argc, char** argv, V v_eva)
{
	V args = new_list();
	V kwargs = new_dict();
	int argi;
	for (argi = 0; argi < argc; argi++)
	{
		char *arg = argv[argi];
		int len = strlen(arg);
		if (!valid_utf8(len, arg))
		{
			handle_error(UnicodeError, NULL);
			return 1;
		}
		if (arg[0] == '-')
		{
			if (len == 1) // -
			{
				goto add_arg;
			}
			if (arg[1] == '-')
			{
				if (len == 2) // --
				{
					goto add_arg;
				}
				char *eq;
				if ((eq = strchr(arg, '='))) // --arg=val
				{
					*eq = '\0';
					V key = get_ident(arg + 2);
					V value = a_to_string(eq + 1);
					set_hashmap(toHashMap(kwargs), key, value);
				}
				else // --arg
				{
					V key = get_ident(arg + 2);
					V current = get_hashmap(toHashMap(kwargs), key);
					if (!current)
					{
						current = intToV(0);
					}
					if (isInt(current))
					{
						set_hashmap(toHashMap(kwargs), key, intToV(toInt(current) + 1));
					}
				}
			}
			else // -arg
			{
				char key[] = {'\0', '\0'};
				int i;
				for (i = 1; i < len; i++)
				{
					key[0] = arg[i];
					V keyv = get_ident(key);
					V current = get_hashmap(toHashMap(kwargs), keyv);
					if (!current)
					{
						current = intToV(0);
					}
					if (isInt(current))
					{
						set_hashmap(toHashMap(kwargs), keyv, intToV(toInt(current) + 1));
					}
				}
			}
		}
		else //arg
		{
			add_arg:
			push(toStack(args), a_to_string(arg));
		}
	}
	set_hashmap(toHashMap(v_eva), get_ident("args"), args);
	set_hashmap(toHashMap(v_eva), get_ident("opts"), kwargs);
	return 0;
}

CFunc eva[] = {
	{"encode", encode},
	{"decode", decode},
	{"read", read_file},
	{"open", open_file},
	{"close", close_file},
	{"write-fragment", write_fragment},
	{"find-module", find_module},
	{"find-file", find_file_},
	{"run-file", run_file},
	{"run-file-in", run_file_in_env},
	{"read-line", read_line},
	{"run-blob", run_blob},
	{"run-blob-in", run_blob_in_env},
	{"(print-stack)", print_stack},
	{"prompt", read_prompt},
	{NULL, NULL}
};
