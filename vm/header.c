#include "header.h"

Header read_header(char *data, size_t size)
{
	Header header = {{0}};
	if (size < sizeof header) return header;
	memcpy(&header, data, sizeof header);
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
