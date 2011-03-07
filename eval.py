from errors import *

from nodes import *
from ifelse import *
from loop import *
from func import *
from trycatch import *

import weakref

class WordObject(object):
	pass

StringObject = str
NumberObject = int
StackObject = list

class IdentObject(WordObject):
	idents = weakref.WeakValueDictionary()
	def __new__(cls, name):
		if name not in self.idents:
			self.idents[name] = object.__new__(cls)
			self.idents[name].name = name
		return self.idents[name]

class FuncObject(WordObject):
	def __init__(self, node):
		self.func_node = node

class Environment(object):
	def __init__(self):
		self.running = True
		self.words = {}
		self.stack = []
		self.call_stack = []

	def pushword(self, word):
		if isinstance(word, FuncObject):
			self.call_stack.append(word.func_node)
			self.step_eval(word.func_node)
		else:
			self.stack.append(word)

	def makeword(self, word):
		if isinstance(word, (Number, String)):
			return word.value
		elif isinstance(word, Ident):
			return IdentObject(word.value)
		elif isinstance(word, ProperWord):
			return self.getword(word.value)

	def step_eval(self, node):
		if isinstance(node, (File, WordList, Clause)):
			for child in node.children:
				if not self.step_eval(child):
					return False
		elif isinstance(node, Word):
			self.pushword(self.makeword(node))
			return True
		elif isinstance(node, IfStatement):
			r = self.step_eval(node.ifclause.conditionclause)
			if not r:
				return r
			if self.popvalue():
				return self.step_eval(node.ifclause)
			for elseif in node.elseifclauses:
				r = self.step_eval(elseif.conditionclause)
				if not r:
					return r
				if self.popvalue():
					return self.step_eval(elseif)
			if node.elseclause:
				return self.step_eval(node.elseclause)
		elif isinstance(node, WhileStatement):
			while True:
				r = self.step_eval(node.conditionclause)
				if not (r and self.popvalue()):
					return r
				r = self.step_eval(node.body)
				if not r:
					return r
		elif isinstance(node, FuncStatement):
			self.setword(node.name, node)
		elif isinstance(node, LabdaStatement):
			self.stack.append(node)
		elif isinstance(node, CatchStatement):
			r = self.step_eval(node.body)
			if not r:
				r = self.step_eval(node.errorhandler)
				if not r:
					return r

		return node

def eval(node, env=None):
	if not env:
		env = Environment()
	env.step_eval(node)
