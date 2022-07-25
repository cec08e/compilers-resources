%{
   #include <string.h>
   #include <stdio.h>
   #include "y.tab.h"

   int put_string(char *str, int len);
   extern int yyline, yycolumn;
   int string_table_index = 0;
   extern char string_table[20000];
%}

%%
\n      {yyline++; yycolumn=1;}
[0-9]+  {yylval.sv = atoi(yytext); yycolumn += yyleng; return(ICONSTnumber);}
print   {yycolumn += yyleng; return(PRINTnumber);}
program {yycolumn += yyleng; return(PROGRAMnumber);}
is      {yycolumn += yyleng; return(ISnumber);}
begin   {yycolumn += yyleng; return(BEGINnumber);}
end     {yycolumn += yyleng; return(ENDnumber);}
var     {yycolumn += yyleng; return(VARnumber);}
[ \t]   {yycolumn += yyleng;}
div     {yycolumn += yyleng; return(DIVnumber);}
";"       { yycolumn += yyleng; return(SEMInumber);}
"("     {yycolumn += yyleng; return(LPARENnumber);}
"-"     {yycolumn += yyleng; return(MINUSnumber);}
"*"     {yycolumn += yyleng; return(TIMESnumber);}
","     {yycolumn += yyleng; return(COMMAnumber);}
")"     {yycolumn += yyleng; return(RPARENnumber);}
"+"     {yycolumn += yyleng; return(PLUSnumber);}
"="     {yycolumn += yyleng; return(EQnumber);}
"(*".*"*)" {yycolumn += yyleng;}
[a-zA-Z]+[0-9]* {yycolumn += yyleng; yylval.sv = put_string(yytext, yyleng);return(IDnumber);}
.       {printf("Unknown character %s, line = %d, column = %d\n", yytext, yyline, yycolumn); yycolumn += yyleng;}
%%

int put_string(char *str, int len)
{
    char buf[2000];
    strncpy(buf, str, len);
    buf[len] = '\0';
    sprintf(string_table+string_table_index, "%s", buf);
    string_table_index += strlen(buf)+1;
    string_table[string_table_index-1] = '\0';
    return (string_table_index-strlen(buf)-1);
}
