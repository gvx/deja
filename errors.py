class DejaError(Exception):
	dj_str = 'error'
	def __init__(self, env, dj_str=None, dj_info=None):
		self.env = env
		if dj_str:
			self.dj_str = dj_str
		self.dj_info = dj_info or ['%s:%d' % (x.node.getfile(), x.node.linenr) for x in env.call_stack]
	def __str__(self):
		return self.name + ':\n ' + '\n '.join(self.dj_info)

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
	def __init__(self, env, ident):
		DejaError.__init__(self, env)
		self.ident = ident
		self.dj_info.append(ident)

