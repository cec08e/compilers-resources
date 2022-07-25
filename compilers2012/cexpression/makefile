cexpr: y.tab.o lex.yy.o
	gcc -o cexpr y.tab.o lex.yy.o -ly -ll

y.tab.o: y.tab.c
	gcc -c y.tab.c

lex.yy.o: lex.yy.c y.tab.h
	gcc -c lex.yy.c

y.tab.c y.tab.h: cexpr.y
	yacc -d -v cexpr.y

lex.yy.c: scan.l
	lex scan.l

clean:
	-rm -f *.o lex.yy.c *.tab.* cexpr
