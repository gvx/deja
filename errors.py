class DejaError(Exception):
	dj_str = 'error'
	def __init__(self, env, node):
		self.env = env
		self.node = node
	def __str__(self):
		return self.name + ':\n ' + '\n '.join('%s:%d\n  %s' % (x.node.getfile(), x.node.linenr, '???') for x in self.env.call_stack)

class DejaSyntaxError(DejaError):
	dj_str = 'syntax-error'
	def __init__(self, error, context, index):
		self.error = error
		self.context = context
		self.index = index
	def __str__(self):
		return "Syntax error:\n %s:%d: %s\n %s\n %s" % (self.context.filename, self.context.linenr, self.error, self.context.origtext, '\t' * self.context.indent + ' ' * self.index + '^')

class DejaStackEmpty(DejaError):
	dj_str = 'empty-stack'
	name = 'Stack empty'

class DejaNameError(DejaError):
	dj_str = 'name-error'
	name = 'Name error'
