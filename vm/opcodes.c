#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>

#include "types.h"
#include "literals.h"
#include "lib.h"
#include "blob.h"

extern V lastCall;

Error inline do_instruction(Header* h, Stack* S, Stack* scope_arr)
{
	V container;
	V v;
	V key;
	V scope = get_head(scope_arr);
	Scope *sc = toScope(scope);
	V file;
	Error e;
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
			while (sc->env == NULL || (v = get_hashmap(toHashMap(sc->env), key)) == NULL)
			{
				if (sc->parent == NULL)
				{
					return NameError;
				}
				sc = toScope(sc->parent);
			}
			if (getType(v) == T_FUNC)
			{
				push(scope_arr, add_rooted(new_function_scope(v, key)));
			}
			else if (getType(v) == T_CFUNC)
			{
				return toCFunc(v)(S, scope_arr);
			}
			else if (getType(v) == T_SCOPE)
			{
				call_scope(scope_arr, v);
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
			while (sc->env == NULL || !change_hashmap(toHashMap(sc->env), key, v))
			{
				if (sc->parent == NULL || toScope(sc->parent)->parent == NULL)
				{
					//set in the semi-global environment
					set_hashmap(toHashMap(sc->env), key, v);
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
			if (sc->env == NULL)
			{
				sc->env = new_sized_dict(16);
			}
			set_hashmap(toHashMap(sc->env), get_literal(h, argument), v);
			clear_ref(v);
			break;
		case OP_SET_GLOBAL:
			if (stack_size(S) < 1)
			{
				return StackEmpty;
			}
			v = popS();
			//set in the semi-global environment
			while (sc->parent != NULL && toScope(sc->parent)->parent != NULL)
			{
				sc = toScope(sc->parent);
			}
			if (sc->env == NULL)
			{
				sc->env = new_sized_dict(16);
			}
			set_hashmap(toHashMap(sc->env), get_literal(h, argument), v);
			clear_ref(v);
			break;
		case OP_GET:
			key = get_literal(h, argument);
			while (sc->env == NULL || (v = get_hashmap(toHashMap(sc->env), key)) == NULL)
			{
				if (sc->parent == NULL)
				{
					return NameError;
				}
				sc = toScope(sc->parent);
			}
			pushS(add_ref(v));
			break;
		case OP_GET_GLOBAL:
			v = get_hashmap(toHashMap(toScope(toFile(sc->file)->global)->env), get_literal(h, argument));
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
			v = get_hashmap(toHashMap(container), key);
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
			e = Nothing;
			switch (getType(container))
			{
				case T_DICT:
					v = get_dict(toHashMap(container), key);
					break;
				case T_LIST:
					if (getType(key) != T_NUM)
					{
						e = TypeError;
						goto cleanupgetfrom;
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
					break;
				case T_BLOB:
					if (getType(key) != T_NUM)
					{
						e = TypeError;
						goto cleanupgetfrom;
					}
					int byte = getbyte_blob(container, (int)toNumber(key));
					if (byte < 0)
					{
						set_error_msg("Index out of range");
						e = ValueError;
						goto cleanupgetfrom;
					}
					v = int_to_value(byte);
					break;
				default:
					e = TypeError;
					goto cleanupgetfrom;
			}
			if (v == NULL)
			{
				e = ValueError;
				goto cleanupgetfrom;
			}
			pushS(add_ref(v));
			cleanupgetfrom:
			clear_ref(container);
			clear_ref(key);
			return e;
		case OP_SET_DICT:
			if (stack_size(S) < 3)
			{
				return StackEmpty;
			}
			container = popS();
			key = popS();
			v = popS();
			e = Nothing;
			switch (getType(container))
			{
				case T_DICT:
					set_hashmap(toHashMap(container), key, v);
					break;
				case T_LIST:
					if (getType(key) != T_NUM)
					{
						e = TypeError;
						goto cleanupsetto;
					}
					int index = (int)toNumber(key);
					Stack *s = toStack(container);
					if (index < 0)
						index = s->used + index;
					if (index < 0 || index >= s->used)
					{
						e = ValueError;
						goto cleanupsetto;
					}
					else
					{
						s->nodes[index] = add_ref(v);
					}
					break;
				case T_BLOB:
					if (getType(key) != T_NUM || getType(v) != T_NUM)
					{
						e = TypeError;
						goto cleanupsetto;
					}
					int num = toNumber(v);
					if (num < 0 || num > 255)
					{
						set_error_msg("Value not in range [0,255]");
						e = ValueError;
						goto cleanupsetto;
					}
					int byte = setbyte_blob(container, toNumber(key), num);
					if (byte < 0)
					{
						set_error_msg("Index out of range");
						e = ValueError;
						goto cleanupsetto;
					}
					break;
				default:
					e = TypeError;
					goto cleanupsetto;
			}
			cleanupsetto:
			clear_ref(key);
			clear_ref(v);
			clear_ref(container);
			return e;
		case OP_CALL:
			if (stack_size(S) < 1)
			{
				return StackEmpty;
			}
			v = popS();
			if (getType(v) == T_FUNC)
			{
				push(scope_arr, add_rooted(new_function_scope(v, NULL)));
				clear_ref(v);
			}
			else if (getType(v) == T_CFUNC)
			{
				Error r = toCFunc(v)(S, scope_arr);
				clear_ref(v);
				return r;
			}
			else if (getType(v) == T_SCOPE)
			{
				call_scope(scope_arr, v);
			}
			else
			{
				pushS(v);
			}
			break;
	}
	return Nothing;
}
