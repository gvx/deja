# Déjà Vu

It is a programming language. Stack-based. Semi-functional. Backwards.
Inspired by Python and Forth. I'm talking about Déjà Vu.

# Setting up the virtual machine

	$ cd vm/
	$ make release
	$ cd ..

# Running things

Assuming you have a module called `yourfile.deja`, you can compile and
run it with:

	$ python dvc.py yourfile.deja yourfile.vu
	$ vm/vu yourfile

# Examples

## Hello world

	print "Hello world!"

## Fibonacci

	fib a:
		if > 1 a:
			fib - 1 a
			fib - 2 a
			+
		elseif = a 1:
			1
		else:
			0

## Quine

	"print . dup"
	print . dup

## Fractions

	. * 1/3 3/2 # prints 1/2

# Grammar

If you know what [BNF](http://en.wikipedia.org/wiki/Backus%E2%80%93Naur_Form)
is, here's the one for Déjà Vu:

	Program ::= <Block>*

	Block ::= <Head> ":" <NewLine>+ (<Indentation> <Block>)+
	        | <Line> <NewLine>+
	        | <IfBlock>
	        | <TryBlock>

	IfBlock ::= "if" <Space> <Line> ":" <NewLine>+ (<Indentation> <Block>)+
	            ("elseif" <Space> <Line> ":" <NewLine>+ (<Indentation> <Block>)+)*
	            ("else" <Space>? ":" <NewLine>+ (<Indentation> <Block>)+)?

	TryBlock ::= "try" <Space>* ":" <NewLine>+ (<Indentation> <Block>)+
	             ("catch" <Space> <ProperWord> ":" <NewLine>+ (<Indentation> <Block>)+)*

	Head ::= "while" <Space> <Line>
	       | "for" <Space> <ProperWord> (<Space> <Line>)? <Space>?
	       | "repeat" <Space> <Line>
	       | "func" <Space> <ProperWord> <Arguments>
	       | "local" <Space> <ProperWord> <Arguments>
	       | "labda" <Arguments>
	       | <ProperWord> <Arguments>

	Indentation ::= <Tab>*

	Line ::= (<Word> (<Space> <Word>)*)? <Space>?

	Arguments ::= (<Space> <ProperWord>)* <Space>?

	Space ::= " "+

	Word ::= ProperWord
	       | Identity
	       | String
	       | Number
	       | Fraction

	Identity ::= ":" <ProperWord>?

	ProperWord ::= <Char>+

	String ::= '"' <StringChar>* '"'

	Number ::= '-'? <Digits> ('.' <Digits>)?

	Fraction ::= '-'? <Digits> '/' <Digits>

	Digits ::= ('0'..'9')+

	Char ::= any non-whitespace, non-# character

	StringChar ::= any character other than a double quote
