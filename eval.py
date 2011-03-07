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

class Closure(WordObject):
	def __init__(self, env, parent, node):
		self.env = env
		self.parent = parent
		self.node = node
		self.words = {}
	def getword(self, ident):
		if ident in self.words:
			return self.words[ident]
		elif self.parent:
			return self.parent.getword(self, ident)
		else:
			return self.env.getword(env)
	def setlocal(self, ident, value):
		self.words[ident] = value

class Environment(object):
	def __init__(self):
		self.running = True
		self.words = {}
		self.stack = []
		self.call_stack = []

	def pushword(self, word):
		if isinstance(word, Closure):
			self.call_stack.append(word)
			r = self.step_eval(word.node, closure)
			if r:
				return r
			self.call_stack.pop()
		else:
			self.stack.append(word)
	
	def popvalue(self):
		if self.call_stack:
			return self.call_stack.pop()
		else:
			return DejaStackEmpty(self, None)

	def makeword(self, word, closure):
		if isinstance(word, (Number, String)):
			return word.value
		elif isinstance(word, Ident):
			return IdentObject(word.value)
		elif isinstance(word, ProperWord):
			return closure.getword(word.value)

	def step_eval(self, node, closure = None):
		if not closure or isinstance(node, (File, Statement)):
			closure = Closure(self, closure, node)

		if isinstance(node, (File, WordList, Clause)):
			for child in node.children:
				if not self.step_eval(child, closure):
					return False
		elif isinstance(node, Word):
			return self.pushword(self.makeword(node, closure), closure)
		elif isinstance(node, IfStatement):
			r = self.step_eval(node.ifclause.conditionclause, closure)
			if r:
				return r
			r = self.popvalue()
			if isinstance(r, DejaError):
				return r
			elif r:
				return self.step_eval(node.ifclause, closure)
			for elseif in node.elseifclauses:
				r = self.step_eval(elseif.conditionclause, closure)
				if r:
					return r
				r = self.popvalue()
				if isinstance(r, DejaError):
					return r
				elif r:
					return self.step_eval(elseif, closure)
			if node.elseclause:
				return self.step_eval(node.elseclause, closure)
		elif isinstance(node, WhileStatement):
			while True:
				r = self.step_eval(node.conditionclause, closure)
				if r or not self.popvalue():
					return r
				r = self.step_eval(node.body, closure)
				if r:
					return r
		elif isinstance(node, ForStatement):
			pass #find a way to implement for
		elif isinstance(node, FuncStatement):
			return self.setword(node.name, closure)
		elif isinstance(node, LabdaStatement):
			self.stack.append(closure)
		elif isinstance(node, CatchStatement):
			orig_callstack = len(self.call_stack)
			r = self.step_eval(node.body, closure)
			if r:
				self.pushword(self.call_stack[orig_callstack:])
				self.pushword(IdentObject(r.dj_str))
				self.call_stack = self.call_stack[:orig_callstack]
				return self.step_eval(node.errorhandler, closure)

	def traceback(self, r):
		if isinstance(r, DejaError):
			print(r)


def eval(node, env=None):
	if not env:
		env = Environment()
	env.traceback(env.step_eval(node))
