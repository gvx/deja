from nodes import *

class IfStatement(Statement):
	def __init__(self, parent, linenr):
		Node.__init__(self, parent)
		self.linenr = linenr
		self.ifclause = None
		self.elseifclauses = []
		self.elseclause = None
	def addif(self, ifclause):
		#self.add(ifclause)
		self.ifclause = ifclause
	def addelseif(self, elseifclause):
		#self.add(elseifclause)
		self.elseifclauses.append(elseifclause)
	def addelse(self, elseclause):
		#self.add(elseclause)
		self.elseclause = elseclause

class IfClause(BodyClause):
	def __init__(self, parent, tokens):
		Node.__init__(self, None)
		parent.addif(self)
		self.conditionclause = ConditionClause(self, tokens)
		self.parent = parent

class ElseIfClause(BodyClause):
	def __init__(self, parent, tokens):
		Node.__init__(self, None)
		parent.addelseif(self)
		self.conditionclause = ConditionClause(self, tokens)
		self.parent = parent

class ElseClause(BodyClause):
	def __init__(self, parent):
		Node.__init__(self, None)
		parent.addelse(self)
		self.parent = parent
