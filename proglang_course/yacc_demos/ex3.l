/* This lex specification file recognizes 
two valid tokens, '1' and '0'. The newline
character indicates the end of the input string 
and therefore returns 0. All other characters 
and invalid */

%{
   #include "y.tab.h"
%}
%%
1         {yylval.num = 1; return ONETOKEN;}
0         {yylval.num = 0; return ZEROTOKEN;}
\n        {return 0;}
.         { printf("Unknown character %s\n", yytext);}
%%
int yywrap(){
    return 1;
}
