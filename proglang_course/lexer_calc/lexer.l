%{
    #include <stdlib.h>
    #include <string.h>
    int yylval;
    int line = 1, col = 1;
    char string_table[20000];
    int string_table_index = 0;
%}

%%

";"            {col += yyleng; return SEMInumber;}
"("            {col += yyleng; return LPARENnumber;} 
"-"            {col += yyleng; return MINUSnumber;} 
"*"            {col += yyleng; return TIMESnumber;} 
","            {col += yyleng; return COMMAnumber;} 
")"            {col += yyleng; return RPARENnumber;} 
"+"            {col += yyleng; return PLUSnumber;} 
"="            {col += yyleng; return EQnumber;} 
<<EOF>>        {col += yyleng; return EOFnumber;}
"begin"        {col += yyleng; return BEGINnumber;}
"program"      {col += yyleng; return PROGRAMnumber;}
"var"          {col += yyleng; return VARnumber;}
"end"          {col += yyleng; return ENDnumber;}
"is"           {col += yyleng; return ISnumber;}
"div"          {col += yyleng; return DIVnumber;}
"print"        {col += yyleng; return PRINTnumber;}
"(*"(.*)"*)"   {col += yyleng;}
([1-9][0-9]*)|0    {col += yyleng; yylval = atoi(yytext); return ICONSTnumber;}
[a-zA-Z]+[0-9]*    {col += yyleng; yylval = putstring(yytext, yyleng); return IDnumber;}
[ \t]          {col += yyleng;}
\n             {line++; col = 1;}           
.              {printf("Unknown character: %s. Line %d, column %d. \n", yytext, line, col); col += yyleng;}

%%

int putstring(char *image, int leng){
   strcpy(string_table+string_table_index, image);
   string_table_index = string_table_index + leng + 1;
   return string_table_index - (leng + 1);

}
