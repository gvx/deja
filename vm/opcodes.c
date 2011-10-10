#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>

#include "opcodes.h"
#include "value.h"
#include "types.h"
#include "stack.h"
#include "literals.h"
#include "scope.h"
#include "gc.h"
#include "func.h"
#include "hashmap.h"
#include "header.h"

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

int* do_instruction(Header* h, int* source, Stack* S, Stack* scope_arr)
{
	V v;
	V key;
	Scope *sc;
	V scope = scope_arr->head->data;
	int opcode, argument;
	decode(ntohl(*source), &opcode, &argument);
	switch (opcode)
	{
		case OP_PUSH_LITERAL:
			push(S, get_literal(h, argument));
			break;
		case OP_PUSH_INTEGER:
			push(S, int_to_value(argument));
			break;
		case OP_PUSH_WORD:
			sc = (Scope*)scope->data.object;
			key = get_literal(h, argument);
			v = get_hashmap(&sc->hm, key);
			while (v == NULL)
			{
				sc = (Scope*)sc->parent->data.object;
				if (sc == NULL)
				{
					//some error?
					//*error = name error
					return source;
				}
				v = get_hashmap(&sc->hm, key);
			}
			if (v->type == T_FUNC)
			{
				//push v to the call stack
				push(scope_arr, new_function_scope(v));
				return ((Func*)v->data.object)->start;
			}
			else
			{
				push(S, add_ref(v));
			}
			break;
		case OP_SET:
			v = pop(S);
			sc = (Scope*)scope->data.object;
			V key = get_literal(h, argument);
			while (!change_hashmap(sc->hm, key, v))
			{
				sc = (Scope*)sc->parent->data.object;
				if (sc == NULL)
				{
					//set in the global environment
					break;
				}
			}
			clear_ref(v);
			break;
		case OP_SET_LOCAL:
			v = pop(S);
			set_hashmap(&((Scope*)scope->data.object)->hm, get_literal(h, argument), v);
			clear_ref(v);
			break;
		case OP_SET_GLOBAL:
			break;
		case OP_GET:
			sc = (Scope*)scope->data.object;
			key = get_literal(h, argument);
			v = get_hashmap(&sc->hm, key);
			while (v == NULL)
			{
				sc = (Scope*)sc->parent->data.object;
				if (sc == NULL)
				{
					//some error?
					//*error = name error
					return source;
				}
				v = get_hashmap(&sc->hm, key);
			}
			push(S, add_ref(v));
			break;
		case OP_GET_GLOBAL:
			break;
		case OP_JMP:
			return source + argument;
		case OP_JMPZ:
			v = pop(S);
			bool t = truthy(v);
			clear_ref(v);
			if (!t)
			{
				return source + argument;
			}
			break;
		case OP_RETURN:
			break;
		case OP_LABDA:
			v = new_value(T_FUNC);
			Func* f = malloc(sizeof(Func));
			f->defscope = scope;
			f->start = source + 1;
			v->data.object = f;
			push(S, v);
			break;
		case OP_ENTER_SCOPE:
			push(scope_arr, new_scope(scope));
			break;
		case OP_LEAVE_SCOPE:
			clear_ref(pop(scope_arr));
			break;
		case OP_NEW_LIST:
			push(S, newlist());
			break;
		case OP_DROP:
			clear_ref(pop(S));
			break;
		case OP_DUP:
			push(S, add_ref(S->head->data));
			break;
		case OP_LINE_NUMBER:
			break;
		case OP_LINE_TEXT:
			break;
		case OP_SOURCE_FILE:
			break;
	}
	return source + 1;
}