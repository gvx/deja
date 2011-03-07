from nodes import *

class LabdaStatement(SimpleStatement):
	def __init__(self, parent, arguments, linenr):
		SimpleStatement.__init__(self, parent, linenr)
		self.arguments = ArgumentList(self, arguments)

class FuncStatement(LabdaStatement):
	def __init__(self, parent, arguments, linenr):
		LabdaStatement.__init__(self, parent, arguments[1:], linenr)
		self.name = arguments[0]

class LocalFuncStatement(FuncStatement):
	def __init__(self, parent, arguments, linenr):
		FuncStatement.__init__(self, parent, arguments, linenr)

class ArgumentList(Clause):
	def __init__(self, parent, tokens):
		Node.__init__(self, None)
		self.parent = parent
		for token in tokens:
			WordList.gettokenclass(token)(self, token)
