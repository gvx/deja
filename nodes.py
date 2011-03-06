class Node(object):
	def __init__(self, parent):
		self.parent = parent
		if parent:
			parent.add(self)
		self.children = []
	def add(self, child):
		self.children.append(child)
	def tostr(self, indent, first=False):
		istr = not first and ' ' * indent or ''
		n = self.__class__.__name__
		if not self.children:
			return istr + n
		if len(self.children) == 1:
			return istr + n + '[' + self.children[0].tostr(indent + 1 + len(n), first=True) + ']'
		return istr + n + '[' + self.children[0].tostr(indent + 1 + len(n), first=True) + '\n' + '\n'.join(x.tostr(indent + 1 + len(n)) for x in self.children[1:]) + '\n' + ' ' * indent + ']'
	def __str__(self):
		return self.tostr(0)
		#return self.__class__.__name__ + '[' + ','.join(str(x) for x in self.children) + ']'

class File(Node):
	def __init__(self, filename):
		Node.__init__(self, None)
		self.filename = filename

class WordList(Node):
	def __init__(self, parent, tokens):
		Node.__init__(self, parent)
		self.tokens = tokens

class Line(WordList):
	def __init__(self, parent, linecontext):
		WordList.__init__(self, parent, linecontext.tokens)
		self.linenr = linecontext.linenr

class Statement(Node):
	def __init__(self, parent, linenr):
		Node.__init__(self, parent)
		self.linenr = linenr

class SimpleStatement(Statement):
	def __init__(self, parent, linenr):
		Statement.__init__(self, parent, linenr)
		self.body = None
	def addbody(self, body):
		self.add(body)
		self.body = body


class Clause(Node):
	pass

class ConditionClause(Clause, WordList):
	def __init__(self, parent, tokens):
		WordList.__init__(self, parent, tokens)

class BodyClause(Clause):
	def __init__(self, parent):
		Node.__init__(self, None)
		parent.addbody(self)
		self.parent = parent

class Word(Node):
	def __init__(self, parent, value):
		Node.__init__(self, parent)
		self.value = self.convert(value)
	def convert(self, value):
		raise NotImplementedError

