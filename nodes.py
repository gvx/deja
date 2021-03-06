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
	def getfile(self):
		f = self
		while not isinstance(f, File):
			f = f.parent
		return f

class File(Node):
	def __init__(self, filename):
		Node.__init__(self, None)
		self.filename = filename

class Word(Node):
	def __init__(self, parent, value):
		Node.__init__(self, parent)
		self.value = self.convert(value)
	def convert(self, value):
		raise NotImplementedError
	def tostr(self, indent, first=False):
		return (not first and ' ' * indent or '') + str(self)
	def __str__(self):
		return self.__class__.__name__ + '(' + str(self.value) + ')'

class String(Word):
	def __init__(self, parent, value):
		value.decode('utf-8')
		Word.__init__(self, parent, value)
	def convert(self, value):
		return value[1:]
	def __str__(self):
		return '"' + self.value + '"'

class Number(Word):
	def convert(self, value):
		return float(value)

class Ident(Word):
	def convert(self, value):
		return value[1:]
	def __str__(self):
		return ":" + self.value

class ProperWord(Word):
	def convert(self, value):
		return value

class Fraction(Word):
	def convert(self, value):
		n, d = value.split('/')
		return int(n), int(d)

class WordList(Node):
	def __init__(self, parent, tokens):
		Node.__init__(self, parent)
		for token in reversed(tokens):
			self.gettokenclass(token)(self, token)
	@staticmethod
	def gettokentype(token):
		if token.startswith('"'):
			return 'str'
		elif token.startswith(":"):
			return 'ident'
		elif token.count('.') < 2 and (token.replace('.', '').isdigit() or token.startswith('-') and token[1:].replace('.', '').isdigit()):
			return 'num'
		elif token.count('/') == 1:
			n, d = token.split('/')
			if (n.isdigit() or (n.startswith('-') and n[1:].isdigit())) and d.isdigit():
				return 'frac'
		return 'word'
	@classmethod
	def gettokenclass(cls, token):
		t = cls.gettokentype(token)
		if t == 'str':
			return String
		elif t == 'num':
			return Number
		elif t == 'word':
			return ProperWord
		elif t == 'ident':
			return Ident
		elif t == 'frac':
			return Fraction

class Line(WordList):
	def __init__(self, parent, tokens, linenr):
		WordList.__init__(self, parent, tokens)
		self.linenr = linenr

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
		WordList.__init__(self, None, tokens)
		self.parent = parent

class BodyClause(Clause):
	def __init__(self, parent):
		Node.__init__(self, parent)
		parent.addbody(self)
