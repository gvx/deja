from bytecode import *

if __name__ == '__main__':
	import sys
	if len(sys.argv) > 1:
		#print flatten(parse(sys.argv[1]))
		sys.stdout.write(write_bytecode(collect(convert(flatten(parse(sys.argv[1]))))))
