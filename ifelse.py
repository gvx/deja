from nodes import *

class IfStatement(Statement):
	def __init__(self, parent, linenr):
		Node.__init__(self, parent)
		self.linenr = linenr
		self.ifclause = None
		self.elseifclauses = []
		self.elseclause = None
	def addif(self, ifclause):
		self.ifclause = ifclause
	def addelseif(self, elseifclause):
		self.elseifclauses.append(elseifclause)
	def addelse(self, elseclause):
		self.elseclause = elseclause

class IfClause(BodyClause):
	def __init__(self, parent, tokens):
		Node.__init__(self, parent)
		parent.addif(self)
		self.conditionclause = ConditionClause(self, tokens)

class ElseIfClause(BodyClause):
	def __init__(self, parent, tokens):
		Node.__init__(self, parent)
		parent.addelseif(self)
		self.conditionclause = ConditionClause(self, tokens)

class ElseClause(BodyClause):
	def __init__(self, parent):
		Node.__init__(self, parent)
		parent.addelse(self)
