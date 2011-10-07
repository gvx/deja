#ifndef HEADER_DEF
#define HEADER_DEF

#define MAGIC "\aDV"
#define VERSION '\x00'

typedef struct
{
	char magic[3];
	char version;
	unsigned int size;
} Header;

#endif