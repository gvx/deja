from errors import *

from nodes import *
from ifelse import *
from loop import *
from func import *
from trycatch import *
from words import *

from context import *

filename = 'test'

with open(filename) as f:
	try:
		filenode = File(filename)
		filecontext = FileContext(filenode)
		for i, line in enumerate(f):
			linecon = LineContext(line.rstrip('\n'), filename, i+1).indent().stringify().decomment().wordify().statementize()
			filecontext.addline(linecon)
		print(filenode)
	except DejaError as e:
		print(e)
