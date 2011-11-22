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
#include "lib.h"

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
			*argument = -(~*argument & 8388607) - 1;
		}
	}
	else
	{
		*argument = instruction & 16777215;
	}
}

Error do_instruction(Header* h, Stack* S, Stack* scope_arr)
{
	V container;
	V v;
	V key;
	V scope = get_head(scope_arr);
	Scope *sc = toScope(scope);
	Func* f;
	V file;
	uint32_t *pc;
	int opcode, argument;
	decode(ntohl(*sc->pc), &opcode, &argument);
	switch (opcode)
	{
		case OP_PUSH_LITERAL:
			push(S, add_ref(get_literal(h, argument)));
			break;
		case OP_PUSH_INTEGER:
			push(S, int_to_value(argument));
			break;
		case OP_PUSH_WORD:
			key = get_literal(h, argument);
			v = get_hashmap(&sc->hm, key);
			while (v == NULL)
			{
				if (sc->parent == NULL)
				{
					return NameError;
				}
				sc = toScope(sc->parent);
				v = get_hashmap(&sc->hm, key);
			}
			if (v->type == T_FUNC)
			{
				push(scope_arr, new_function_scope(v));
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
			set_hashmap(&sc->hm, get_literal(h, argument), v);
			clear_ref(v);
			break;
		case OP_SET_GLOBAL:
			v = pop(S);
			set_hashmap(&toScope(toFile(sc->file)->global)->hm, get_literal(h, argument), v);
			clear_ref(v);
			break;
		case OP_GET:
			key = get_literal(h, argument);
			v = get_hashmap(&sc->hm, key);
			while (v == NULL)
			{
				if (sc->parent == NULL)
				{
					return NameError;
				}
				sc = toScope(sc->parent);
				v = get_hashmap(&sc->hm, key);
			}
			push(S, add_ref(v));
			break;
		case OP_GET_GLOBAL:
			v = get_hashmap(&toScope(toFile(sc->file)->global)->hm, get_literal(h, argument));
			if (v == NULL)
			{
				return NameError;
			}
			push(S, add_ref(v));
			break;
		case OP_JMP:
			sc->pc += argument - 1;
			break;
		case OP_JMPZ:
			v = pop(S);
			bool t = truthy(v);
			clear_ref(v);
			if (!t)
			{
				sc->pc += argument - 1;
			}
			break;
		case OP_RETURN:
			v = NULL;
			file = sc->file;
			do
			{
				clear_ref(v);
				v = pop(scope_arr);
				if (v == NULL)
				{
					return Exit;
				}
			}
			while (!toScope(v)->is_func_scope && toScope(v)->file == file);
			clear_ref(v);
			if (stack_size(scope_arr) == 0)
			{
				return Exit;
			}
			break;
		case OP_RECURSE:
			v = NULL;
			file = sc->file;
			do
			{
				clear_ref(v);
				v = pop(scope_arr);
				if (v == NULL)
				{
					return Exit;
				}
			}
			while (!toScope(v)->is_func_scope && toScope(v)->file == file);
			push(scope_arr, v);
			sc = toScope(v);
			sc->pc = toFunc(sc->func)->start;
			break;
		case OP_LABDA:
			v = new_value(T_FUNC);
			f = malloc(sizeof(Func));
			f->defscope = scope;
			f->start = sc->pc;
			v->data.object = f;
			push(S, v);
			sc->pc += argument - 1;
			break;
		case OP_ENTER_SCOPE:
			push(scope_arr, new_scope(scope));
			break;
		case OP_LEAVE_SCOPE:
			pc = sc->pc;
			clear_ref(pop(scope_arr));
			sc = toScope(get_head(scope_arr));
			sc->pc = pc;
			break;
		case OP_NEW_LIST:
			push(S, new_list());
			break;
		case OP_POP_FROM:
			container = pop(S);
			if (container == NULL)
			{
				return StackEmpty;
			}
			if (container->type != T_STACK)
			{
				clear_ref(container);
				return TypeError;
			}
			v = pop(toStack(container));
			if (v == NULL)
			{
				return StackEmpty;
			}
			push(S, v);
			clear_ref(container);
			break;
		case OP_PUSH_TO:
			if (stack_size(S) < 2)
			{
				return StackEmpty;
			}
			container = pop(S);
			if (container->type != T_STACK)
			{
				clear_ref(container);
				return TypeError;
			}
			push(toStack(container), pop(S));
			clear_ref(container);
			break;
		case OP_PUSH_THROUGH:
			if (stack_size(S) < 2)
			{
				return StackEmpty;
			}
			container = pop(S);
			if (container->type != T_STACK)
			{
				clear_ref(container);
				return TypeError;
			}
			push(toStack(container), pop(S));
			push(S, container);
			break;
		case OP_DROP:
			clear_ref(pop(S));
			break;
		case OP_DUP:
			push(S, add_ref(get_head(S)));
			break;
		case OP_SWAP:
			if (stack_size(S) < 2)
			{
				return StackEmpty;
			}
			V v1 = pop(S);
			V v2 = pop(S);
			push(S, v1);
			push(S, v2);
			break;
		case OP_ROT:
			if (stack_size(S) < 3)
			{
				return StackEmpty;
			}
			Node *a = S->head;
			Node *b = a->next;
			Node *c = b->next;
			a->next = c->next;
			c->next = a;
			S->head = b;
			break;
		case OP_OVER:
			if (stack_size(S) < 2)
			{
				return StackEmpty;
			}
			push(S, add_ref(S->head->next->data));
			break;
		case OP_LINE_NUMBER:
			sc->linenr = argument;
			break;
		case OP_SOURCE_FILE:
			toFile(sc->file)->source = get_literal(h, argument);
			//don't bother with refcounting: literals exist
			//exactly as long as the file they belong to.
			break;
		case OP_ENTER_ERRHAND:
			v = new_scope(scope);
			push(scope_arr, v);
			sc = toScope(v);
			sc->is_error_handler = true;
			sc->pc += argument - 1;
			break;
		case OP_LEAVE_ERRHAND:
			pc = sc->pc;
			clear_ref(pop(scope_arr));
			sc = toScope(get_head(scope_arr));
			sc->pc = pc;
			break;
		case OP_NEW_DICT:
			push(S, new_dict());
			break;
		case OP_HAS_DICT:
			container = pop(S);
			if (container == NULL)
			{
				return StackEmpty;
			}
			if (container->type != T_DICT)
			{
				return TypeError;
			}
			v = get_hashmap(toHashMap(container), get_literal(h, argument));
			push(S, v == NULL ? v_true : v_false);
			clear_ref(container);
			break;
		case OP_GET_DICT:
			container = pop(S);
			if (container == NULL)
			{
				return StackEmpty;
			}
			if (container->type != T_DICT)
			{
				return TypeError;
			}
			v = get_hashmap(toHashMap(container), get_literal(h, argument));
			if (v == NULL)
			{
				return ValueError;
			}
			push(S, v);
			clear_ref(container);
			break;
		case OP_SET_DICT:
			container = pop(S);
			if (container == NULL)
			{
				return StackEmpty;
			}
			if (container->type != T_DICT)
			{
				return TypeError;
			}
			v = pop(S);
			if (v == NULL)
			{
				clear_ref(container);
				return StackEmpty;
			}
			set_hashmap(toHashMap(container), get_literal(h, argument), v);
			clear_ref(v);
			clear_ref(container);
			break;
	}
	return Nothing;
}