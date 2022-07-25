/* Caitlin Carnahan
 * Assignment 4: Calculator
 * To compile: "make"
 * To run: "cexpr < test.txt > tmp.out"
 */
%{
#include <stdio.h>
#include <stdlib.h>
#include "y.tab.h"
%}
%%
[ \n\t]+        { /* Consumes whitespace */ ; }
dump            { /* Returns COMM for dump command */
                  yylval.comm = yytext[0];
                  return COMM; }
clear           { /* Returns COMM for clear command */
                  yylval.comm = yytext[0]; 
                  return COMM; }
[0-9]+		{ /* Returns NUM for number */
                  sscanf(yytext, "%d", &yylval.num);
		  return NUM; }
[a-z]           { /* Returns VAR for variable */
                  yylval.num = yytext[0];
                  return VAR; }
\+=             { /* The following return tokens for multi-char operators */
                  return ADD_A; } 
\-=             { return NEG_A; }
\*=             { return MULT_A; }
\/=             { return DIV_A; }
%=              { return MOD_A; }
\<\<=           { return SHIFTL_A; }
\>\>=           { return SHIFTR_A; }
\&=             { return BAND_A; }
\^=             { return BXOR_A; }
\|=             { return BOR_A; }
.		{ /* Returns picked-up symbol otherwise */
                  return yytext[0]; }