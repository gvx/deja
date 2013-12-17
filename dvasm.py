from bytecode import *
from collections import defaultdict
from strquot import unquote

def asm(intext):
	acc = [HEADER, chr(VERSION[0] * 16 + VERSION[1]), None]
	mode = 'code'
	labels = {}
	label_reqs = defaultdict(list)
	literal_reqs = defaultdict(list)
	num_inst = 0
	lit_index = 0
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
				arg = 0
				if words:
					if words[0].startswith('$'):
						label_reqs[words[0][1:]].append((num_inst, len(acc)))
					elif words[0].startswith('%'):
						literal_reqs[words[0][1:]].append(len(acc))
					else:
						arg = int(words[0])
				acc.append(OPCODES[s.upper()] | (arg & 0xFFFFFF))
		else:
			if line:
				for len_acc in literal_reqs.pop(line, ()):
					acc[len_acc] |= lit_index & 0xFFFFFF
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
					r = unquote(r)
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
				lit_index += 1
	if mode == 'code':
		raise Exception("Literal segment missing!")
	if literal_reqs:
		undefined_literals = literal_reqs.keys()
		if len(undefined_literals) > 1:
			raise Exception("Mentioned literals %s and %s do not exist." % (', '.join(undefined_literals[0:-1]), undefined_literals[-1]))
		else:
			raise Exception("Mentioned literal %s does not exist." % undefined_literals[0])
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
