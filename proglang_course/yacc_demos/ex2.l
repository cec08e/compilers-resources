/* This lex specification file accepts only 'a', 'b', and 'c'
 as valid tokens. Newline characters are used to indicate
 the end of the token stream (will return 0 to yacc).
 Any other character is invalid.  */

%{
   #include "y.tab.h"
%}
%%
a         {return ATOKEN;}
b         {return BTOKEN;}
c         {return CTOKEN;}
\n        {return 0;}
.         { printf("Unknown character %s\n", yytext);}
%%
int yywrap(){
    return 1;
}
