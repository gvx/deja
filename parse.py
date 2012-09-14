from context import *

def parse(filename):
	with open(filename) as f:
		filenode = File(filename)
		filecontext = FileContext(filenode)
		for i, line in enumerate(f):
			filecontext.addline(LineContext(line.rstrip('\n'), filename, i+1).process())
		return filenode
