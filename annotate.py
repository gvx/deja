import struct
from bytecode import OPCODES, unsigned_int_s, signed_int_s, double_s

DECODE_OPCODES = {}
for k in OPCODES:
	DECODE_OPCODES[OPCODES[k] / 0x1000000] = k

WORD_ARG = set('GET SET GET_GLOBAL SET_GLOBAL SET_LOCAL PUSH_LITERAL PUSH_WORD SOURCE_FILE'.split())
POS_ARG = set('JMP JMPZ LABDA'.split())
LIT_ARG = set('PUSH_INTEGER LINE_NUMBER'.split())

clb = '\033[%sm'

header = clb % 96
opcode = clb % 0
arg_index = clb % 93
arg_literal = clb % 94
arg_offset = clb % 91
arg_nothing = clb % 30
literal_type = clb % 0
literal_str = clb % 31
literal_ident = clb % 32
literal_num = clb % 33
end = clb % 0

def unsigned_int(x):
	return unsigned_int_s.unpack(x)[0]

_count = -1
def nl():
	global _count
	_count += 1
	if _count == 8:
		_count = 0
		return '\n'
	else:
		return ''

def hexify(s):
	return ' '.join(nl() + '%02x' % ord(x) for x in s) + ' '

def ann(text):
	yield header
	yield hexify(text[:8])
	size = unsigned_int(text[4:8])

	text = text[8:]
	for j in range(0, size * 4, 4):
		yield opcode
		yield hexify(text[j])
		op = DECODE_OPCODES[ord(text[j])]
		if op in WORD_ARG:
			yield arg_index
		elif op in LIT_ARG:
			yield arg_literal
		elif op in POS_ARG:
			yield arg_offset
		else:
			yield arg_nothing
		yield hexify(text[j + 1:j + 4])

	text = text[size * 4:]
	i = 0
	while i < len(text):
		yield literal_type
		s = text[i]
		yield hexify(s)
		if s == '\x02':
			j = 9
			yield literal_num
		elif s < '\x80':
			j = unsigned_int(text[i+1:i+5]) + 5
			if s == '\x00':
				yield literal_ident
			elif s == '\x01':
				yield literal_str
		else:
			j = ord(text[i+1]) + 2
			if s == '\x80':
				yield literal_ident
			elif s == '\x81':
				yield literal_str
		yield hexify(text[i+1:i+j])
		i += j
	yield '\n'
	yield end


if __name__ == '__main__':
	import sys
	sys.stdout.write(''.join(ann(sys.stdin.read())))
