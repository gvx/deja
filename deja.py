from parse import *
from eval import *
import sys

if len(sys.argv) > 1:
	eval(parse(sys.argv[1]))
else:
	env = Environment()
	for tree in parse_interactive():
		eval(tree, env)
