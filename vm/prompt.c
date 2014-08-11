//THINGS MISSING
// * history
// * tab-completion

#include <termios.h>
#include <string.h>
#include <unistd.h>

#include "prompt.h"

#define next_char(index) while ((out[++index] & 0xc0) == 0x80)
#define prev_char(index) while ((out[--index] & 0xc0) == 0x80)

void write_assert(int fd, const void *buf, size_t count)
{
	ssize_t a = write(fd, buf, count);
	if (a < 0)
	{
		// something went wrong
	}
}

void read_assert(int fd, void *buf, size_t count)
{
	ssize_t a = read(fd, buf, count);
	if (a < 0)
	{
		// something went wrong
	}
}

int get_column()
{
	// SEQ: find out current cursor position
	write_assert(1, "\e[6n", 4);
	unsigned char tmp;
	int column = 0;
	do
	{
		read_assert(0, &tmp, 1);
	}
	while (tmp != ';');
	read_assert(0, &tmp, 1);
	while (tmp != 'R')
	{
		column = column * 10 + tmp - '0';
		read_assert(0, &tmp, 1);
	}
	return column;
}

void move_left(int n)
{
	int column = get_column();
	while (n--)
	{
		if (column > 1)
		{
			// SEQ: go 1 char to the left
			write_assert(1, "\e[1D", 4);
			column--;
		}
		else
		{
			// SEQ: go 1 char up and to the far right
			write_assert(1, "\e[1A\e[999C", 10);
			column = get_column();
		}
	}
}

void move_right(int n)
{
	int i;
	int column, old_column = get_column();
	startover:
	i = n;
	while (i--)
	{
		// SEQ: go 1 char to the right
		write_assert(1, "\e[1C", 4);
	}
	column = get_column();
	if (column - old_column < n)
	{
		// SEQ: go to start of next line
		write_assert(1, "\e[1E", 4);
		n = n - column + old_column - 1;
		old_column = 1;
		goto startover;
	}
}

enum prompt_result prompt(const char *in, prompt_t out, prompt_t history[])
{
	unsigned char tmp;
	int index, maxindex, indentation, nleft, movei;
	enum prompt_result prompt_result;
	struct termios original, raw;

	write_assert(1, in, strlen(in));
	tcgetattr(0, &original);
	raw = original;
	cfmakeraw(&raw);
	tcsetattr(0, TCSADRAIN, &raw);

	index = 0;
	maxindex = 0;
	indentation = 0;

