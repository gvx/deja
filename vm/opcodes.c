#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>

#include "types.h"
#include "literals.h"
#include "lib.h"

extern V lastCall;

Error inline do_instruction(Header* h, Stack* S, Stack* scope_arr)
{
	V container;
	V v;
	V key;
	V scope = get_head(scope_arr);
	Scope *sc = toScope(scope);
	V file;
	bool t;
	uint32_t *pc;
	int argument;
	unsigned int instruction = ntohl(*sc->pc);
	int opcode = instruction >> 24;
	lastCall = NULL;
	switch (opcode)
	{
		case OP_PUSH_INTEGER:
		case OP_JMP:
		case OP_JMPZ:
		case OP_JMPEQ:
		case OP_JMPNE:
			argument = instruction & 8388607;
			if (instruction & (1 << 23))
			{
				argument = -(~argument & 8388607) - 1;
			}
			break;
		default:
			argument = instruction & 16777215;
	}
	switch (opcode)
	{
		case OP_PUSH_LITERAL:
			pushS(add_ref(get_literal(h, argument)));
			break;
		case OP_PUSH_INTEGER:
			pushS(int_to_value(argument));
			break;
		case OP_PUSH_WORD:
			lastCall = key = get_literal(h, argument);
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
			if (getType(v) == T_FUNC)
			{
				push(scope_arr, add_rooted(new_function_scope(v)));
			}
			else if (getType(v) == T_CFUNC)
			{
				return toCFunc(v)(S, scope_arr);
			}
			else
			{
				pushS(add_ref(v));
			}
			break;
		case OP_SET:
			if (stack_size(S) < 1)
			{
				return StackEmpty;
			}
			v = popS();
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
			if (stack_size(S) < 1)
			{
				return StackEmpty;
			}
			v = popS();
			set_hashmap(&sc->hm, get_literal(h, argument), v);
			clear_ref(v);
			break;
		case OP_SET_GLOBAL:
			if (stack_size(S) < 1)
			{
				return StackEmpty;
			}
			v = popS();
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
			pushS(add_ref(v));
			break;
		case OP_GET_GLOBAL:
			v = get_hashmap(&toScope(toFile(sc->file)->global)->hm, get_literal(h, argument));
			if (v == NULL)
			{
				return NameError;
			}
			pushS(add_ref(v));
			break;
		case OP_JMP:
			sc->pc += argument - 1;
			break;
		case OP_JMPZ:
			if (stack_size(S) < 1)
			{
				return StackEmpty;
			}
			v = popS();
			t = truthy(v);
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
				clear_base_ref(v);
				v = pop(scope_arr);
				if (v == NULL)
				{
					return Exit;
				}
			}
			while (!toScope(v)->is_func_scope && toScope(v)->file == file);
			clear_base_ref(v);
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
				clear_base_ref(v);
				v = pop(scope_arr);
				if (v == NULL)
				{
					return Exit;
				}
			}
			while (!toScope(v)->is_func_scope && toScope(v)->file == file);
			push(scope_arr, add_rooted(v));
			sc = toScope(v);
			sc->pc = toFunc(sc->func)->start;
			break;
		case OP_JMPEQ:
			if (stack_size(S) < 2)
			{
				return StackEmpty;
			}
			v = popS();
			key = popS(); //variable reuse
			t = equal(v, key);
			clear_ref(v);
			clear_ref(key);
			if (t)
			{
				sc->pc += argument - 1;
			}
			break;
		case OP_JMPNE:
			if (stack_size(S) < 2)
			{
				return StackEmpty;
			}
			v = popS();
			key = popS(); //variable reuse
			t = equal(v, key);
			clear_ref(v);
			clear_ref(key);
			if (!t)
			{
				sc->pc += argument - 1;
			}
			break;
		case OP_LABDA:
			pushS(new_func(scope, sc->pc));
			sc->pc += argument - 1;
			break;
		case OP_ENTER_SCOPE:
			push(scope_arr, add_rooted(new_scope(scope)));
			break;
		case OP_LEAVE_SCOPE:
		case OP_LEAVE_ERRHAND:
			pc = sc->pc;
			clear_base_ref(pop(scope_arr));
			sc = toScope(get_head(scope_arr));
			sc->pc = pc;
			break;
		case OP_NEW_LIST:
			pushS(new_list());
			break;
		case OP_POP_FROM:
			if (stack_size(S) < 1)
			{
				return StackEmpty;
			}
			container = popS();
			if (getType(container) != T_LIST)
			{
				clear_ref(container);
				return TypeError;
			}
			if (stack_size(toStack(container)) < 1)
			{
				clear_ref(container);
				return ValueError;
			}
			v = pop(toStack(container));
			pushS(v);
			clear_ref(container);
			break;
		case OP_PUSH_TO:
			if (stack_size(S) < 2)
			{
				return StackEmpty;
			}
			container = popS();
			if (getType(container) != T_LIST)
			{
				clear_ref(container);
				return TypeError;
			}
			push(toStack(container), popS());
			clear_ref(container);
			break;
		case OP_PUSH_THROUGH:
			if (stack_size(S) < 2)
			{
				return StackEmpty;
			}
			container = popS();
			if (getType(container) != T_LIST)
			{
				clear_ref(container);
				return TypeError;
			}
			push(toStack(container), popS());
			pushS(container);
			break;
		case OP_DROP:
			if (stack_size(S) < 1)
			{
				return StackEmpty;
			}
			clear_ref(popS());
			break;
		case OP_DUP:
			if (stack_size(S) < 1)
			{
				return StackEmpty;
			}
			pushS(add_ref(get_head(S)));
			break;
		case OP_SWAP:
			if (stack_size(S) < 2)
			{
				return StackEmpty;
			}
			V v1 = popS();
			V v2 = popS();
			pushS(v1);
			pushS(v2);
			break;
		case OP_ROT:
			if (stack_size(S) < 3)
			{
				return StackEmpty;
			}
			v = S->nodes[S->used-3];
			S->nodes[S->used-3] = S->nodes[S->used-2];
			S->nodes[S->used-2] = S->nodes[S->used-1];
			S->nodes[S->used-1] = v;
			break;
		case OP_OVER:
			if (stack_size(S) < 2)
			{
				return StackEmpty;
			}
			pushS(add_ref(S->nodes[S->used - 2]));
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
			push(scope_arr, add_rooted(v));
			sc = toScope(v);
			sc->is_error_handler = true;
			sc->pc += argument - 1;
			break;
		case OP_RAISE:
			if (stack_size(S) < 1)
			{
				return StackEmpty;
			}
			v = popS();
			if (getType(v) != T_IDENT)
			{
				return TypeError;
			}
			return ident_to_error(v);
		case OP_RERAISE:
			if (stack_size(S) < 1)
			{
				return StackEmpty;
			}
			v = popS();
			if (getType(v) != T_IDENT)
			{
				return TypeError;
			}
			extern bool reraise;
			reraise = true;
			return ident_to_error(v);
		case OP_NEW_DICT:
			pushS(new_dict());
			break;
		case OP_HAS_DICT:
			if (stack_size(S) < 2)
			{
				return StackEmpty;
			}
			container = popS();
			key = popS();
			if (getType(container) != T_DICT)
			{
				return TypeError;
			}
			v = real_get_hashmap(toHashMap(container), key);
			pushS(add_ref(v != NULL ? v_true : v_false));
			clear_ref(container);
			clear_ref(key);
			break;
		case OP_GET_DICT:
			if (stack_size(S) < 2)
			{
				return StackEmpty;
			}
			container = popS();
			key = popS();
			if (getType(container) == T_DICT)
			{
				v = get_hashmap(toHashMap(container), key);
			}
			else if (getType(container) == T_LIST)
			{
				if (getType(key) != T_NUM)
				{
					clear_ref(container);
					clear_ref(key);
					return TypeError;
				}
				int index = (int)toNumber(key);
				Stack *s = toStack(container);
				if (index < 0)
					index = s->used + index;
				if (index < 0 || index >= s->used)
				{
					v = NULL;
				}
				else
				{
					v = s->nodes[index];
				}
			}
			else
			{
				clear_ref(container);
				clear_ref(key);
				return TypeError;
			}
			if (v == NULL)
			{
				clear_ref(container);
				clear_ref(key);
				return ValueError;
			}
			pushS(add_ref(v));
			clear_ref(container);
			clear_ref(key);
			break;
		case OP_SET_DICT:
			if (stack_size(S) < 3)
			{
				return StackEmpty;
			}
			container = popS();
			key = popS();
			v = popS();
			if (getType(container) == T_DICT)
			{
				set_hashmap(toHashMap(container), key, v);
			}
			else if (getType(container) == T_LIST)
			{
				if (getType(key) != T_NUM)
				{
					clear_ref(container);
					clear_ref(key);
					clear_ref(v);
					return TypeError;
				}
				int index = (int)toNumber(key);
				Stack *s = toStack(container);
				if (index < 0)
					index = s->used + index;
				if (index < 0 || index >= s->used)
				{
					clear_ref(key);
					clear_ref(v);
					clear_ref(container);
					return ValueError;
				}
				else
				{
					s->nodes[index] = add_ref(v);
				}
			}
			else
			{
				clear_ref(key);
				clear_ref(v);
				clear_ref(container);
				return TypeError;
			}
			clear_ref(key);
			clear_ref(v);
			clear_ref(container);
			break;
		case OP_CALL:
			v = popS();
			if (getType(v) == T_IDENT)
			{
				key = v;
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
			}
			if (getType(v) == T_FUNC)
			{
				push(scope_arr, add_rooted(new_function_scope(v)));
				clear_ref(v);
			}
			else if (getType(v) == T_CFUNC)
			{
				Error r = toCFunc(v)(S, scope_arr);
				clear_ref(v);
				return r;
			}
			else
			{
				pushS(v);
			}
			break;
	}
	return Nothing;
}
