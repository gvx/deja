#ifndef BLOB_DEF
#define BLOB_DEF

#include "value.h"
#include "lib.h"

typedef int blobsize_t;

typedef struct
{
	blobsize_t size;
	unsigned char *data;
} Blob;

V new_blob(int);
int getbyte_blob(V, blobsize_t);
int setbyte_blob(V, blobsize_t, unsigned char);
void resize_blob(V, blobsize_t);
V clone_blob(V);
int blit_blob(V, V, blobsize_t);

Error make_blob(Stack*, Stack*);
Error getbyte_blob_(Stack*, Stack*);
Error setbyte_blob_(Stack*, Stack*);
Error resize_blob_(Stack*, Stack*);
Error clone_blob_(Stack*, Stack*);
Error blit_blob_(Stack*, Stack*);

#endif
