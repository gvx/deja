from flatten import *

POS_SIZE = 2**31 - 1
NEG_SIZE = -2**31

OPTIMIZE = True
OPTIMIZERS = {
	'set': 'SET',
	'setglobal': 'SET_GLOBAL',
	'local': 'SET_LOCAL',
	'get': 'GET',
	'getglobal': 'GET_GLOBAL',
	'return': 'RETURN',
	'+': 'ADD',
	'add': 'ADD',
	'-': 'SUB',
	'sub': 'SUB',
	'*': 'MUL',
	'mul': 'MUL',
	'/': 'DIV',
	'div': 'DIV',
	'%': 'MOD',
	'mod': 'MOD',
	'drop': 'DROP',
	'dup': 'DUP',
	'[]': 'NEW_STACK',
}
ARGED_OPT = set('SET SET_LOCAL SET_GLOBAL GET GET_GLOBAL'.split())

positional_instructions = set('JMP JMPZ LABDA'.split())

def convert(flat):
	bytecode = []
	for k in flat:
		if isinstance(k, SingleInstruction):
			bytecode.append(k)
		elif isinstance(k, Code):
			for w in k.words:
				if isinstance(w, ProperWord):
					if OPTIMIZE and w.value in OPTIMIZERS:
						if w.value in ARGED_OPT:
							if acc and acc[-1].opcode == 'PUSH_LITERAL' and acc[-1].ref.type == 'ident':
								s = acc.pop().ref
							else:
								bytecode.append(SingleInstruction('PUSH_WORD', w))
								continue
						else:
							s = 0
						bytecode.append(SingleInstruction(OPTIMIZERS[w.value], s))
					else:
						bytecode.append(SingleInstruction('PUSH_WORD', w))
				elif isinstance(w, Number) and w.convert().is_integer() and w.convert() <= POS_SIZE and w.convert() >= NEG_SIZE:
					bytecode.append(SingleInstruction('PUSH_INTEGER', int(w.convert())))
				else:
					bytecode.append(SingleInstruction('PUSH_LITERAL', w))
		elif isinstance(k, Marker):
			bytecode.append(k)
		elif isinstance(k, GoTo):
			bytecode.append(SingleInstruction('JMP', k.index))
		elif isinstance(k, Branch):
			bytecode.append(SingleInstruction('JMPZ', k.index))
		elif isinstance(k, LabdaNode):
			bytecode.append(SingleInstruction('LABDA', k.index))
	return bytecode

def refine(flattened): #removes all markers and replaces them by indices
	#first pass: fill dictionary
	memo = {}
	for item in reversed(flattened):
		i = flattened.index(item)
		if isinstance(item, Marker):
			memo[item] = i
			del flattened[i]
	#second pass: change all goto and branches
	for i, item in enumerate(flattened):
		if positional_instructions:
			item.ref = memo[item.ref] - i
	return flattened
