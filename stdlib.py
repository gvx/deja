from errors import *

stdlib = {}

def add(f, *names):
	if isinstance(f, str):
		return lambda x: add(x, f, *names)
	if not names:
		names = [f.__name__]
	for name in names:
		stdlib[name] = f

@add('.')
def prints(env, closure):
	print(env.popvalue())

@add
def swap(env, closure):
	a,b = env.popvalue(), env.popvalue()
	env.pushvalue(a)
	env.pushvalue(b)

@add('(print-stack)')
def print_stack(env, closure):
	print(env.stack)

@add
def dup(env, closure):
	if not env.stack:
		raise DejaStackEmpty()
	env.pushvalue(env.stack[-1])

@add
def drop(env, closure):
	env.popvalue()

@add
def get(env, closure):
	env.pushvalue(closure.getword(env.popvalue()))

@add
def getglobal(env, closure):
	env.pushvalue(env.getword(env.popvalue()))

@add
def set(env, closure):
	ident = env.popvalue()
	value = env.popvalue()
	if not hasattr(ident, 'name'):
		raise DejaTypeError(env, ident, 'ident')
	env.setword(ident.name, value)

@add
def local(env, closure):
	ident = env.popvalue()
	value = env.popvalue()
	env.ensure(ident, 'ident')
	closure.setlocal(ident.name, value)

@add('type')
def gettype(env, closure):
	try:
		v = env.popvalue()
		env.ensure(v, 'ident')
		env.pushvalue(env.gettype(closure.getword(v)))
	except DejaNameError:
		env.pushvalue(env.getident('nil'))

