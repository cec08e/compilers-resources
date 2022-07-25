/* This yacc specification file recognizes
and performs the semantic actions associated
with the attribute grammar presented in slide
13 of lecture 11. */


%{
       #include <stdio.h>
%}

%union{
  int num;
  struct{
    int order;
    int val;
  }attr;
}

%token <num> ONETOKEN
%token <num> ZEROTOKEN

%type <attr> W
%type <num> B
%%

P:   W               {printf("%d\n", $1.val);}
     ;
W:   B W             {$$.val = $2.val + ($1*$2.order*2);
                      $$.order = $2.order*2; }
     | B             {$$.val = $1; $$.order = 1;}
     ;
B:   ONETOKEN        {$$ = $1;}
     | ZEROTOKEN      {$$ = $1;}
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
