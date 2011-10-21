import struct
from bytecode import OPCODES, unsigned_int_s, signed_int_s, double_s

DECODE_OPCODES = {}
for k in OPCODES:
	DECODE_OPCODES[OPCODES[k] / 0x1000000] = k

WORD_ARG = set('GET SET GET_GLOBAL SET_GLOBAL SET_LOCAL PUSH_LITERAL PUSH_WORD'.split())
POS_ARG = set('JMP JMPZ LABDA'.split())

def d_unsigned_int(x):
	return unsigned_int_s.unpack('\x00' + x)[0]

def unsigned_int(x):
	return unsigned_int_s.unpack(x)[0]

def d_signed_int(x):
	if x[0] >= '\x80':
		x = '\xff' + x
	else:
		x = '\x00' + x 
	return signed_int_s.unpack(x)[0]

def d_double(x):
	return double_s.unpack(x)[0]

class Literals(object):
	def __init__(self, source):
		self.source = source
		self.cache = []
	def __getitem__(self, item):
		while item >= len(self.cache):
			s = self.source[0]
			if s == '\x00':
				length = unsigned_int(self.source[1:5])
				b = "'" + self.source[5:5 + length] + "'"
				self.source = self.source[5 + length:]
			elif s == '\x01':
				length = unsigned_int(self.source[1:5]) #<-- length?
				b = '"' + self.source[5:5 + length] + '"'
				self.source = self.source[5 + length:]
			elif s == '\x02':
				b = d_double(self.source[:8+1])
				self.source = self.source[8+1:]
			self.cache.append(b)
		return self.cache[item]

def make_line_00(i, x, literals):
	op = DECODE_OPCODES[ord(x[0])]
	if op in WORD_ARG:
		arg = literals[d_unsigned_int(x[1:])]
	elif op == 'PUSH_INTEGER':
		arg = d_signed_int(x[1:])
	elif op in POS_ARG:
		arg = i + d_signed_int(x[1:])
	else:
		return op
	return op + ' ' + str(arg)

def dis_00(text):
	if len(text) < 4:
		raise Exception("Code file to short")
	size = unsigned_int(text[:4])
	text = text[4:]
	code = [text[j * 4:j * 4 + 4] for j in range(size)]
	literals = Literals(text[size * 4:])
	return '\n'.join(make_line_00(i, x, literals) for i, x in enumerate(code))

def dis(text):
	if not text.startswith('\x07DV'):
		raise Exception("Not a Deja Vu byte code file.")
	if text[3] == '\x00':
		return dis_00(text[4:])
	else:
		raise Exception("Byte code version not recoginised.")

if __name__ == '__main__':
	import sys
	sys.stdout.write(dis(sys.stdin.read()))
