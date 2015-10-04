all: tokenize parse build

tokenize:
	flex tokenizer.l

parse:
	bison -d parser.y

build:
	gcc lex.yy.c parser.c parser.tab.c -o parser

clean:
	rm parser.tab.* *.dot *.png lex.yy.c parser