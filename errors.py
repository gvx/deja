class DejaError(Exception):
	pass

class DejaSyntaxError(DejaError):
	def __init__(self, error, context, index):
		self.error = error
		self.context = context
		self.index = index
	def __str__(self):
		return "Syntax error:\n %s:%d: %s\n %s\n %s" % (self.context.filename, self.context.linenr, self.error, self.context.origtext, '\t' * self.context.indent + ' ' * self.index + '^')

