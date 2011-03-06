from context import *

def parse(filename):
	with open(filename) as f:
		try:
			filenode = File(filename)
			filecontext = FileContext(filenode)
			for i, line in enumerate(f):
				linecon = LineContext(line.rstrip('\n'), filename, i+1).indent().stringify().decomment().wordify().statementize()
				filecontext.addline(linecon)
			return filenode
		except DejaError as e:
			print(e)

if __name__ == '__main__':
	import sys
	if len(sys.argv) > 1:
		print(parse(sys.argv[1]))
