from nodes import *

class WhileStatement(SimpleStatement):
	def __init__(self, parent, conditionclause, linenr):
		SimpleStatement.__init__(self, parent, linenr)
		self.conditionclause = ConditionClause(self, conditionclause)

class ForStatement(SimpleStatement):
	def __init__(self, parent, tokens, linenr):
		SimpleStatement.__init__(self, parent, linenr)
		self.countername = tokens[0]
		self.forclause = ForClause(self, tokens[1:])

class ForClause(ConditionClause):
	pass

class RepeatStatement(SimpleStatement):
	def __init__(self, parent, tokens, linenr):
		SimpleStatement.__init__(self, parent, linenr)
		self.forclause = ForClause(self, tokens)
