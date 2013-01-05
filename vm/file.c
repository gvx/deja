#include "file.h"
#include <assert.h>

V load_file(V file_name, V global)
{
	if (file_name == NULL || getType(file_name) != T_STR)
	{
		clear_ref(file_name);
		return file_name;
	}
	FILE* f = fopen(getChars(file_name), "rb");
	if (f == NULL)
	{
		clear_ref(file_name);
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	size_t length = ftell(f);
	rewind(f);
	char *data = malloc(length * sizeof(char));
	size_t read = fread(data, sizeof(char), length, f);
	assert (read == length);
	V new_file = load_memfile(data, length, file_name, global);
	free(data);
	fclose(f);
	clear_ref(file_name);
	return new_file;
}

#define BUF_SIZE 1024

struct vec {
	int s;
	char *d;
};

struct vec read_all()
{
	char *t = malloc(BUF_SIZE);
	int len = fread(t, 1, BUF_SIZE, stdin);
	int buffer_size = BUF_SIZE;
	while (!feof(stdin))
	{
		buffer_size *= 2;
		t = realloc(t, buffer_size);
		len += fread(t + buffer_size - BUF_SIZE, 1, BUF_SIZE, stdin);
	}
	return (struct vec){len, t};
}


V load_stdin(V global)
{
	struct vec v = read_all();
	V file_name = a_to_value("(stdin)");
	V obj = load_memfile(v.d, v.s, file_name, global);
	free(v.d);
	clear_ref(file_name);
	return obj;
}

V load_memfile(char *data, size_t length, V file_name, V global)
{
	V new_file = NULL;
	Header h = read_header(data, length);
	data += 8;
	if (header_correct(&h))
	{
		if (!read_literals(data, length, &h))
		{
			return NULL;
		}
		new_file = make_new_value(T_FILE, false, sizeof(File));
		File* f_obj = toFile(new_file);
		f_obj->name = add_ref(file_name);
		f_obj->source = NULL;
		f_obj->header = h;
		f_obj->global = global;
		uint32_t *code = malloc(h.size * 4);
		memcpy(code, data, 4 * h.size);
		f_obj->code = code;
	}
	else
		error_msg = "not a valid Déjà Vu bytecode file";
	return new_file;
}
