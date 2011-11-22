from collect import *

import struct

HEADER = '\x07DV'
VERSION = (0, 2)
OP_SIZE = 5

OPCODES = {
	'PUSH_LITERAL':		'00000000',
	'PUSH_INTEGER':		'00000001',
	'PUSH_WORD':		'00000010',
	'SET':				'00000011',
	'SET_LOCAL':		'00000100',
	'SET_GLOBAL':		'00000101',
	'GET':				'00000110',
	'GET_GLOBAL':		'00000111',
	'JMP':				'00010000',
	'JMPZ':				'00010001',
	'RETURN':			'00010010',
	'RECURSE':			'00010011',
	'LABDA':			'00100000',
	'ENTER_SCOPE':		'00100001',
	'LEAVE_SCOPE':		'00100010',
	'NEW_LIST':			'00110000',
	'POP_FROM':			'00110001',
	'PUSH_TO':			'00110010',
	'PUSH_THROUGH':		'00110011',
	'DROP':				'01000000',
	'DUP':				'01000001',
	'SWAP':				'01000010',
	'ROT':				'01000011',
	'OVER':				'01000100',
	'LINE_NUMBER':		'01010000',
	'SOURCE_FILE':		'01010010',
	'ENTER_ERRHAND':	'01100000',
	'LEAVE_ERRHAND':	'01100001',
	'NEW_DICT':			'01110000',
	'HAS_DICT':			'01110001',
	'GET_DICT':			'01110010',
	'SET_DICT':			'01110011',
}
for k in OPCODES:
	OPCODES[k] = int(OPCODES[k], 2) * 0x1000000

TYPES = {
	'ident':	'00000000',
	'str':		'00000001',
	'num':		'00000010',
}
for k in TYPES:
	TYPES[k] = chr(int(TYPES[k], 2))

signed_int_s = struct.Struct('>i')
def signed_int(x):
	return signed_int_s.pack(x)

unsigned_int_s = struct.Struct('>I')
def unsigned_int(x):
	return unsigned_int_s.pack(x)

double_s = struct.Struct('>d')
def double(x):
	return double_s.pack(x)

def write_code(code, acc):
	for op in code:
		acc.append(signed_int(OPCODES[op.opcode] | (op.ref & 0xFFFFFF)))

def write_literals(literals, acc):
	for literal in literals:
		acc.append(TYPES[literal[0]])
		if literal[0] == 'num':
			acc.append(double(literal[1]))
		else:
			acc.append(unsigned_int(len(literal[1])))
			acc.append(literal[1])

def write_bytecode(flat_code):
	code, literals = flat_code
	acc = [HEADER, chr(VERSION[0] * 16 + VERSION[1]), unsigned_int(len(code))]
	write_code(code, acc)
	write_literals(literals, acc)
	return ''.join(acc)
