from errors import *

from nodes import *
from ifelse import *
from loop import *
from func import *
from trycatch import *

STATEMENTS = set('func labda local if elseif else while for try catch repeat'.split())
STATEMENT_CLASS = {'func': FuncStatement, 'labda': LabdaStatement,
	'local': LocalFuncStatement, 'while': WhileStatement,
	'for': ForStatement,
	'repeat': RepeatStatement
}

class Context(object):
	pass

class LineContext(Context):
	def __init__(self, text, filename, linenr):
		self.text = text
		self.origtext = text
		self.filename = filename
		self.linenr = linenr

	def indent(self):
		self.text = self.text.replace('    ', '\t').lstrip('\t')
		self.indent = len(self.origtext) - len(self.text)
		return self

	def stringify(self):
		self.stringwise = self.text.split('"')
		return self

	def decomment(self):
		for i, s in enumerate(self.stringwise):
			if i % 2 == 0:
				if '#' in s: #found comment!
					self.stringwise = self.stringwise[:i]
					self.stringwise.append(s.split('#', 1)[0])
					break
		if len(self.stringwise) % 2 == 0:
			raise DejaSyntaxError("Unclosed string", self, self.text.rindex('"'))
		return self

	def wordify(self):
		self.tokens = []
		for i, s in enumerate(self.stringwise):
			if i % 2:
				self.tokens.append('"' + s)
			else:
				tokens = s.split()
				for i, token in enumerate(tokens):
					if token.startswith('@') and len(token) > 1:
						tokens[i] = ":%s" % token[1:]
						tokens.insert(i, 'get')
				self.tokens.extend(tokens)
		return self

	def statementize(self):
		self.statement = None
		if self.tokens and not self.tokens[-1].startswith('"') and self.tokens[-1].endswith(':'):
			self.tokens[-1] = self.tokens[-1][:-1]
			if not self.tokens[-1]: #remove last word if empty
				self.tokens.pop()
				if not self.tokens:
					raise DejaSyntaxError("Empty statement", self, self.text.index(':'))
			if self.tokens[0].startswith('"'):
				raise DejaSyntaxError("Statement starting with a string", self, self.text.index('"'))
			if self.tokens[0] not in STATEMENTS:
				self.statement = 'func'
			else:
				self.statement = self.tokens.pop(0)
			getattr(self, 'assert_' + self.statement)()
		return self

	def assert_labda(self):
		for token in self.tokens:
			if WordList.gettokentype(token) != 'word':
				raise DejaSyntaxError("Function definition containing wrong type of word", self, self.text.index(token))

	def assert_func(self):
		if not self.tokens:
			raise DejaSyntaxError("Missing function name", self, self.text.index(':'))
		self.assert_labda()

	assert_local = assert_func

	def assert_for(self):
		if not self.tokens:
			raise DejaSyntaxError("Missing counter name", self, self.text.index(':'))

	def assert_if(self):
		pass

	def assert_while(self):
		pass

	def assert_repeat(self):
		pass

	def assert_try(self):
		if self.tokens:
			raise DejaSyntaxError("A try statement cannot have other words", self, self.text.index('try') + 4)

	def assert_catch(self):
		if not self.tokens:
			raise DejaSyntaxError("Missing exception name", self, self.text.index(':'))
		for ex in self.tokens:
			if WordList.gettokentype(ex) != 'word':
				raise DejaSyntaxError("Error category list must consist of proper words", self, self.text.index(ex))


	def assert_elseif(self):
		pass #needs to follow an if or another elseif

	def assert_else(self):
		#needs to follow an if or elseif
		if self.tokens:
			raise DejaSyntaxError("An else statement cannot have other words", self, self.text.index(self.tokens[0]))

	def process(self):
		return self.indent().stringify().decomment().wordify().statementize()

class FileContext(Context):
	def __init__(self, filenode):
		self.filenode = filenode
		self.last_node = filenode
		self.last_indent = 0
		self.has_statement = False
		self.indentation_stack = [filenode]

	def addline(self, linecontext):
		if not linecontext.tokens and not linecontext.statement:
			return #skip empty lines

		if self.has_statement:
			if linecontext.indent != self.last_indent + 1:
				raise DejaSyntaxError("Expected a single extra indentation", linecontext, 0)
		else:
			if linecontext.indent > self.last_indent:
				raise DejaSyntaxError("Expected no extra indentation", linecontext, 0)

		self.last_indent = linecontext.indent
		self.has_statement = linecontext.statement is not None

		if self.has_statement:
			st = linecontext.statement
			if st == 'if':
				self.last_node = IfClause(IfStatement(self.indentation_stack[self.last_indent], linecontext.linenr), linecontext.tokens)
			elif st == 'elseif':
				if len(self.indentation_stack) <= self.last_indent + 1 or not isinstance(self.indentation_stack[self.last_indent + 1], (IfClause, ElseIfClause)):
					raise DejaSyntaxError("No if clause or elseif clause preceding elseif clause", linecontext, 0)
				self.last_node = ElseIfClause(self.indentation_stack[self.last_indent + 1].parent, linecontext.tokens)
			elif st == 'else':
				if len(self.indentation_stack) <= self.last_indent + 1 or not isinstance(self.indentation_stack[self.last_indent + 1], (IfClause, ElseIfClause)):
					raise DejaSyntaxError("No if clause or elseif clause preceding else clause", linecontext, 0)
				self.last_node = ElseClause(self.indentation_stack[self.last_indent + 1].parent)
			elif st == 'try':
				self.last_node = TryClause(TryStatement(self.indentation_stack[self.last_indent], linecontext.linenr))
			elif st == 'catch':
				if len(self.indentation_stack) <= self.last_indent + 1 or not isinstance(self.indentation_stack[self.last_indent + 1], (TryClause, CatchClause)):
					raise DejaSyntaxError("No try clause preceding catch clause", linecontext, 0)
				self.last_node = CatchClause(self.indentation_stack[self.last_indent + 1].parent, linecontext.tokens)
			else:
				self.last_node = BodyClause(STATEMENT_CLASS[st](self.indentation_stack[self.last_indent], linecontext.tokens, linecontext.linenr))
			self.indentation_stack = self.indentation_stack[:self.last_indent + 1]
			self.indentation_stack.append(self.last_node)
		else:
			try:
				self.last_node = Line(self.indentation_stack[self.last_indent], linecontext.tokens, linecontext.linenr)
			except UnicodeDecodeError as e:
				raise DejaSyntaxError("Encoding error: all strings need to be UTF-8", linecontext, linecontext.text.index(e.object))
