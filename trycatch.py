from nodes import *

class TryStatement(Statement):
	def __init__(self, parent, linenr):
		Node.__init__(self, parent)
		self.linenr = linenr
		self.tryclause = None
		self.catchclauses = []
	def addtry(self, tryclause):
		self.tryclause = tryclause
	def addcatch(self, catchclause):
		self.catchclauses.append(catchclause)

class TryClause(BodyClause):
	def __init__(self, parent):
		Node.__init__(self, parent)
		parent.addtry(self)

class CatchClause(BodyClause):
	def __init__(self, parent, tokens):
		Node.__init__(self, parent)
		parent.addcatch(self)
		self.exceptions = tokens
