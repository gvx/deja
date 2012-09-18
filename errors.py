class DejaSyntaxError(Exception):
	def __init__(self, error, context=None, index=None):
		self.error = error
		self.context = context
		self.index = index
	def __str__(self):
		if self.context:
			return "Syntax error:\n %s:%d: %s\n %s\n %s" % (self.context.filename, self.context.linenr, self.error, self.context.origtext, '\t' * self.context.indent + ' ' * self.index + '^')
		else:
			return "Syntax error:\n %s\n" % (self.error,)
