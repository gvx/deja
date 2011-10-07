#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>

#include "opcodes.h"
#include "value.h"

int signed_opcode(int opcode)
{
	switch (opcode)
	{
		case OP_PUSH_INTEGER:
		case OP_JMP:
		case OP_JMPZ:
			return true;
		default:
			return false;
	}
}

void decode(int instruction, int *opcode, int *argument)
{
	*opcode = instruction >> 24;
	if (signed_opcode(*opcode))
	{
		*argument = instruction & 8388607;
		if (instruction & (1 << 23))
		{
			*argument = -*argument;
		}
	}
	else
	{
		*argument = instruction & 16777215;
	}
}

void do_instruction(int* source, V scope)
{
	int opcode, argument;
	decode(ntohl(*source), &opcode, &argument);
	switch (opcode)
	{
		case OP_PUSH_LITERAL:
			break;
		case OP_PUSH_INTEGER:
			break;
		case OP_PUSH_WORD:
			break;
		case OP_SET:
			break;
		case OP_SET_LOCAL:
			break;
		case OP_SET_GLOBAL:
			break;
		case OP_GET:
			break;
		case OP_GET_GLOBAL:
			break;
		case OP_JMP:
			break;
		case OP_JMPZ:
			break;
		case OP_RETURN:
			break;
		case OP_LABDA:
			break;
		case OP_ENTER_SCOPE:
			break;
		case OP_LEAVE_SCOPE:
			break;
		case OP_NEW_LIST:
			break;
		case OP_DROP:
			break;
		case OP_DUP:
			break;
		case OP_LINE_NUMBER:
			break;
		case OP_LINE_TEXT:
			break;
		case OP_SOURCE_FILE:
			break;
	}
}