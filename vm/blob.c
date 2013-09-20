#include "blob.h"
#include <stdlib.h>

V new_blob(int size)
{
	V t = make_new_value(T_BLOB, true, sizeof(Blob));
	toBlob(t)->size = size;
	toBlob(t)->data = calloc(size, 1); //TODO: check return value
	return t;
}

int getbyte_blob(V blob, blobsize_t index)
{
	if (index <= 0 || index > toBlob(blob)->size)
	{
		return -1; // :(
	}
	return toBlob(blob)->data[index - 1];
}

int setbyte_blob(V blob, blobsize_t index, unsigned char value)
{
	if (index <= 0 || index > toBlob(blob)->size)
	{
		return -1; // :(
	}
	toBlob(blob)->data[index - 1] = value;
	return 0;
}

void resize_blob(V blob, blobsize_t size)
{
	Blob *b = toBlob(blob);
	b->data = realloc(b->data, size);
	if (size > b->size)
	{
		memset(b->data + b->size, 0, size - b->size);
	}
	b->size = size;
}

V clone_blob(V blob)
{
	V t = make_new_value(T_BLOB, true, sizeof(Blob));
	toBlob(t)->size = toBlob(blob)->size;
	toBlob(t)->data = malloc(toBlob(blob)->size); //TODO: check return value
	memcpy(toBlob(t)->data, toBlob(blob)->data, toBlob(blob)->size);
	return t;
}

int blit_blob(V dest, V src, blobsize_t offset)
{
	if (offset < 0 || toBlob(dest)->size < offset + toBlob(src)->size)
	{
		return -1;
	}
	memcpy(toBlob(dest)->data + offset, toBlob(src)->data, toBlob(src)->size);
	return 0;
}

Error make_blob(Stack *S, Stack *scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	pushS(new_blob(toNumber(v)));
	clear_ref(v);
	return Nothing;
}

Error getbyte_blob_(Stack *S, Stack *scope_arr)
{
	require(2);
	V blob = popS();
	V index = popS();
	Error e = Nothing;
	if (getType(blob) != T_BLOB || getType(index) != T_NUM)
	{
		e = TypeError;
		goto cleanup;
	}
	int byte = getbyte_blob(blob, toNumber(index));
	if (byte < 0)
	{
		error_msg = "Index out of range";
		e = ValueError;
		goto cleanup;
	}
	pushS(int_to_value(byte));
	cleanup:
	clear_ref(blob);
	clear_ref(index);
	return e;
}

Error setbyte_blob_(Stack *S, Stack *scope_arr)
{
	require(3);
	V blob = popS();
	V index = popS();
	V value = popS();
	Error e = Nothing;
	if (getType(blob) != T_BLOB || getType(index) != T_NUM || getType(value) != T_NUM)
	{
		e = TypeError;
		goto cleanup;
	}
	int num = toNumber(value);
	if (num < 0 || num > 255)
	{
		error_msg = "Value not in range [0,255]";
		e = ValueError;
		goto cleanup;
	}
	int byte = setbyte_blob(blob, toNumber(index), num);
	if (byte < 0)
	{
		error_msg = "Index out of range";
		e = ValueError;
		goto cleanup;
	}
	cleanup:
	clear_ref(blob);
	clear_ref(index);
	clear_ref(value);
	return e;
}

Error resize_blob_(Stack *S, Stack *scope_arr)
{
	require(2);
	V blob = popS();
	V newsize = popS();
	Error e = Nothing;
	if (getType(blob) != T_BLOB || getType(newsize) != T_NUM)
	{
		e = TypeError;
		goto cleanup;
	}
	resize_blob(blob, toNumber(newsize));
	cleanup:
	clear_ref(blob);
	clear_ref(newsize);
	return e;
}

Error clone_blob_(Stack *S, Stack *scope_arr)
{
	require(1);
	V blob = popS();
	Error e = Nothing;
	if (getType(blob) != T_BLOB)
	{
		e = TypeError;
		goto cleanup;
	}
	pushS(clone_blob(blob));
	cleanup:
	clear_ref(blob);
	return e;
}

Error blit_blob_(Stack *S, Stack *scope_arr)
{
	require(3);
	V dest = popS();
	V src = popS();
	V offset = popS();
	Error e = Nothing;
	if (getType(dest) != T_BLOB || getType(src) != T_BLOB || getType(offset) != T_NUM)
	{
		e = TypeError;
		goto cleanup;
	}
	if (blit_blob(dest, src, toNumber(offset) - 1) < 0)
	{
		error_msg = "Index out of range";
		e = ValueError;
		goto cleanup;
	}
	cleanup:
	clear_ref(dest);
	clear_ref(src);
	clear_ref(offset);
	return e;
}
