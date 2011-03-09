from errors import *
from parse import *

from nodes import *
from ifelse import *
from loop import *
from func import *
from trycatch import *

from stdlib import stdlib

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
	def __repr__(self):
		return self.__str__()

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
			return self.parent.getword(ident)
		else:
			return self.env.getword(ident)

	def setlocal(self, ident, value):
		self.words[ident] = value

	def setword(self, ident, value):
		if ident in self.words:
			self.words[ident] = value
		elif self.parent:
			self.parent.setword(ident, value)
		else:
			self.env.setword(ident, value)

	def __str__(self):
		if hasattr(self.node, 'name'):
			return 'Func ' + self.node.name
		else:
			return 'Labda'

class Environment(object):
	types = {NumberObject: 'num', StringObject: 'str', Closure: 'func',
			IdentObject: 'ident', StackObject: 'stack'}

	def __init__(self):
		self.words = stdlib
		self.stack = []
		self.call_stack = []

		self.step_eval(parse_line("set '(' '('"))
		self.step_eval(parse_line("set ')' ')'"))
		self.step_eval(parse_line("set ']' ']'"))

	def gettype(self, word):
		if type(word) not in self.types:
			return 'func' # assume implementation function
		#	raise DejaError() #wrong type, should not happen
		return self.types[type(word)]

	def ensure(self, word, expected_type):
		if self.gettype(word) != expected_type:
			raise DejaTypeError(self, word, expected_type)
		return word

	def getword(self, word):
		if word in self.words:
			return self.words[word]
		else:
			raise DejaNameError(self, word)

	@staticmethod
	def getident(name):
		if name not in IdentObject.idents:
			return IdentObject(name)
		else:
			return IdentObject.idents[name]

	def setword(self, ident, value):
		self.words[ident] = value

	def pushvalue(self, value):
		self.stack.append(value)

	def pushword(self, word, closure):
		if isinstance(word, Closure):
			self.call_stack.append(word)
			for name in word.node.arguments.children:
				word.setlocal(name.value, self.popvalue())
			try:
				self.step_eval(word.node.body, word)
			except ReturnException:
				pass
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
			return self.getident(word.value)
		elif isinstance(word, ProperWord):
			return closure.getword(word.value)

	def getnextnode(self, node):
		if node.children:
			return node.children[0]
		return self.getnextupnode(node)

	def getnextupnode(self, node):
		while node.parent:
			p = node.parent
			if isinstance(node, ConditionClause):
				if isinstance(p, (WhileStatement, IfClause, ElseIfClause)):
					if self.popvalue():
						return p.body
				elif isinstance(p, IfClause):
					if self.popvalue():
						return p.body
					else:
						if p.parent.elseifclauses:
							return p.parent.elseifclauses[0]
						elif p.parent.elseclause:
							return p.parent.elseclause
					node = p.parent
					continue
				elif isinstance(p, ElseIfClause):
					if self.popvalue():
						return p.body
					node = p.parent
					continue
			elif isinstance(node, BodyClause):
				if isinstance(p, (WhileStatement, ForStatement)):
					return p.condition
				elif isinstance(p, (IfClause, ElseIfClause)):
					node = p.parent
					continue
				elif isinstance(p, LabdaStatement):
					return self.call_stack.pop()
			i = p.children.index(node)
			if i < len(p.children) - 1:
				next_node = p.children[i + 1]
				if isinstance(next_node, WhileStatement):
					if self.popvalue():
						return next_node.body
				elif isinstance(next_node, IfClause):
					if self.popvalue():
						return next_node.body
				elif not isinstance(next_node, (IfClause, ElseIfClause, ElseClause)):
					return next_node
			node = p

	def traceup(self, node): #look for an error handler
		while node:
			if isinstance(node, CatchStatement):
				return node.errorhandler
			node = node.parent

	def step_eval(self, node, closure = None):
		while node:
			if not closure or isinstance(node, (File, Statement)):
				closure = Closure(self, closure, node)

			if isinstance(node, Word):
				self.pushword(self.makeword(node, closure), closure)
				continue

			elif isinstance(node, IfStatement):
				self.step_eval(node.ifclause.conditionclause, closure)
				if self.popvalue():
					return self.step_eval(node.ifclause, closure)
				for elseif in node.elseifclauses:
					self.step_eval(elseif.conditionclause, closure)
					if self.popvalue():
						return self.step_eval(elseif, closure)
				if node.elseclause:
					return self.step_eval(node.elseclause, closure)

			elif isinstance(node, WhileStatement):
				self.step_eval(node.conditionclause, closure)
				while self.popvalue():
					self.step_eval(node.body, closure)
					self.step_eval(node.conditionclause, closure)

			elif isinstance(node, ForStatement):
				self.step_eval(node.forclause, closure)
				item = self.popvalue()
				info = self.popvalue()
				func = self.popvalue()
				while func:
					closure.setlocal(node.countername, item)
					self.step_eval(node.body, closure)
					self.pushvalue(info)
					if self.gettype(func) == 'ident':
						func = closure.getword(func.name)
					if callable(func):
						func(self, closure)
					else:
						self.step_eval(func, closure)
					item = self.popvalue()
					info = self.popvalue()
					func = self.popvalue()
				if info: # use-item
					closure.setlocal(node.countername, item)
					self.step_eval(node.body, closure)

			elif isinstance(node, LocalFuncStatement) and closure.parent:
				return closure.parent.setlocal(node.name, closure)

			elif isinstance(node, FuncStatement):
				return self.setword(node.name, closure)

			elif isinstance(node, LabdaStatement):
				self.stack.append(closure)

			elif isinstance(node, CatchStatement):
				orig_callstack = len(self.call_stack)
				try:
					return self.step_eval(node.body, closure)
				except DejaError as r:
					self.pushvalue(r.dj_info)
					self.pushvalue(self.getident(r.dj_str))
					self.call_stack = self.call_stack[:orig_callstack]
					return self.step_eval(node.errorhandler, closure)

			node = self.getnextnode(node)

def eval(node, env=None):
	if not env:
		env = Environment()
	try:
		env.step_eval(node)
	except ReturnException:
		pass
	except RuntimeError as e:
		print(e)
	except DejaError as e:
		print(e)
