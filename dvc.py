from __future__ import print_function
from bytecode import *

if __name__ == '__main__':
	import sys
	if len(sys.argv) > 1:
		try:
			sys.stdout.write(write_bytecode(collect(refine(optimize(convert(sys.argv[1], flatten(parse(sys.argv[1]))))))))
		except DejaSyntaxError as e:
			print(e, file=sys.stderr)
