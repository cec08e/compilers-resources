/************************************************************************
 *    A simple manual lexical analyzer for a subset of C.               *
 *    Simply parses the incoming C program and reports the tokens found *
 *    Compile: "gcc -g -o cscan cscan.c"                                *
 *                                                                      *
 ************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

int handle_string(char c);
int handle_digit(char c);
int handle_identifier(char c);
int handle_forward(char c, int*, char**);

int main()
{
  int number_count = 0;
  int ident_count = 0;
  int char_count = 0;
  int string_count = 0;
  char* lexeme_arr[] = {"<<=", ">>=", "&&", "++", "--", "!=", "%=", "&=", "*=", "+=", "-=", "->", "/=", "<<", "<=", "==", ">=", ">>", "^=", "|=", "||", "!", "&", "%", "(", ")", "*", "+", ",", "-", ".", "/", ":", ";", "<", "=", ">", "?", "[", "]", "^", "{", "|", "}", "~"};
  int lexeme_count[45] = { 0 };

  char c;
  while((c = getc(stdin)) != EOF){

    if(c == '"'){
      handle_string(c);
    }
    else if(isdigit(c)){
      if(handle_digit(c) == 0){
	       fprintf(stderr, "Error completing digit token.\n");
      }
      else{
	       number_count = number_count + 1;
      }
    }
    else if(isalpha(c)){
      if(handle_identifier(c) == 0){
	       fprintf(stderr, "Error completing identifier token. \n");
      }
      else{
	       ident_count = ident_count + 1;
      }
    }
    else if(c == '/'){
      if(handle_forward(c, lexeme_count, lexeme_arr) == 1){
	       fprintf(stderr, "Error handling backslash token. \n");
      }
    }
    printf("\n");
  }
  printf("      token           count \n");
  printf("-------------------   ----- \n");




}

int handle_string(char c){
  return(1);
}

int handle_digit(char c){
  //char* digit = c;
  while(isdigit(c)){
    //printf("%c", c);
    putchar(c);
    c = getc(stdin);
  }
  ungetc(c, stdin);
  return(1);
}

int handle_identifier(char c){
  //char* ident = c;
  while(isalpha(c) || isdigit(c)){
    //printf("%c", c);
    putchar(c);
    c = getc(stdin);
  }
  ungetc(c, stdin);
  return(1);

}

int handle_forward(char c, int* lexeme_count, char** lexeme_arr){
  c = getc(stdin);
  int comment_flag = 0;
  int multiline_flag = 1;

  // handle multi-line comments
  if(c == '*'){
    c = getc(stdin);
    while(comment_flag == 0 && c != EOF){
      if(c == '*'){
	c = getc(stdin);
	if(c == '/'){
	  comment_flag = 1;
	  multiline_flag = 0;
	  return(multiline_flag);
	}
      }
      else{
	c = getc(stdin);
      }
    }

  }

  // handle single-line comments
  else if(c == '/'){
    while(c != '\n' && c != EOF){
      c = getc(stdin);
    }
    return(0);
  }

  // handle divide-equals
  else if(c == '='){
    int j = -1;
    int i;
    for (i = 0 ; i < 45 && j == -1; i++)
      {
	if (strncmp("/=",lexeme_arr[i], 10) == 0)
	  {
	    j = i;
	    lexeme_count[i] = lexeme_count[i] + 1;
	  }
      }
    return(0);
  }

  // handle divide
  else{
    int j = -1;
    int i;
    for (i = 0 ; i < 45 && j == -1; i++)
      {
        if (strncmp("/",lexeme_arr[i], 10) == 0)
          {
            j = i;
            lexeme_count[i] = lexeme_count[i] + 1;
          }
      }
    return(0);

  }

  return(1);
}
