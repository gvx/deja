#ifndef HEADER_DEF
#define HEADER_DEF

#define MAGIC "\aDV"
#define VERSION '\x00'

#include <netinet/in.h>
#include <stdbool.h>

typedef struct
{
	char magic[3];
	char version;
	unsigned int size;
} Header;

Header read_header(FILE *f)
{
	Header header;
	fread(&header, sizeof(header), 1, f);
	header.size = ntohl(header.size);
	return header;
}

bool header_correct(Header* h)
{
	int i;
	for (i = 0; i < 3; i++)
	{
		if (MAGIC[i] != header.magic[i])
		{
			printf("!!");
		}
	}
	if (VERSION != header.version)
	{
		printf("!!");
	}
}

#endif