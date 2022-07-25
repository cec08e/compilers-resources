%{
       #include <stdio.h>
       #include <string.h>
       #include <stdlib.h>
       char string_table[20000];
       /*void yyerror (char *s);*/
       int yyline = 1;
       int yycolumn = 1;

       struct Entry{
	 char id[50];
	 int value;
	 int init;
       };

       struct Entry symbol_table[100];
       int symbol_index = 0;
       int x;

       int check_for_dec(int);
       void print_header();
       void print_end();
       void print_exp(const char * );
	   
	 
       FILE *outfile;
%}

%union {
  int sv;
  struct {
    int v;
    char s[1000];
  } attr;
}


%token SEMInumber
%token LPARENnumber
%token RPARENnumber
%token <sv> ICONSTnumber
%token BEGINnumber
%token PROGRAMnumber
%token MINUSnumber
%token TIMESnumber
%token VARnumber
%token INTnumber
%token COMMAnumber
%token <sv> IDnumber
%token ENDnumber
%token ISnumber
%token PLUSnumber
%token DIVnumber
%token PRINTnumber
%token EQnumber

%type <attr> factor
%type <attr> statement_list
%type <attr> statement
%type <attr> declaration
%type <attr> id_list
%type <attr> exp
%type <attr> term

%left PLUSnumber MINUSnumber
%left DIVnumber TIMESnumber

%%

program: 				PROGRAMnumber  IDnumber ISnumber  compound_statement 
						;
compound_statement: 	BEGINnumber {print_header();}  statement_list ENDnumber {print_end();}
						;
statement_list: 	        statement {sprintf($<attr>$.s ,"%s;", $1.s); print_exp($1.s);} SEMInumber statement_list
                                             | statement   {sprintf($$.s ,"%s;", $1.s); print_exp($1.s);}
						;
statement:				IDnumber EQnumber exp {if((x = check_for_dec($1)) == -1){printf("Referencing undeclared variable on line %d.\n", yyline); return 1;}else{symbol_table[x].init = 1; symbol_table[x].value = $3.v;} sprintf($$.s ,"%s=%s", string_table+$1, $3.s);}
                                               | PRINTnumber exp { sprintf($$.s ,"cout << %s << endl", $2.s); printf("%d\n", $2.v);}
						| declaration { sprintf($$.s ,"%s", $1.s);}
						;
declaration:			VARnumber id_list { sprintf($$.s ,"int %s", $2.s);}
						;
id_list:				   id_list COMMAnumber IDnumber {if(check_for_dec($3) >= 0){printf("Duplicate declaration on line %d.\n", yyline); return 1;} sprintf($$.s ,"%s,%s", $1.s, string_table+$3);}
                                           | IDnumber {if(check_for_dec($1) >= 0){printf("Duplicate declaration on line %d.\n", yyline); return 1;}  sprintf($$.s ,"%s", string_table+$1);}
						;
exp:			        exp PLUSnumber term { $$.v = $1.v+$3.v; sprintf($$.s ,"%s+%s", $1.s, $3.s);}
                                                | exp MINUSnumber term { $$.v = $1.v-$3.v; sprintf($$.s ,"%s-%s", $1.s, $3.s);}
                                                | term { $$.v = $1.v; sprintf($$.s ,"%s", $1.s);}
                                                | MINUSnumber term { $$.v = -1*$2.v; sprintf($$.s ,"-%s", $2.s);}

						;
term:					factor { $$.v = $1.v; sprintf($$.s ,"%s", $1.s);}
                                                | term TIMESnumber factor { $$.v = $1.v*$3.v; sprintf($$.s ,"%s*%s", $1.s, $3.s);}
                                                | term DIVnumber factor {if($3.v == 0){printf("Dividing by zero on line %d.\n", yyline); return 1;} $$.v = $1.v/$3.v; sprintf($$.s ,"%s/%s", $1.s, $3.s);}
						;
factor:					ICONSTnumber      {$$.v = $1; sprintf($$.s, "%d", $1);}
                                                | IDnumber  {x = check_for_init($1); if(x != 0 ){return 1;}else{$$.v = find_value($1);} strcpy($$.s , string_table+$1);}
                                                | LPARENnumber exp RPARENnumber {$$.v = $2.v; sprintf($$.s ,"(%s)", $2.s);}
						;
%%

void print_header()
{
if ((outfile = fopen("mya.cpp", "w")) == NULL) {
printf("Can't open file mya.cpp.\n");
exit(0);
}

fprintf(outfile, "#include <iostream>\n");
fprintf(outfile, "#include <stdio.h>\n");
fprintf(outfile, "using namespace std;\n");
fprintf(outfile, "\nint main()\n");
fprintf(outfile, "{\n");
}

void print_end()
{
  fprintf(outfile, "}\n");
  fclose(outfile);
}

void print_exp(const char * s)
{
  fprintf(outfile, "%s;\n", s);
}

int check_for_dec(int string_index){
  int i;
  for(i=0; i < symbol_index; i++){
    if(strcmp(symbol_table[i].id, string_table+string_index) == 0){
      return i;
    }
  }

  sprintf(symbol_table[symbol_index].id, "%s", string_table+string_index);
  symbol_table[symbol_index].init = 0;
  symbol_index++;
  return -1; 
}

int find_value(int string_index){

  int i;  
  for(i=0; i < symbol_index; i++){
    if(strcmp(symbol_table[i].id, string_table+string_index) == 0){
      if(symbol_table[i].init){
        return symbol_table[i].value;
     }
   }
  }

  return 1;

}

int check_for_init(int string_index){
  int i;
  for(i=0; i < symbol_index; i++){
    if(strcmp(symbol_table[i].id, string_table+string_index) == 0){
      if(symbol_table[i].init){
         return 0;
      }
      else{
        printf("Referencing an uninitialized variable on line %d. \n", yyline);
        return -1;
      }
    }
  }
  printf("Referencing an undeclared variable on line %d. \n", yyline);
  return 1;

}

void yyerror(char *s) {
  fprintf(stderr, "line %d: %s\n", yyline, s);
}

int main()
{
  return yyparse();
	 
} 
