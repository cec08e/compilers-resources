/* Caitlin Carnahan
 * Assignment 4: Calculator
 * To compiler: "make"
 * To run: "cexpr < test.txt > temp.out"
 */

%{
#include <stdio.h>
#include <string.h>

// Defines a min function for checking for overflows
#define min(a, b) (((a) < (b)) ? (a) : (b)) 

// Holds the value assigned to the char w/ ascii value index+97
int vars[26];
int i;
// Set if an overflow or divide by zero occurred
int ERR_FLAG;
// Buffer for output
char buffer[100];
int char_count;

%}

%union{
  int num;
  char comm;
}

%token <num> VAR
%token <num> NUM
%token <comm> COMM

%token ADD_A NEG_A MULT_A DIV_A MOD_A SHIFTL_A SHIFTR_A BAND_A BXOR_A BOR_A

%start commands
%type <num> command2 term expr1 expr2 expr3 expr4 expr5 factor1 factor2 factor3
%type <comm> command1

%%
commands:
        |	commands command1 
        |       commands command2               { /* Print buffer and reset flag and buffer */
                                                  printf("%s", buffer, char_count);
	                                          memset(buffer, '\0', sizeof(buffer));
                                                  ERR_FLAG = 0;}
	;

command1 :      COMM ';'                        { /* If 'c', clear array. If 'd', dump contents */
                                                  if($1 == 'c') 
                                                    memset(vars, 0, sizeof(vars));
                                                  else if($1 == 'd')
                                                    for(i = 0; i < 26; i++)
                                                      printf("%c: %d\n", i+97, vars[i]);
                                                }
         ;


command2 :	expr1 ';'     			{ /* Error flag checked on each command rule to see 
                                                     if the rest of the command should be carried out.
                                                     Otherwise, value is printed to buffer */
                                                  if(!ERR_FLAG){
                                                    $$ = $1;
                                                    char_count = sprintf(buffer, "%d\n", $1);}
                                                }
         |      term '=' command2               { /* No need to update buffer here */
                                                  if(!ERR_FLAG){
	                                            vars[$1-97] = $3; 
                                                    $$ = $3;}
                                                }
         |      term ADD_A command2             { /* Assignment operators that perform functions additionally
                                                     check for errors. If no error, they rewrite the buffer to
                                                     update the output */
                                                  if(!ERR_FLAG){
                                                    if(vars[$1-97] + $3 < min(vars[$1-97], $3))
					               yyerror("overflow");
                                                    else{                     
                                                       vars[$1-97] += $3;
                                                       memset(buffer, '\0', sizeof(buffer));
						       char_count = sprintf(buffer, "%d\n", vars[$1-97]);}}
	                                        }
         |      term NEG_A command2             { vars[$1-97] -= $3; 
                                                  memset(buffer, '\0', sizeof(buffer));
                                                  char_count = sprintf(buffer, "%d\n", vars[$1-97]);}
							 
         |      term MULT_A command2            { if(!ERR_FLAG){
	                                             if((vars[$1-97]*$3)/$3 != vars[$1-97])
						       yyerror("overflow"); 
                                                     else{
                                                       vars[$1-97] *= $3;
                                                       memset(buffer, '\0', sizeof(buffer));
						       char_count = sprintf(buffer, "%d\n", vars[$1-97]); }}}    
         |      term DIV_A command2             { if(!ERR_FLAG){
	                                            if($3 != 0){
	                                               vars[$1-97] /= $3;
						       memset(buffer, '\0', sizeof(buffer));
                                                       char_count = sprintf(buffer, "%d\n", vars[$1-97]);}
                                                    else
                                                       yyerror("dividebyzero");} 
                                                }
         |      term MOD_A command2             { if(!ERR_FLAG){
                                                    vars[$1-97] %= $3;
                                                    memset(buffer, '\0', sizeof(buffer));
                                                    char_count = sprintf(buffer, "%d\n", vars[$1-97]);} }
         |      term SHIFTL_A command2          { if(!ERR_FLAG){
                                                    vars[$1-97] <<= $3;
                                                    memset(buffer, '\0', sizeof(buffer));
                                                    char_count = sprintf(buffer, "%d\n", vars[$1-97]);}}
         |      term SHIFTR_A command2          { if(!ERR_FLAG){
                                                    vars[$1-97] >>= $3;
                                                    memset(buffer, '\0', sizeof(buffer));
                                                    char_count = sprintf(buffer, "%d\n", vars[$1-97]); }}
         |      term BAND_A command2            { if(!ERR_FLAG){
                                                    vars[$1-97] &= $3;
                                                    memset(buffer, '\0', sizeof(buffer));
                                                    char_count = sprintf(buffer, "%d\n", vars[$1-97]);} }
         |      term BXOR_A command2            { if(!ERR_FLAG){
                                                    vars[$1-97] ^= $3;
                                                    memset(buffer, '\0', sizeof(buffer));
                                                    char_count = sprintf(buffer, "%d\n", vars[$1-97]);} }
         |       term BOR_A command2            { if(!ERR_FLAG){
                                                    vars[$1-97] |= $3;
                                                    memset(buffer, '\0', sizeof(buffer));
                                                    char_count = sprintf(buffer, "%d\n", vars[$1-97]);} } 
	;

term    :       VAR                              { $$ = $1; }
        ;       

expr1   :       expr1 '|' expr2                 { $$ = $1 | $3; }
        |       expr2                           { $$ = $1; }
        ;

expr2   :       expr2 '^' expr3                 { $$ = $1 ^ $3; }
        |       expr3                           { $$ = $1; }
        ;

expr3   :       expr3 '&' expr4                 { $$ = $1 & $3; }
        |       expr4                           { $$ = $1; }
        ;

expr4	:	expr4 '+' expr5			{ if(($1+$3) < min($1,$3)){
                                                    ERR_FLAG = 1;
                                                    yyerror("overflow");}
                                                  else
                                                    $$ = $1 + $3; }
        |       expr4 '-' expr5                 { $$ = $1 - $3; }
        |       expr5                           { $$ = $1; }
 	;       

expr5   :       expr5 '*' factor1               { if($1 != 0 && $3 != 0 && ($1*$3)/$3 != $1){
                                                    ERR_FLAG = 1;
                                                    yyerror("overflow");}
                                                  else
                                                    $$ = $1 * $3; }
        |       expr5 '/' factor1               { if($3 == 0){
	                                            ERR_FLAG = 1;
	                                            yyerror("dividebyzero");}
	                                          else
                                                    $$ = $1 / $3; }
        |       expr5 '%' factor1               { $$ = $1 % $3; }
        |       factor1                         { $$ = $1; }
        ;

factor1 :       '-' factor2                     { $$ = -$2; }
        |       factor2                         { $$ = $1; }
        ;

factor2 :       '~' factor3                     { $$ = ~$2; }
        |       factor3                         { $$ = $1; }   
        ;

factor3 :       NUM                             { $$ = $1; }
        |       '(' expr1 ')'                   { $$ = $2; }
        |       term                            { $$ = vars[$1-97]; } 
        ;
%%

#include <stdio.h>

main()
{
   /* Initialization of variables */
   ERR_FLAG = 0;
   memset(vars, 0, sizeof(vars));
   memset(buffer, '\0', sizeof(buffer));
   char_count = 0;
   
   if (yyparse())
      printf("\nInvalid expression.\n");
   else
      printf("\nCalculator off.\n");
}

yyerror(s)
char *s;
{
   fprintf(stderr, "%s\n", s);
}

yywrap()
{
  return(1);
}
