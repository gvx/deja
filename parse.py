from context import *

def parse_line(line):
	filename = 'line: '  + line
	filenode = File(filename)
	filecontext = FileContext(filenode)
	filecontext.addline(LineContext(line.rstrip('\n'), filename, 0).process())
	if filecontext.has_statement: # doesn't work on statements
		return False
	return filenode

def parse(filename):
	with open(filename) as f:
		try:
			filenode = File(filename)
			filecontext = FileContext(filenode)
			for i, line in enumerate(f):
				filecontext.addline(LineContext(line.rstrip('\n'), filename, i+1).process())
			return filenode
		except DejaError as e:
			print(e)

def parse_interactive(filename='(interactive)'):
	try:
		import readline
	except ImportError:
		pass
	l = 1
	try:
		while True:
			try:
				inp = raw_input(">> ").rstrip()
				if inp:
					i = 1
					filenode = File(filename)
					filecontext = FileContext(filenode)
					#filecontext.indentation_stack = [filenode, body] #just enforce it, because you never know
					filecontext.addline(LineContext(inp, filename, l).process())
					while inp and (filecontext.has_statement or filecontext.last_indent):
						inp = raw_input(".. ").rstrip()
						i += 1
						filecontext.addline(LineContext(inp, filename+':'+str(l), i).process())
					yield filenode
			except KeyboardInterrupt:
				print
			except DejaError as e:
				print(e)
			l += 1
	except EOFError:
		print
		return

if __name__ == '__main__':
	import sys
	if len(sys.argv) > 1:
		print(parse(sys.argv[1]))
