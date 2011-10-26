#include "file.h"

V load_file(V file_name, V global)
{
	if (file_name == NULL)
	{
		return NULL;
	}
	V new_file = NULL;
	FILE* f = fopen(toString(file_name)->data, "rb");
	Header h = read_header(f);
	if (header_correct(&h))
	{
		read_literals(f, &h);
		new_file = new_value(T_FILE);
		File* f_obj = malloc(sizeof(File));
		f_obj->name = add_ref(file_name);
		f_obj->header = h;
		f_obj->global = global;
		uint32_t *code = malloc(h.size * 4);
		fread(code, 4, h.size, f);
		f_obj->code = code;
		new_file->data.object = f_obj;
	}
	fclose(f);
	return new_file;
}