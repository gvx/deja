from convert import *

valued_opcodes = set('PUSH_WORD PUSH_LITERAL SET SET_LOCAL SET_GLOBAL GET GET_GLOBAL'.split())

class Contain(object):
	def __init__(self, t, value):
		self.type = t
		self.value = value
		self.count = 1
		self.index = None

class Bucket(object):
	def __init__(self):
		self.bucket = []
	def get_value_and_type(self, v):
		if isinstance(v, (ProperWord, Ident)):
			b = 'ident'
		elif isinstance(v, Number):
			b = 'num'
		elif isinstance(v, String):
			b = 'str'
		return v, b
	def add(self, v):
		value, b = self.get_value_and_type(v)
		for item in self.bucket:
			if item.value == value and item.type == b: #need to disambiguate str and ident
				item.count += 1
				break
		else:
			self.bucket.append(Contain(b, value))
	def get(self, v):
		value, b = self.get_value_and_type(v)
		for item in self.bucket:
			if item.value == value and item.type == b: #need to disambiguate str and ident
				return item.index
	def sort(self):
		self.bucket.sort(key=lambda x: x.count, reverse=True)
	def number(self):
		for i, item in enumerate(self.bucket):
			item.index = i
	def raw(self):
		return [(p.type, p.value) for p in self.bucket]

def collect(flat_file):
	buck = Bucket()
	for instruction in flat_file:
		if instruction.opcode in valued_opcodes:
			buck.add(instruction.ref)
	buck.sort()
	bucket.number()
	for instruction in flat_file:
		if instruction.opcode in valued_opcodes:
			instruction.ref = bucket.get(instruction.ref)
	return flat_file, bucket.raw()
