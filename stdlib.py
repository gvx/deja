from errors import *

stdlib = {}

def add(f, *names):
	if isinstance(f, str):
		return lambda x: add(x, f, *names)
	if not names:
		names = [f.__name__]
	for name in names:
		stdlib[name] = f
	return f

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
		env.pushvalue(env.getident(env.gettype(closure.getword(v.name))))
	except DejaNameError:
		env.pushvalue(env.getident('nil'))

@add('[]')
def newstack(env, closure):
	env.pushvalue([])

@add('push-to')
def push_to(env, closure):
	stack = env.popvalue()
	env.ensure(stack, 'stack')
	stack.append(env.popvalue())

@add('pop-from')
def pop_from(env, closure):
	stack = env.popvalue()
	env.ensure(stack, 'stack')
	if not stack:
		raise DejaEmptyStack()
	env.pushvalue(stack.pop())

@add
def call(env, closure):
	p = env.popvalue()
	if env.gettype(p) == 'ident':
		p = closure.getword(p.name)
	env.pushword(p, closure)

@add('error', 'raise')
def error(env, closure):
	sort = env.popvalue()
	env.ensure(sort, 'ident')
	raise DejaError(env, sort.name, env.popvalue())

@add('=', 'equal')
def equal(env, closure):
	env.pushvalue(env.popvalue() == env.popvalue())

@add('not')
def not_(env, closure):
	env.pushvalue(not env.popvalue() and 1 or 0)

@add('+', 'add')
def plus(env, closure):
	a = env.popvalue()
	env.ensure(a, 'num')
	b = env.popvalue()
	env.ensure(b, 'num')
	env.pushvalue(a + b)

@add('return')
def return_(env, closure):
	raise ReturnException

@add('[')
def stackify(env, closure):
	v = env.popvalue()
	acc = []
	while env.gettype(v) != 'ident' or v.name != ']':
		acc.insert(0, v)
		v = env.popvalue()
	env.pushvalue(acc)

@add('stop-iter')
def stop_iter(env, closure=False):
	env.pushvalue(0)
	env.pushvalue(closure is True)
	env.pushvalue(0)

@add('range')
def range_(env, closure):
	curr = env.popvalue()
	step = 1
	if isinstance(curr, list):
		step, stop, curr = curr
	else:
		stop = env.popvalue()
		step = 1
	if (step > 0 and curr >= stop) or (step < 0 and curr <= stop):
		stop_iter(env)
	else:
		env.pushvalue(env.getident('range'))
		env.pushvalue([step, stop, curr + step])
		env.pushvalue(curr)

