%{
   #include "y.tab.h"
   int put_string(char *str, int len);
   extern int yyline, yycolumn;
   char string_table[20000];
   int string_table_index = 0;
%}

delim     [ \t]
ws        {delim}+
letter    [a-zA-Z]
digit     [0-9]
id        {letter}({letter}|{digit})*
number    {digit}+

%%
\n             {yyline++; yycolumn=1;}
print          {yycolumn += yyleng; return (PRINTNUM);}
program        {yycolumn += yyleng; return (PROGRAMNUM);}
is             {yycolumn += yyleng; return (ISNUM);}
begin          {yycolumn += yyleng; return (BEGINNUM);}
end            {yycolumn += yyleng; return (ENDNUM);}
var            {yycolumn += yyleng; return (VARNUM);}
{ws}           {yycolumn += yyleng;}
if             {yycolumn += yyleng; return (IFNUM);}
"("            {yycolumn += yyleng; return (LPARENNUM);}
")"            {yycolumn += yyleng; return (RPARENNUM);}
;              {yycolumn += yyleng; return (SEMINUM);}
,              {yycolumn += yyleng; return (COMMANUM);}
then           {yycolumn += yyleng; return (THENNUM);}
{id}           {yycolumn += yyleng;
                yylval.sv = put_string(yytext, yyleng); 
                return (IDNUM);}
{number}       {yylval.sv = atoi(yytext); yycolumn += yyleng; return (INTEGERCONST);}
\+            {yycolumn += yyleng;return (PLUSNUM);}
"*"           {yycolumn += yyleng;return (TIMESNUM);}
"="           {yycolumn += yyleng;return(EQUALNUM);}
.             { printf("Unknown character %s, line = %d, column = %d\n", yytext, yyline, yycolumn);yycolumn += yyleng;}

%%

#include <stdio.h>
#include <string.h>

int yywrap()
{  if (feof(stdin)) return 1;
   return 0;
}

int put_string(char *str, int len)
{
  sprintf(string_table+string_table_index, "%s", str);
  string_table_index += len+1;
  string_table[string_table_index-1] = '\0';
  return (string_table_index-len-1);
}







