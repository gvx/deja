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
	def __repr__(self):
		return str(self.opcode) + ' ' + str(self.ref)

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
			if isinstance(branch, Line):
				acc.append(SingleInstruction('LINE_NUMBER', branch.linenr))
			if acc and isinstance(acc[-1], Code):
				acc[-1].words.extend(branch.children)
			else:
				acc.append(Code(list(branch.children)))
		elif isinstance(branch, LabdaStatement):
			m = Marker()
			acc.append(LabdaNode(m))
			for argument in branch.arguments:
				acc.append(SingleInstruction('SET_LOCAL', argument))
			flatten(branch.body, acc)
			acc.append(SingleInstruction('RETURN', 0))
			acc.append(m)
			if isinstance(branch, LocalFuncStatement):
				acc.append(SingleInstruction('SET_LOCAL', branch.name))
			elif isinstance(branch, FuncStatement):
				acc.append(SingleInstruction('SET_GLOBAL', branch.name))
		elif isinstance(branch, WhileStatement):
			m1 = Marker()
			m2 = Marker()
			acc.append(SingleInstruction('ENTER_SCOPE', 0))
			acc.append(m1)
			flatten(branch.conditionclause, acc)
			acc.append(Branch(m2))
			flatten(branch.body, acc)
			acc.append(GoTo(m1))
			acc.append(m2)
			acc.append(SingleInstruction('LEAVE_SCOPE', 0))
		elif isinstance(branch, ForStatement):
			m1 = Marker()
			m2 = Marker()
			flatten(branch.forclause, acc)
			acc.append(m1)
			acc.append(SingleInstruction('DUP', 0))
			acc.append(Branch(m2))
			acc.append(SingleInstruction('ENTER_SCOPE', 0))
			acc.append(SingleInstruction('SET_LOCAL', '#f'))
			acc.append(SingleInstruction('SET_LOCAL', '#h'))
			acc.append(SingleInstruction('SET_LOCAL', branch.countername))
			flatten(branch.body, acc)
			acc.append(SingleInstruction('PUSH_WORD', '#h'))
			acc.append(SingleInstruction('PUSH_WORD', '#f'))
			acc.append(SingleInstruction('LEAVE_SCOPE', 0))
			acc.append(GoTo(m1))
			acc.append(m2)
			acc.append(SingleInstruction('DROP', 0))
		elif isinstance(branch, IfStatement):
			m_end = Marker()
			m = Marker()
			acc.append(SingleInstruction('ENTER_SCOPE', 0))
			flatten(branch.ifclause.conditionclause, acc)
			acc.append(Branch(m))
			flatten(branch.ifclause, acc)
			acc.append(GoTo(m_end))
			acc.append(m)
			for elseifclause in branch.elseifclauses:
				m = Marker()
				flatten(elseifclause.conditionclause, acc)
				acc.append(Branch(m))
				flatten(elseifclause, acc)
				acc.append(GoTo(m_end))
				acc.append(m)
			if branch.elseclause:
				flatten(branch.elseclause, acc)
			acc.append(m_end)
			acc.append(SingleInstruction('LEAVE_SCOPE', 0))
		#elif isinstance(branch, CatchStatement): # left purposefully unimplemented
	return acc
