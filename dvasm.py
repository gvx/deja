from bytecode import *
from collections import defaultdict

def asm(intext):
	acc = [HEADER, chr(VERSION[0] * 16 + VERSION[1]), None]
	mode = 'code'
	labels = {}
	label_reqs = defaultdict(list)
	num_inst = 0
	last_code = None
	for line in intext.split('\n'):
		if mode == 'code':
			words = line.split()
			if words:
				s = words.pop(0)
				if s == '...':
					acc[2] = unsigned_int(num_inst)
					mode = 'literals'
					last_code = len(acc)
					continue
				if s[0] == '$':
					labels[s[1:]] = num_inst
					if not words:
						continue
					s = words.pop(0)
				num_inst += 1
				if s.upper() not in OPCODES:
					raise Exception("%s not a legal opcode." % s)
				if not words:
					arg = 0
				elif words[0].startswith('$'):
					label_reqs[words[0][1:]].append((num_inst, len(acc)))
					arg = 0
				else:
					arg = int(words[0])
				acc.append(OPCODES[s.upper()] | (arg & 0xFFFFFF))
		else:
			if line:
				t = line[0]
				r = line[1:]
				if t == 'i':
					if len(r) < 256:
						acc.append('\x80')
						acc.append(chr(len(r)))
						acc.append(r)
					else:
						acc.append('\x00')
						acc.append(unsigned_int(len(r)))
						acc.append(r)
				elif t == 's':
					if len(r) < 256:
						acc.append('\x81')
						acc.append(chr(len(r)))
						acc.append(r)
					else:
						acc.append('\x01')
						acc.append(unsigned_int(len(r)))
						acc.append(r)
				elif t == 'f':
					n, d = r.split('/', 1)
					n, d = int(n), int(d)
					if -128 <= n < 128 and d < 256:
						acc.append('\x87')
						acc.append(signed_char(int(n)))
						acc.append(chr(int(d)))
					else:
						acc.append('\x07')
						acc.append(signed_long_int(int(n)))
						acc.append(unsigned_long_int(int(d)))
				elif t == 'n':
					acc.append('\x02')
					acc.append(double(float(r)))
				else:
					raise Exception("%s is not a legal literal type." % t)
	if mode == 'code':
		raise Exception("Literal segment missing!")
	for l in label_reqs:
		if l not in labels:
			raise Exception("Label %s used but not defined." % l)
		for i, loc in label_reqs[l]:
			acc[loc] |= (labels[l] - i + 1) & 0xFFFFFF
	for i in range(3, last_code):
		acc[i] = unsigned_int(acc[i])
	return acc

if __name__ == '__main__':
	import sys
	sys.stdout.write(''.join(asm(sys.stdin.read())))
