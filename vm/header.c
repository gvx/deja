#include "header.h"

Header read_header(FILE *f)
{
	Header header;
	fread(&header, 8, 1, f);
	header.size = ntohl(header.size);
	return header;
}

bool header_correct(Header* h)
{
	if (memcmp(&MAGIC, &h->magic, 3))
	{
		return false;
	}
	return (VERSION & '\xf0') == (h->version & '\xf0') && (VERSION & '\x0f') >= (h->version & '\x0f');
}
