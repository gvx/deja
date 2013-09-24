#include <stdio.h>
#include <assert.h>

#include "eva.h"
#include "blob.h"
#include "strings.h"

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

Error encode(Stack *S, Stack *scope_arr)
{
	require(2);
	V encoding = popS();
	V cooked_data = popS();
	if (getType(encoding) != T_IDENT || getType(cooked_data) != T_STR)
	{
		clear_ref(encoding);
		clear_ref(cooked_data);
		return TypeError;
	}
	//just assume UTF-8 for now
	V output = new_blob(toNewString(cooked_data)->size);
	memcpy(toBlob(output)->data, toNewString(cooked_data)->text, toNewString(cooked_data)->size);
	pushS(output);
	clear_ref(encoding);
	clear_ref(cooked_data);
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
			resize_blob(output, extrasize + READ_BUFF_SIZE);
			extrasize += READ_BUFF_SIZE;
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
		error_msg = "Invalid open mode, use one of :read, :write, :append or :read-write";
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
	V handle = get_hashmap(toHashMap(file_obj), get_ident("handle"));
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
	if (getType(file_obj) != T_DICT || getType(blob) != T_BLOB)
	{
		clear_ref(file_obj);
		clear_ref(blob);
		return TypeError;
	}
	V handle = get_hashmap(toHashMap(file_obj), get_ident("handle"));
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

CFunc eva[] = {
	{"encode", encode},
	{"decode", decode},
	{"read", read_file},
	{"open", open_file},
	{"close", close_file},
	{"write-fragment", write_fragment},
	{NULL, NULL}
};
