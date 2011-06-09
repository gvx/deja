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
		self.i = _I

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
			acc.append(FuncDef(refine(flatten(branch)), hasattr(branch, 'name') and branch.name or None, isinstance(branch, LocalFuncStatement)))
		else isinstance(branch, WhileStatement):
			m1 = Marker()
			m2 = Marker()
			acc.append(m1)
			flatten(branch.conditionclause, acc)
			acc.append(Branch(m2))
			flatten(branch.body, acc)
			acc.append(GoTo(m1))
			acc.append(m2)
		elif isinstance(branch, ForStatement):
			m1 = Marker()
			m2 = Marker()
			flatten(branch.forclause, acc)
			#///SET LOCAL
			acc.append(m1)
			acc.append(ForHeader(m2, branch.countername))
			flatten(branch.body, acc)
			acc.append(GoTo(m1))
			acc.append(m2)
			pass #???
		elif isinstance(branch, IfStatement):
			m_end = Marker()
			m = Marker()
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
		elif isinstance(branch, CatchStatement):
			m1 = Marker()
			m2 = Marker()
			m_end = Marker()
			acc.append(PushErrorHandler(m2))
			acc.append(GoTo(m1))
			acc.append(m2)
			flatten(branch.errorhandler, acc)
			acc.append(GoTo(m_end))
			acc.append(m1)
			flatten(branch.body, acc)
			acc.append(PopErrorHandler())
			acc.append(m_end)
			
			pass #??? #push/pop error handlers?
	return acc
