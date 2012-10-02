import struct
from bytecode import (
	OPCODES, unsigned_int_s, signed_int_s, double_s, signed_long_int_s,
	unsigned_long_int_s, signed_char_s, positional_instructions
)

DECODE_OPCODES = {}
for k in OPCODES:
	DECODE_OPCODES[OPCODES[k] / 0x1000000] = k

WORD_ARG = set('GET SET GET_GLOBAL SET_GLOBAL SET_LOCAL PUSH_LITERAL PUSH_WORD SOURCE_FILE'.split())
POS_ARG = positional_instructions

def d_unsigned_int(x):
	return unsigned_int_s.unpack('\x00' + x)[0]

def unsigned_int(x):
	return unsigned_int_s.unpack(x)[0]

def unsigned_long_int(x):
	return unsigned_long_int_s.unpack(x)[0]

def signed_long_int(x):
	return signed_long_int_s.unpack(x)[0]

def signed_char(x):
	return signed_char_s.unpack(x)[0]

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
			if s == '\x80':
				length = ord(self.source[1])
				b = "'" + self.source[2:2 + length] + "'"
				self.source = self.source[2 + length:]
			elif s == '\x81':
				length = ord(self.source[1]) #<-- length?
				b = '"' + self.source[2:2 + length] + '"'
				self.source = self.source[2 + length:]
			elif s == '\x02':
				b = d_double(self.source[1:9])
				self.source = self.source[9:]
			elif s == '\x82':
				b = d_signed_int(self.source[1:4])
				self.source = self.source[4:]
			elif s == '\x07':
				n = signed_long_int(self.source[1:9])
				d = signed_long_int(self.source[9:17])
				b = str(n) + '/' + str(d)
				self.source = self.source[17:]
			elif s == '\x87':
				n = signed_char(self.source[1])
				d = ord(self.source[2])
				b = str(n) + '/' + str(d)
				self.source = self.source[3:]
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
	elif op == 'LINE_NUMBER':
		arg = d_unsigned_int(x[1:])
	else:
		arg = ''
	return '%03d %s %s' % (i, op, arg)

def dis_00(text):
	if len(text) < 4:
		raise Exception("Code file too short")
	size = unsigned_int(text[:4])
	text = text[4:]
	code = [text[j * 4:j * 4 + 4] for j in range(size)]
	literals = Literals(text[size * 4:])
	return '\n'.join(make_line_00(i, x, literals) for i, x in enumerate(code))

def dis(text):
	if not text.startswith('\x07DV'):
		raise Exception("Not a Deja Vu byte code file.")
	elif text[3] in ('\x00', '\x01', '\x02', '\x03'):
		return dis_00(text[4:])
	else:
		raise Exception("Byte code version not recognised.")

if __name__ == '__main__':
	import sys
	sys.stdout.write(dis(sys.stdin.read()))
