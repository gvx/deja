#include "strings.h"

unichar decode_codepoint(utf8 source, utf8index *ix)
{
	char a, b, c, d, e, f;

	a = source[*ix++];
	if (!(a >> 7))
		return a;
		// 0aaaaaaa

	b = source[*ix++] & 63;
	if (!(a & 32))
		return (a & 31) << 6 | b;
		// 110aaaaa 10bbbbbb

	c = source[*ix++] & 63;
	if (!(a & 16))
		return (a & 15) << 12 | b << 6 | c;
		// 1110aaaa 10bbbbbb 10cccccc

	d = source[*ix++] & 63;
	if (!(a & 8))
		return (a & 7) << 18 | b << 12 | c << 6 | d;
		// 11110aaa 10bbbbbb 10cccccc 10dddddd

	e = source[*ix++] & 63;
	if (!(a & 4))
		return (a & 3) << 24 | b << 18 | c << 12 | d << 6 | e;
		// 111110aa 10bbbbbb 10cccccc 10dddddd 10eeeeee

	f = source[*ix++] & 63;
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

bool valid_utf8(utf8 source)
{
	utf8index index = 0;
	unichar cp = 1;
	while (cp)
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