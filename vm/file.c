#include "file.h"

V load_file(V file_name, V global)
{
	if (file_name == NULL)
	{
		return NULL;
	}
	if (getType(file_name) != T_STR)
	{
		return file_name;
	}
	V new_file = NULL;
	FILE* f = fopen(getChars(file_name), "rb");
	Header h = read_header(f);
	if (header_correct(&h))
	{
		read_literals(f, &h);
		new_file = make_new_value(T_FILE, false, sizeof(File));
		File* f_obj = toFile(new_file);
		f_obj->name = add_ref(file_name);
		f_obj->source = NULL;
		f_obj->header = h;
		f_obj->global = global;
		uint32_t *code = malloc(h.size * 4);
		fread(code, 4, h.size, f);
		f_obj->code = code;
	}
	else
		error_msg = "Not a valid Déjà Vu bytecode file.";
	fclose(f);
	clear_ref(file_name);
	return new_file;
}
