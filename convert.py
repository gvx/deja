from flatten import *

POS_SIZE = 2**23 - 1
NEG_SIZE = -2**23

OPTIMIZE = True
OPTIMIZERS = {
	'set': 'SET',
	'setglobal': 'SET_GLOBAL',
	'local': 'SET_LOCAL',
	'get': 'GET',
	'getglobal': 'GET_GLOBAL',
	'return': 'RETURN',
	'recurse': 'RECURSE',
	'drop': 'DROP',
	'dup': 'DUP',
	'[]': 'NEW_LIST',
	'{}': 'NEW_DICT',
	'swap': 'SWAP',
	'rot': 'ROT',
	'over': 'OVER',
	'pop-from': 'POP_FROM',
	'push-to': 'PUSH_TO',
	'push-through': 'PUSH_THROUGH',
	'has': 'HAS_DICT',
	'get-from': 'GET_DICT',
	'set-to': 'SET_DICT',
}
ARGED_OPT = set('SET SET_LOCAL SET_GLOBAL GET GET_GLOBAL'.split())

positional_instructions = set('JMP JMPZ LABDA ENTER_ERRHAND'.split())

def convert(filename, flat):
	bytecode = [SingleInstruction('SOURCE_FILE', String(None, filename))]
	for k in flat:
		if isinstance(k, SingleInstruction):
			bytecode.append(k)
		elif isinstance(k, Code):
			for w in k.words:
				if isinstance(w, ProperWord):
					if OPTIMIZE and w.value in OPTIMIZERS:
						if OPTIMIZERS[w.value] in ARGED_OPT:
							if bytecode and bytecode[-1].opcode == 'PUSH_LITERAL' and isinstance(bytecode[-1].ref, Ident):
								s = bytecode.pop().ref
							else:
								bytecode.append(SingleInstruction('PUSH_WORD', w))
								continue
						else:
							s = 0
						bytecode.append(SingleInstruction(OPTIMIZERS[w.value], s))
					elif w.value == 'for:':
						mstart = Marker()
						mend = Marker()
						bytecode.extend([
							mstart,
							SingleInstruction('DUP', 0),
							SingleInstruction('JMPZ', mend),
							SingleInstruction('PUSH_WORD', 'call'),
							SingleInstruction('JMP', mstart),
							mend,
							SingleInstruction('DROP', 0)
						])
					elif w.value == '(:split:)':
						mparent = Marker()
						mchild = Marker()
						bytecode.extend([
							SingleInstruction('LABDA', mparent),
							SingleInstruction('JMP', mchild),
							mparent,
							SingleInstruction('RETURN', 0),
							mchild,
						])
					else:
						bytecode.append(SingleInstruction('PUSH_WORD', w))
				elif isinstance(w, Number) and w.value.is_integer() and w.value <= POS_SIZE and w.value >= NEG_SIZE:
					bytecode.append(SingleInstruction('PUSH_INTEGER', int(w.value)))
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
	bytecode.append(SingleInstruction('RETURN', 0))
	return bytecode

def is_return(node):
	return isinstance(node, SingleInstruction) and node.opcode == 'RETURN'

def optimize(flattened): #optimize away superfluous RETURN statements
	prev_instruction = None
	prev_prev_instruction = None
	for instruction in reversed(flattened):
		if is_return(instruction) and (is_return(prev_instruction) or (isinstance(prev_instruction, Marker) and is_return(prev_prev_instruction))):
			flattened.remove(instruction)
		prev_prev_instruction = prev_instruction
		prev_instruction = instruction
	return flattened

def refine(flattened): #removes all markers and replaces them by indices
	#first pass: fill dictionary
	memo = {}
	i = 0
	while i < len(flattened):
		item = flattened[i]
		if isinstance(item, Marker):
			memo[item] = i
			del flattened[i]
		else:
			i += 1
	#second pass: change all goto and branches
	for i, item in enumerate(flattened):
		if item.opcode in positional_instructions:
			item.ref = memo[item.ref] - i
	return flattened
