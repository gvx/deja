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

Error do_instruction(Header* h, Stack* S, Stack* scope_arr)
{
	V v;
	V key;
	uint32_t *pc;
	V scope = scope_arr->head->data;
	Scope *sc = toScope(scope);
	uint32_t* source = toScope(scope)->pc;
	Func* f;
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
			key = get_literal(h, argument);
			v = get_hashmap(&sc->hm, key);
			while (v == NULL)
			{
				sc = toScope(sc->parent);
				if (sc == NULL)
				{
					//some error?
					//*error = name error
					return NameError;
				}
				v = get_hashmap(&sc->hm, key);
			}
			if (v->type == T_FUNC)
			{
				//push v to the call stack
				push(scope_arr, new_function_scope(v));
				//return toFunc(v)->start;
				return Nothing;
			}
			else if (v->type == T_CFUNC)
			{
				return toCFunc(v)(h, S, scope_arr);
			}
			else
			{
				push(S, add_ref(v));
			}
			break;
		case OP_SET:
			v = pop(S);
			sc = toScope(scope);
			V key = get_literal(h, argument);
			while (!change_hashmap(&sc->hm, key, v))
			{
				if (sc->parent == NULL)
				{
					//set in the global environment
					set_hashmap(&sc->hm, key, v);
					break;
				}
				else
				{
					sc = toScope(sc->parent);
				}
			}
			clear_ref(v);
			break;
		case OP_SET_LOCAL:
			v = pop(S);
			set_hashmap(&(toScope(scope)->hm), get_literal(h, argument), v);
			clear_ref(v);
			break;
		case OP_SET_GLOBAL:
			v = pop(S);
			set_hashmap(&toScope(toFile(toScope(scope)->file)->global)->hm, get_literal(h, argument), v);
			clear_ref(v);
			break;
		case OP_GET:
			sc = toScope(scope);
			key = get_literal(h, argument);
			v = get_hashmap(&sc->hm, key);
			while (v == NULL)
			{
				sc = toScope(sc->parent);
				if (sc == NULL)
				{
					//some error?
					//*error = name error
					return NameError;
				}
				v = get_hashmap(&sc->hm, key);
			}
			push(S, add_ref(v));
			break;
		case OP_GET_GLOBAL:
			v = get_hashmap(&toScope(toFile(toScope(scope)->file)->global)->hm, get_literal(h, argument));
			if (v == NULL)
			{
				return NameError;
			}
			push(S, add_ref(v));
			break;
		case OP_JMP:
			sc->pc += argument;
			//return source + argument;
			//return Nothing;
		case OP_JMPZ:
			v = pop(S);
			bool t = truthy(v);
			clear_ref(v);
			if (!t)
			{
				sc->pc += argument;
		//		return Nothing;
			}
			break;
		case OP_RETURN:
			v = sc->func;
			if (v == NULL)
			{
				return Exit;
			}
			f = toFunc(v);
			v = NULL;
			do
			{
				if (v != NULL)
				{
					clear_ref(v);
				}
				v = pop(scope_arr);
				if (v == NULL)
				{
					return Exit;
				}
			}
			while (toFunc(toScope(v)->func) == f);
			push(scope_arr, v);
			break;
		case OP_LABDA:
			v = new_value(T_FUNC);
			f = malloc(sizeof(Func));
			f->defscope = scope;
			f->start = source + 1;
			v->data.object = f;
			push(S, v);
			break;
		case OP_ENTER_SCOPE:
			push(scope_arr, new_scope(scope));
			break;
		case OP_LEAVE_SCOPE:
			pc = sc->pc;
			v = pop(scope_arr);
			sc = toScope(get_head(scope_arr));
			sc->pc = pc;
			clear_ref(v);
			break;
		case OP_NEW_LIST:
			push(S, newlist());
			break;
		case OP_DROP:
			clear_ref(pop(S));
			break;
		case OP_DUP:
			push(S, add_ref(get_head(S)));
			break;
		case OP_LINE_NUMBER:
			break;
		case OP_LINE_TEXT:
			break;
		case OP_SOURCE_FILE:
			break;
	}
	return Nothing;
}