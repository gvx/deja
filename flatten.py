from errors import *
from parse import *

from nodes import *
from func import *
from loop import *
from ifelse import *

class FlatNode(object):
	pass

class Code(FlatNode):
	def __init__(self, words):
		self.words = words

class GoTo(FlatNode):
	def __init__(self, index):
		self.index = index

class Branch(FlatNode):
	"""Branch-If-Zero object"""
	def __init__(self, index):
		self.index = index

class LabdaNode(FlatNode):
	def __init__(self, index):
		self.index = index

class Marker(object):
	pass

class SingleInstruction(object):
	def __init__(self, opcode, ref):
		self.opcode = opcode
		self.ref = ref

class FreeLocals(object):
	def __init__(self):
		self.used = set()
	@staticmethod
	def charify(x):
		if x > 255:
			return self.charify(x // 256) + chr(x % 256)
		return chr(x % 256)
	@staticmethod
	def uncharify(x):
		if len(x) > 1:
			return self.uncharify(x[1:]) * 256 + ord(x[0])
		return ord(x)
	def get_next(self):
		i = 0
		while i in self.used:
			i += 1
		self.used.add(i)
		return '#' + self.charify(i)
	def free(self, x):
		assert x.startswith('#')
		assert len(x) > 1
		self.used.remove(self.uncharify(x[1:]))

free_locals = FreeLocals()

def flatten(tree, acc=None):
	if acc is None:
		acc = []
	for branch in tree.children:
		if isinstance(branch, Word):
			if acc and isinstance(acc[-1], Code):
				acc[-1].words.append(branch)
			else:
				acc.append(Code([branch]))
		elif isinstance(branch, WordList):
			if acc and isinstance(acc[-1], Code):
				acc[-1].words.extend(branch.children)
			else:
				acc.append(Code(list(branch.children)))
		elif isinstance(branch, LabdaStatement):
			m = Marker()
			acc.append(LabdaNode(m))
			for argument in branch.arguments:
				acc.append(SingleInstruction('SET_LOCAL', argument))
			flatten(branch, acc)
			acc.append(SingleInstruction('RETURN', 0))
			acc.append(m)
			if isinstance(branch, LocalFuncStatement):
				acc.append(SingleInstruction('SET_LOCAL', branch.name))
			elif isinstance(branch, FuncStatement):
				acc.append(SingleInstruction('SET_GLOBAL', branch.name))
		elif isinstance(branch, WhileStatement):
			m1 = Marker()
			m2 = Marker()
			acc.append(SingleInstruction('ENTER_CLOSURE', 0))
			acc.append(m1)
			flatten(branch.conditionclause, acc)
			acc.append(Branch(m2))
			flatten(branch.body, acc)
			acc.append(GoTo(m1))
			acc.append(m2)
			acc.append(SingleInstruction('LEAVE_CLOSURE', 0))
		elif isinstance(branch, ForStatement):
			m1 = Marker()
			m2 = Marker()
			flatten(branch.forclause, acc)
			acc.append(m1)
			acc.append(SingleInstruction('DUP', 0))
			acc.append(Branch(m2))
			acc.append(SingleInstruction('ENTER_CLOSURE', 0))
			a = free_locals.get_next()
			b = free_locals.get_next()
			acc.append(SingleInstruction('SET_LOCAL', a))
			acc.append(SingleInstruction('SET_LOCAL', b))
			acc.append(SingleInstruction('SET_LOCAL', branch.countername))
			flatten(branch.body, acc)
			acc.append(SingleInstruction('PUSH_WORD', b))
			acc.append(SingleInstruction('PUSH_WORD', a))
			acc.append(SingleInstruction('LEAVE_CLOSURE', 0))
			acc.append(GoTo(m1))
			free_locals.free(a)
			free_locals.free(b)
			acc.append(m2)
			acc.append(SingleInstruction('DROP', 0))
			acc.append(SingleInstruction('DROP', 0))
			acc.append(SingleInstruction('DROP', 0))
		elif isinstance(branch, IfStatement):
			m_end = Marker()
			m = Marker()
			acc.append(SingleInstruction('ENTER_CLOSURE', 0))
			flatten(branch.ifclause.conditionclause, acc)
			acc.append(Branch(m))
			flatten(branch.ifclause, acc)
			acc.append(GoTo(m_end))
			acc.append(m)
			for elseifclause in branch.elseifclauses:
				m = Marker()
				flatten(elseif.conditionclause, acc)
				acc.append(Branch(m))
				flatten(elseifclause, acc)
				acc.append(GoTo(m_end))
				acc.append(m)
			if branch.elseclause:
				flatten(branch.elseclause, acc)
			acc.append(m_end)
			acc.append(SingleInstruction('LEAVE_CLOSURE', 0))
		#elif isinstance(branch, CatchStatement): # left purposefully unimplemented
	return acc
