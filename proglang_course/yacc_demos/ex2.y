/* This yacc specification file recognizes
 and performs the semantic actions of the 
 attribute grammar presented in slide 7 of
 lecture 11.*/

%{
       #include <stdio.h>
%}

%union{
  int num;
  int st;
}

%token ATOKEN
%token BTOKEN
%token CTOKEN

%type <num> A
%type <num> B
%type <num> C
%type <st> P
%%

P:   A B C         {if(($1 == $2)&&($1 == $3)) $$ = 1; else $$ = 0;
                    printf("Value of P.st: %d\n", $$);}
     ;
A:   A ATOKEN      {$$ = $1 + 1;}
     | ATOKEN      {$$ = 1;}
     ;
B:   B BTOKEN      {$$ = $1 + 1;}
     | BTOKEN      {$$ = 1;}
     ;
C:   C CTOKEN      {$$ = $1 + 1;}
     | CTOKEN      {$$ = 1;}
     ;
%%
void yyerror(const char *str)
{
    printf("yyerror: %s \n", str);
}

int main()
{
  return yyparse();
	 
} 
