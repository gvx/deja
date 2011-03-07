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
	def __init__(self, name):
		self.name = name
		self.idents[name] = self
	def __str__(self):
		return "'" + self.name + "'"

def getident(name):
	if name not in IdentObject.idents:
		return IdentObject(name)
	else:
		return IdentObject.idents[name]


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
			return self.env.getword(ident)

	def setlocal(self, ident, value):
		self.words[ident] = value

def print_s(env, closure):
	print(env.popvalue())

class Environment(object):
	def __init__(self):
		self.running = True
		self.words = {'.': print_s}
		self.stack = []
		self.call_stack = []
	
	def getword(self, word):
		if word in self.words:
			return self.words[word]
		else:
			raise DejaNameError(self, word)

	def setword(self, ident, value):
		self.words[ident] = value

	def pushword(self, word, closure):
		if isinstance(word, Closure):
			self.call_stack.append(word)
			self.step_eval(word.node, closure)
			self.call_stack.pop()
		elif callable(word):
			word(self, closure)
		else:
			self.stack.append(word)
	
	def popvalue(self):
		if self.stack:
			return self.stack.pop()
		else:
			raise DejaStackEmpty(self)

	def makeword(self, word, closure):
		if isinstance(word, (Number, String)):
			return word.value
		elif isinstance(word, Ident):
			return getident(word.value)
		elif isinstance(word, ProperWord):
			return closure.getword(word.value)

	def step_eval(self, node, closure = None):
		if not closure or isinstance(node, (File, Statement)):
			closure = Closure(self, closure, node)

		if isinstance(node, (File, WordList, Clause)):
			for child in node.children:
				r = self.step_eval(child, closure)
				if r:
					return r
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
			try:
				return self.step_eval(node.body, closure)
			except DejaError as r:
				self.pushword(r.dj_info)
				self.pushword(getident(r.dj_str))
				self.call_stack = self.call_stack[:orig_callstack]
				return self.step_eval(node.errorhandler, closure)

def eval(node, env=None):
	if not env:
		env = Environment()
	try:
		env.step_eval(node)
	except DejaError as e:
		print(e)
