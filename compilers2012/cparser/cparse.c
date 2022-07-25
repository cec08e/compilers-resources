/**************************************************
 *                                                *
 *       Name: Caitlin Carnahan                   *
 *      Class: COP5621                            *
 * Assignment: Asg 3 (Implementing a C parser)    *
 *    Compile: "gcc -o cparse cparse.c"           *
 *        Run: "./cparse < test.txt > tmp.out"    *
 *                                                *
 **************************************************/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Node for linked list of productions
typedef struct production{
  char lhs;
  char rhs[100];
  struct production * link;
} production;

// Node for linked list of items
typedef struct item{
  int cursor_position;
  struct production * prod;
  struct item * link;
  int go_to;
} item;

// Node for linked list of item sets
typedef struct set{
  struct set* link;
  struct item* first_item;
  int set_number;
  char go_to_input;
} set;

// Holds set of chars to check for goto states
char goto_symbols[100];

// Function prototypes
production* add_production(production*, char, char*);
item* add_item(item*, production*, int, int);
set* add_set(set*, item*, char);
void print_augmented_grammar_title();
void print_augmented_grammar(production*);
void print_sets_title();
void print_sets(production*);
set* search_sets(set*, item*, char);
void print_sets_items(set*);
void print_items(item*);
char* closure(item*, production*);
void go_to_func(item*, char, set*);



int main(){
  production *production_link, *head;
  char lhs = '\0';
  char rhs[100] = { '\0' };
  int j = 0;
  int rhs_flag = 0;
  char c;

  production_link = NULL;

  // Grabbing the start symbol and adding the first production to the augmented grammar
  c = getc(stdin);
  lhs = '\'';
  rhs[0] = c;
  while(c!='\n'){
    c = getc(stdin);
  }
  production_link = add_production(production_link, lhs, rhs);
  head = production_link;

  // Adding the rest of the productions to the augmented grammar
  while((c = getc(stdin)) != EOF){
    if(c == '\n'){
      if(rhs[0]!='\0' && lhs!='\0'){
	production_link = add_production(production_link, lhs, rhs);
        lhs = '\0';
      }
      memset( rhs, '\0', sizeof(rhs));
      rhs_flag = 0;
      j = 0;
    }
    else if(c == '-'){
      if((c = getc(stdin)) == '>'){
        rhs_flag = 1;
      }
      else{
	ungetc(c, stdin);
	rhs[j] = '-';
	j = j + 1;
      }
    }
    else if(rhs_flag && c!='\n' && c!=' ' && c!='\t'){
      rhs[j] = c;
      j = j + 1;
    }
    else if(!rhs_flag){
      lhs = c;
    }
  }
  if(rhs[0]!='\0' && lhs!='\0')
    production_link = add_production(production_link, lhs, rhs);

  // Printing Augmented Grammar title
  print_augmented_grammar_title();

  // Printing Augmented Grammar
  print_augmented_grammar(head);

  // Printing sets of lr(0) items title
  print_sets_title();

  // Printing sets of lr(0) items
  print_sets(head);

  return 0;
}

// Adds a production to a linked list of productions
production* add_production(production* production_link, char lhs, char* rhs) {
  production* pl = production_link;

  // If list isn't empty, append to end and return original node
  if (production_link != NULL) {
    while (production_link -> link != NULL)
      production_link = production_link -> link;
    production_link -> link = (struct production *) malloc (sizeof (production));
    production_link = production_link -> link;
    production_link -> link = NULL;
    production_link -> lhs = lhs;
    strcpy(production_link -> rhs, rhs);
    return pl;
  }
  else {
    // If list is empty, create a node and return this node
    production_link = (struct production *) malloc (sizeof (production));
    production_link -> link = NULL;
    production_link -> lhs = lhs;
    strcpy(production_link -> rhs, rhs);
    return production_link;
  }
}

// Adds an item to a linked list of items
item* add_item(item* item_link, production* prod, int cursor_position, int go_to) {
  item* il = item_link;

  // If list isn't empty, append to end and return original node
  if (item_link != NULL) {
    while (item_link -> link != NULL)
      item_link = item_link -> link;
    item_link -> link = (struct item  *) malloc (sizeof (item));
    item_link = item_link -> link;
    item_link -> link = NULL;
    item_link -> prod = prod;
    item_link -> cursor_position = cursor_position;
    item_link -> go_to = go_to;
    return il;
  }
  else {
    // If list is empty, create a node and return this node
    item_link = (struct item *) malloc (sizeof (item));
    item_link -> link = NULL;
    item_link -> prod = prod;
    item_link -> cursor_position = cursor_position;
    item_link -> go_to = go_to;
    return item_link;
  }
}

// Adds a set to a linked list of sets
set* add_set(set* set_link, item* it, char g){
  set* sl = set_link;
  int set_number = 0;

  // If list isn't empty, append to end and return newest node
  if (set_link != NULL) {
    while (set_link -> link != NULL)
      set_link = set_link -> link;
    set_link -> link = (struct set *) malloc (sizeof (set));
    set_number = set_link->set_number;
    set_link = set_link -> link;
    set_link->link = NULL;
    set_link->set_number = set_number + 1;
    set_link->first_item = it;
    set_link->go_to_input = g;
    return set_link;
  }
  else {
    // If list is empty, create a node and return this node
    set_link = (struct set *) malloc (sizeof (set));
    set_link -> link = NULL;
    set_link->set_number = set_number;
    set_link->first_item = it;
    set_link->go_to_input = g;
    return set_link;
  }

}

