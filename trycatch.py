from nodes import *

class CatchStatement(SimpleStatement):
	def __init__(self, parent, handler, linenr):
		SimpleStatement.__init__(self, parent, linenr)
		self.errorhandler = ErrorHandler(self, handler)

class ErrorHandler(Clause, WordList):
	def __init__(self, parent, tokens):
		WordList.__init__(self, parent, tokens)