	readnextchar:
	read_assert(0, &tmp, 1);
	switch (tmp)
	{
		case '\x01': //ctrl A
			goto homekey;
		case '\x03': //ctrl C
			write_assert(1, "^C", 2);
			prompt_result = prompt_result_interrupt;
			goto endloop;
		case '\x04': //ctrl D
			if (maxindex == 0 && indentation == 0)
			{
				prompt_result = prompt_result_eof;
				goto endloop;
			}
			else
			{
				goto delete;
			}
		case '\x05': //ctrl E
			goto endkey;
		case '\x08': //ctrl H
		case '\x7f': //backspace
			if (index > 0)
			{
				int oldindex = index;
				prev_char(index);
				maxindex -= oldindex - index;
				memmove(out + index, out + oldindex, maxindex - index);
				move_left(1);
				// SEQ: save position
				write_assert(1, "\e[s", 3);
				write_assert(1, out + index, maxindex - index);
				// SEQ: clear line to the right, restore position
				write_assert(1, "\e[K\e[u", 6);
			}
			else if (indentation > 0)
			{
				//dedent
				indentation--;
				// SEQ: go 4 chars to the left, save position
				write_assert(1, "\e[4D\e[s", 7);
				write_assert(1, out, maxindex);
				// SEQ: clear line to the right, restore postion
				write_assert(1, "\e[K\e[u", 6);
			}
			goto readnextchar;
		case '\x09': //tab, ctrl I
			if (index > 0)
			{
				//autocomplete
			}
			else if (maxindex + indentation + 1 < MAX_PROMPT)
			{
				//indent
				indentation++;
				// SEQ: clear line to the right, go four chars to the right, save position
				write_assert(1, "\e[K\e[4C\e[s", 10);
				write_assert(1, out, maxindex);
				// SEQ: restore position
				write_assert(1, "\e[u", 3);
			}
			goto readnextchar;
		case '\x0b': //ctrl K
			maxindex = index;
			// SEQ: clear line to the right
			write_assert(1, "\e[K", 3);
			goto readnextchar;
		case '\x0d': //enter, ctrl M
			write_assert(1, &tmp, 1);
			tmp = '\n';
			write_assert(1, &tmp, 1);
			prompt_result = prompt_result_normal;
			goto endloop;
		case '\x15': //ctrl U
			if (index > 0)
			{
				int oldindex = index;
				movei = 0;
				while (index > 0)
				{
					prev_char(index);
					movei++;
				}
				move_left(movei);
				maxindex -= oldindex;
				memmove(out, out + oldindex, maxindex);
				// SEQ: save position
				write_assert(1, "\e[s", 3);
				write_assert(1, out, maxindex);
				// SEQ: clear line to the right, restore position
				write_assert(1, "\e[K\e[u", 6);
			}
			goto readnextchar;
		case '\x17': //ctrl W
			if (index > 0)
			{
				int oldindex = index;
				movei = 0;
				while (index > 0 && out[index - 1] != ' ')
				{
					prev_char(index);
					movei++;
				}
				if (index > 0)
				{
					prev_char(index);
					movei++;
				}
				move_left(movei);
				maxindex -= oldindex - index;
				memmove(out + index, out + oldindex, maxindex - index);
				// SEQ: save position
				write_assert(1, "\e[s", 3);
				write_assert(1, out + index, maxindex - index);
				// SEQ: clear line to the right, restore position
				write_assert(1, "\e[K\e[u", 6);
			}
			goto readnextchar;
		//ignored keys
		case '\x02': //ctrl B
		case '\x06': //ctrl F
		case '\x07': //ctrl G
		case '\x0a': //ctrl J
		case '\x0c': //ctrl L
		case '\x0e': //ctrl N
		case '\x0f': //ctrl O
		case '\x10': //ctrl P
		case '\x11': //ctrl Q
		case '\x12': //ctrl R
		case '\x13': //ctrl S
		case '\x14': //ctrl T
		case '\x16': //ctrl V
		case '\x18': //ctrl X
		case '\x19': //ctrl Y
		case '\x1a': //ctrl Z
			goto readnextchar;
		//escaped combinations
		case '\e':
			read_assert(0, &tmp, 1);
			switch (tmp)
			{
				case '[': //csi
					read_assert(0, &tmp, 1);
					switch (tmp)
					{
						case 'A': //up
							//prev history
							goto readnextchar;
						case 'B': //down
							//next history
							goto readnextchar;
						case 'C': //right
							if (index < maxindex)
							{
								next_char(index);
								move_right(1);
							}
							goto readnextchar;
						case 'D': //left
							if (index > 0)
							{
								prev_char(index);
								move_left(1);
							}
							goto readnextchar;
						case '1':
							read_assert(0, &tmp, 1);
							if (tmp == '~') // home
							{
								goto homekey;
							}
							read_assert(0, &tmp, 1);
							read_assert(0, &tmp, 1);
							switch (tmp)
							{
								case 'A': //ctrl up
								case 'B': //ctrl down
									goto readnextchar;
								case 'C': //ctrl right
									movei = 0;
									while (index < maxindex && out[index] != ' ')
									{
										next_char(index);
										movei++;
									}
									if (index < maxindex)
									{
										next_char(index);
										movei++;
									}
									move_right(movei);
									goto readnextchar;
								case 'D': //ctrl left
									movei = 0;
									while (index > 0 && out[index - 1] != ' ')
									{
										prev_char(index);
										movei++;
									}
									if (index > 0)
									{
										prev_char(index);
										movei++;
									}
									move_left(movei);
									goto readnextchar;
							}
							goto readnextchar;
						case '3': // delete
							//read out count and discard it
							do
							{
								read_assert(0, &tmp, 1);
							}
							while (tmp != '~');
							delete:
							if (index < maxindex)
							{
								int oldindex = index;
								next_char(oldindex);
								maxindex -= oldindex - index;
								memmove(out + index, out + oldindex, maxindex - index);
								// SEQ: save position
								write_assert(1, "\e[s", 3);
								write_assert(1, out + index, maxindex - index);
								// SEQ: clear line to the right, restore position
								write_assert(1, "\e[K\e[u", 6);
							}
							goto readnextchar;
						case '4': // end
							read_assert(0, &tmp, 1);
							goto endkey;
						case '5': // page up
						case '6': // page down
							//read out count and discard it
							do
							{
								read_assert(0, &tmp, 1);
							}
							while (tmp != '~');
							goto readnextchar;
						default: // unknown CSI combo
							// ignore
							goto readnextchar;
					}
					goto readnextchar;
				case 'O':
					read_assert(0, &tmp, 1);
					switch (tmp)
					{
						case 'H': //home
							homekey:
							movei = 0;
							while (index > 0)
							{
								prev_char(index);
								movei++;
							}
							move_left(movei);
							goto readnextchar;
						case 'F': //end
							endkey:
							movei = 0;
							while (index < maxindex)
							{
								next_char(index);
								movei++;
							}
							move_right(movei);
							goto readnextchar;
					}
					goto readnextchar;
				default: // unknown escape sequence
					// ignore
					goto readnextchar;
			}
		default: //actual characters
			if ((tmp & 0xc0) == 0xc0) //wow utf-8, such beauty, much elegant
			{
				nleft = 1;
				unsigned char cmpr = 0x20;
				while (tmp & cmpr)
				{
					nleft++;
					cmpr >>= 1;
				}
			}
			else
			{
				nleft = 0;
			}
			normalbyte:
			if (maxindex + indentation + 1 >= MAX_PROMPT)
			{
				goto readnextchar;
			}
			if (index < maxindex)
			{
				memmove(out + index + 1, out + index, maxindex - index);
			}
			out[index] = tmp;
			write_assert(1, &tmp, 1);
			maxindex++;
			index++;
			if (nleft-- > 0)
			{
				read_assert(0, &tmp, 1);
				goto normalbyte;
			}
			if (index < maxindex)
			{
				// SEQ: save position
				write_assert(1, "\e[s", 3);
				write_assert(1, out + index, maxindex - index);
				// SEQ: restore position
				write_assert(1, "\e[u", 3);
			}
			tcflush(0, TCOFLUSH);
			goto readnextchar;
	}
	endloop:
	tcsetattr(0, TCSADRAIN, &original);
	memmove(out + indentation, out, maxindex);
	memset(out, 9, indentation);
	out[maxindex + indentation] = '\0';
	return prompt_result;
}
