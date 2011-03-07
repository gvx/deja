# Déjà Vu

It is a programming language. Stack-based. Semi-functional. Backwards.
Inspired by Python and Forth. I'm talking about Déjà Vu.

# Example

Sure, why not:

	fib a:
		if > 1 a:
			fib - 1 a
			fib - 2 a
			+
		elseif = a 1:
			1
		else:
			0

# Grammar

If you know what [BNF](http://en.wikipedia.org/wiki/Backus%E2%80%93Naur_Form)
is, here's the one for Déjà Vu:

	Program ::= <Block>*

	Block ::= <Head> ":" <NewLine>+ (<Indentation> <Block>)+
	        | <Line> <NewLine>+
	        | <IfBlock>

	IfBlock ::= "if" <Space> <Line> ":" <NewLine>+ (<Indentation> <Block>)+
	            ("elseif" <Space> <Line> ":" <NewLine>+ (<Indentation> <Block>)+)*
	            ("else" <Space>? ":" <NewLine>+ (<Indentation> <Block>)+)?

	Head ::= "while" <Space> <Line>
	       | "for" <Space> <ProperWord> (<Space> <Line>)? <Space>?
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

	Identity ::= "'" <ProperWord>? "'"

	ProperWord ::= <Char>+

	String :: '"' <StringChar>* '"'

	Char ::= any non-whitespace character

	StringChar ::= any character other than a double quote
