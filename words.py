from nodes import *

class String(Word):
	def convert(self, value):
		return value[1:]

class Number(Word):
	def convert(self, value):
		return int(value)

class Ident(Word):
	def convert(self, value):
		return value[1:-1]

class ProperWord(Word):
	def convert(self, value):
		return value