// Prints the "augmented grammar" title
void print_augmented_grammar_title(){
  printf("Augmented Grammar\n");
  printf("-----------------\n");
}

// Prints the productions in the augmented grammar
void print_augmented_grammar(production * production_link){
  while (production_link != NULL) {
    printf ("%c->%s\n", production_link->lhs, production_link->rhs);
    production_link = production_link -> link;
  }
  printf ("\n");
}

// Prints the "sets of lr(0) items" title
void print_sets_title(){
  printf("Sets of LR(0) Items\n");
  printf("-------------------\n");
}


void print_sets(production* head){
  item* head_item;
  item* item_link = NULL;
  set* set_link = NULL;
  set* first_set;
  int go_to = -1;
  char target_token;
  int index;

  // Create item from first production of augmented grammar
  item_link = add_item(item_link, head, 0, go_to);

  // Create first set from first item
  set_link = add_set(set_link, item_link, '\0');
  head_item = item_link;
  first_set = set_link;

  // Loop through as sets are appended
  while(set_link != NULL){
    // Find closure of item
    closure(set_link->first_item, head);

    // Determine goto for set of goto symbols from closure
    for(index = 0; index < strlen(goto_symbols); index++){
      go_to_func(set_link->first_item, goto_symbols[index], first_set);
    }

    // Move to next set in list
    set_link = set_link->link;
  }

  // Print out all sets
  print_sets_items(first_set);
}

// Searches the existing sets for an appropriate goto action
set* search_sets(set* set_link, item* item_link, char go_to_input){
  while (set_link != NULL) {
    if(set_link->go_to_input == go_to_input){
      if(set_link->first_item->prod == item_link->prod){
	return set_link;
      }
    }
    set_link = set_link -> link;
  }
  return set_link;
}

// Prints set labels and calls function to print set items
void print_sets_items(set* set_link){
  if (set_link == NULL)
    printf ("");
  else
    while (set_link != NULL) {
      printf("I%d:\n", set_link->set_number);
      print_items(set_link->first_item);
      set_link = set_link -> link;
    }
}

// Prints set items following the first (passed) set
void print_items(item * item_link){
  if (item_link == NULL)
    printf ("No items in set. \n");
  else
    while (item_link != NULL) {
      char prod_str[105] = {'\0'};
      int i = 0;
      int j = 3;

      prod_str[0] = item_link->prod->lhs;
      prod_str[1] = '-';
      prod_str[2] = '>';
      for(i = 0; i < 100; i++){
	if(item_link->cursor_position == i){
	  prod_str[j] = '@';
	  j = j + 1;
	}
	prod_str[j] = item_link->prod->rhs[i];
	j = j + 1;
      }

      if(item_link->go_to != -1)
	printf("   %-20s goto(%c)=I%d\n", prod_str, item_link->prod->rhs[item_link->cursor_position], item_link->go_to);
      else
	printf("   %-20s\n", prod_str);
      item_link = item_link -> link;
    }
  printf("\n");
}

// Finds closure of item list starting with item_link
char* closure(item* item_link, production* head){
  item* closure_item = item_link;
  production* production_link;
  char target_token = '@';
  int cursor_position = 0;
  int gts = 0;
  memset(goto_symbols, '\0', sizeof(goto_symbols));
  while(closure_item != NULL){
    production_link = head;
    if(closure_item->cursor_position >= strlen(closure_item->prod->rhs)){
      production_link = NULL;
    }
    else{
      target_token = closure_item->prod->rhs[closure_item->cursor_position];
      goto_symbols[gts] = target_token;
      gts = gts + 1;
    }

    if (production_link == NULL)
      printf ("");
    else
      while (production_link != NULL) {
	if(production_link->lhs == target_token){
	  item_link = add_item(item_link, production_link, cursor_position, -1);
	}
	production_link = production_link->link;
      }
    while(closure_item != NULL){
      if(closure_item->cursor_position >= strlen(closure_item->prod->rhs)){
	closure_item = closure_item->link;
      }
      else if(closure_item->prod->rhs[closure_item->cursor_position] == target_token){
	closure_item = closure_item->link;
      }
      else{
	break;
      }
    }
  }
  return goto_symbols;

}

// Finds goto action for items following item_link on input symbol
void go_to_func(item* item_link, char symbol, set* set_link){
  item* new_item = NULL;
  item* follow_item = NULL;
  int new_item_flag = 1;
  int existing_set = 0;
  set* temp_set;
  int set_number = 0;
  int cursor_position;

  while (item_link != NULL) {
    if(item_link->prod->rhs[item_link->cursor_position] == symbol){
      if(new_item_flag){
	if((temp_set = search_sets(set_link, item_link, symbol)) != NULL){
	  item_link->go_to = temp_set->set_number;
	  set_number = item_link->go_to;
	  existing_set = 1;
	  new_item_flag = 0;
	}
	else{
	  cursor_position = item_link->cursor_position + 1;
	  new_item = add_item(new_item, item_link->prod, cursor_position, -1);
	  temp_set = add_set(set_link, new_item, symbol);
	  set_number = temp_set->set_number;
	  item_link->go_to = set_number;
	  new_item_flag = 0;
	  follow_item = new_item;
	}
      }
      else{
	if(existing_set){
	  //item_link->go_to = set_number;
	  printf("");
	}
	else{
	  cursor_position = item_link->cursor_position + 1;
      	  follow_item = add_item(follow_item, item_link->prod, cursor_position, -1);
	}
      }

    }
    item_link = item_link->link;
  }
}
