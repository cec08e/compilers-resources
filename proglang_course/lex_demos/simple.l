%{

   int numints = 0, numdoubles = 0, numids = 0, numops = 0, whitechars = 0;

%}

inttoken (-?[1-9][0-9]*)|0
real (-|{inttoken})?"."[0-9]+ 

%%
{inttoken}                {printf("Matched integer: %s\n", yytext); numints++;}
{real}  {printf("Matched real: %s\n", yytext); numdoubles++;}
[a-zA-Z][a-zA-Z_0-9]*     {printf("Matched identifier: %s \n", yytext); numids++;}
[+\-/*=]                  {printf("Matched operator: %s \n", yytext); numops++;}
[ \n\t]                   {whitechars++;}

%%

int main(){
    yylex();
    printf("Number of integer tokens: %d \n", numints);
    printf("Number of real tokens: %d \n", numdoubles);
    printf("Number of identifiers: %d \n", numids);
    printf("Number of operators: %d \n", numops);
    printf("Number of whitespace characters: %d \n", whitechars);
    return 0;
}
