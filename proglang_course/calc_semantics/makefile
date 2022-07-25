LEX = flex
YACC = yacc
CC = gcc

a.out: y.tab.c lex.yy.c
	$(CC) lex.yy.c y.tab.c -lfl
lex.yy.c: lexer.l y.tab.h
	$(LEX) -i lexer.l
y.tab.c: calc.y
	$(YACC) -d calc.y
clean:
	rm -f *.o lex.yy.c *.tab.* a.out *.output
