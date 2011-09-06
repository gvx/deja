from bytecode import *

if __name__ == '__main__':
	import sys
	if len(sys.argv) > 1:
		ifile = open(sys.argv)
		ofile = open('out.dvc', 'w')
	else:
		ifile = sys.stdin
		ofile = sys.stdout
	ofile.write(bytecode(collect(convert(flatten(parse(ifile.read()))))))
