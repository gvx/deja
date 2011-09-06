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

class FuncDef(FlatNode):
	def __init__(self, func, name, is_local):
		self.func = func
		self.name = name
		self.is_local = is_local

class PushErrorHandler(FlatNode):
	def __init__(self, index):
		self.index = index

class PopErrorHandler(FlatNode):
	pass

class ForHeader(FlatNode):
	def __init__(self, index, name):
		self.index = index
		self.name = name

class Marker(object):
	_I = 0
	def __init__(self):
		Marker._I += 1
		self.i = Marker._I

class SingleInstruction(object):
	def __init__(self, opcode, ref):
		self.opcode = opcode
		self.ref = ref

class FreeLocals(object):
	def __init__(self):
		self.used = set()
	def charify(x):
		if x > 255:
			return self.charify(x // 256) + chr(x % 256)
		return chr(x % 256)
	def uncharify(x):
		if len(x) > 1:
			return self.uncharify(x[1:]) * 256 + ord(x[0])
		return ord(x)
	def get_next():
		i = 0
		while i in self.used:
			i += 1
		self.used.add(i)
		return '#' + self.charify(i)
	def free(x):
		assert x.starts_with('#')
		assert len(x) > 1
		self.used.remove(self.uncharify(x[1:]))

free_locals = FreeLocals()

def refine(flattened): #removes all markers and replaces them by indices
	#first pass: fill dictionary
	memo = {}
	for item in reversed(flattened):
		i = flattened.index(item)
		if isinstance(item, Marker, int):
			memo[item] = i
		del flattened[i]
	#second pass: change all goto and branches
	for item in flattened:
		if hasattr(item, 'index'):
			item.index = memo[item.index]
	return flattened

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
			acc.append(flatten(branch))
			acc.append(SingleInstruction('RETURN', 0))
			acc.append(m)
			if isinstance(branch, LocalFuncStatement):
				acc.append(SingleInstruction('SET_LOCAL', branch.name))
			elif isinstance(branch, FuncStatement):
				acc.append(SingleInstruction('SET', branch.name))
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
