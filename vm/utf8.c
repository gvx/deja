#include "strings.h"

unichar decode_codepoint(utf8 source, utf8index *ix)
{
	char a, b, c, d, e, f;

	a = source[(*ix)++];
	if (!(a >> 7))
		return a;
		// 0aaaaaaa

	b = source[(*ix)++] & 63;
	if (!(a & 32))
		return (a & 31) << 6 | b;
		// 110aaaaa 10bbbbbb

	c = source[(*ix)++] & 63;
	if (!(a & 16))
		return (a & 15) << 12 | b << 6 | c;
		// 1110aaaa 10bbbbbb 10cccccc

	d = source[(*ix)++] & 63;
	if (!(a & 8))
		return (a & 7) << 18 | b << 12 | c << 6 | d;
		// 11110aaa 10bbbbbb 10cccccc 10dddddd

	e = source[(*ix)++] & 63;
	if (!(a & 4))
		return (a & 3) << 24 | b << 18 | c << 12 | d << 6 | e;
		// 111110aa 10bbbbbb 10cccccc 10dddddd 10eeeeee

	f = source[(*ix)++] & 63;
	if (true)
		return (a & 1) << 30 | b << 24 | c << 18 | d << 12 | e << 6 | f;
		// 1111110a 10bbbbbb 10cccccc 10dddddd 10eeeeee 10ffffff
}

utf8byte encode_first_byte(unichar source)
{
	if (source <= 0x007F)
	{
		return source;
	}
	else if (source <= 0x07FF)
	{
		return (source >> 6) | 192;
	}
	else if (source <= 0xFFFF)
	{
		return (source >> 12) | 224;
	}
	else if (source <= 0x1FFFF)
	{
		return (source >> 18) | 240;
	}
	else if (source <= 0x3FFFF)
	{
		return (source >> 24) | 248;
	}
	else
	{
		return (source >> 30) | 252;
	}
}

unichar rest_of_codepoint(unichar source)
{
	if (source <= 0x007F)
	{
		return 0;
	}
	else if (source <= 0x07FF)
	{
		return source & 63;
	}
	else if (source <= 0xFFFF)
	{
		return source & 4095;
	}
	else if (source <= 0x1FFFF)
	{
		return source & 262143;
	}
	else if (source <= 0x3FFFF)
	{
		return source & 16777215;
	}
	else
	{
		return source & 1073741823;
	}
}


bool valid_utf8(size_t size, utf8 source)
{
	utf8index index = 0;
	unichar cp = 1;
	while (index < size)
	{
		// first check whether numbers are correct
		utf8byte c = source[index];
		if (c >> 7)
		{
			if (!(c & 64))
				return false;
			if ((source[index + 1] & 192) != 128)
				return false;
			if (c & 32)
			{
				if ((source[index + 2] & 192) != 128)
					return false;
				if (c & 16)
				{
					if ((source[index + 3] & 192) != 128)
						return false;
					if (c & 8)
					{
						if ((source[index + 4] & 192) != 128)
							return false;
						if (c & 4)
						{
							if ((source[index + 5] & 192) != 128)
								return false;
							if (c & 3)
								return false;
						}
					}
				}
			}
		}
		// then check if shortest encoding
		cp = decode_codepoint(source, &index);
		if (encode_first_byte(cp) != c)
			return false;
	}
	return true;
}

utf8index codepoint_length(unichar source)
{
	if (source <= 0x007F)
	{
		return 1;
	}
	else if (source <= 0x07FF)
	{
		return 2;
	}
	else if (source <= 0xFFFF)
	{
		return 3;
	}
	else if (source <= 0x1FFFF)
	{
		return 4;
	}
	else if (source <= 0x3FFFF)
	{
		return 5;
	}
	else
	{
		return 6;
	}
}

void encode_codepoint(unichar source, utf8 dest)
{
	*dest++ = encode_first_byte(source);
	utf8index t = codepoint_length(source) - 1;
	source = rest_of_codepoint(source);
	while (t--)
	{
		*dest++ = (source >> (t*6)) | 128;
		source = source & ((1 << (t*6)) - 1);
	}
}
