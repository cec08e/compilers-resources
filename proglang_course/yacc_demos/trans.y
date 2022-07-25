%{
  #include <stdio.h>
  #include <stdlib.h>
  FILE *outfile;
  int yyline = 1; 
  int yycolumn = 1;

  void print_header();
  void print_end();
  void print_exp(const char*);

%}

%union {
  int sv;
  struct {
    int v;
    char s[1000];
  } attr;
}

%token   EOFnumber             
%token   STRINGCONST           
%token   <sv> INTEGERCONST      
%token   CHARCONST             
%token   PLUSNUM               
%token   TIMESNUM              
%token   IFNUM                 
%token   THENNUM               
%token   ELSENUM               
%token   EQUALNUM              
%token   IDNUM                 
%token   LPARENNUM             
%token   RPARENNUM             
%token   SEMINUM               
%token   BEGINNUM              
%token   ENDNUM                
%token   PRINTNUM
%token   PROGRAMNUM
%token   ISNUM
%token   VARNUM
%token   COMMANUM

%type <attr> exp
%type <attr> term
%type <attr> item

%% 

program: BEGINNUM {print_header();} explist ENDNUM {print_end();}
         ;

explist: exp {printf("%d\n",  $1.v); print_exp($1.s);}
         | exp {printf("%d\n",  $1.v); print_exp($1.s);} SEMINUM explist
	 ;

exp:     exp PLUSNUM term
           { 
                $$.v = $1.v + $3.v;
                sprintf($$.s, "%s + %s", $1.s, $3.s);
           }
         | term
           {
                $$.v = $1.v;
                strcpy($$.s, $1.s);
           }
         ;
term:    term TIMESNUM item
           {
                $$.v = $1.v * $3.v;
                sprintf($$.s, "%s * %s", $1.s, $3.s);
           }
         | item
           {
                $$.v = $1.v; strcpy($$.s, $1.s);
           }
         ;
item:    LPARENNUM exp RPARENNUM
           {
	       $$.v = $2.v; sprintf($$.s , "(%s)", $2.s);
           }
         | INTEGERCONST
           {
                $$.v = $1; sprintf($$.s, "%d", $1); 
           }
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
  fprintf(outfile, "  cout << %s << \"\\n\";\n", s);
}

void yyerror(const char *str)
{    printf("yyerror: %s at line %d\n", str, yyline);
}

int main()
{
  if (!yyparse()){
     printf("accept\n");
  }
  else printf("reject\n");
}    








