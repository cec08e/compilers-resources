/************************************
 *    Caitlin Carnahan              *                     
 *    sem.c                         *
 ************************************/
# include <stdio.h>
# include "cc.h"
# include "semutil.h"
# include "sem.h"
# include "sym.h"

int last_label = 0;
int continue_label = 0;

/*
 * backpatch - backpatch list of triples starting at p with k
 */
void backpatch(struct sem_rec *p, int k)
{
  while(p != NULL){
    printf("%d.\tB%d=L%d\n", nexttrip(), p->s_place, k);
    p = p->back.s_link;
  }
}

/*
 * call - procedure invocation
 */
struct sem_rec *call(char *f, struct sem_rec *args)
{
  struct sem_rec *arg;
  struct id_entry *p;
  int num_args = 0;
  char mode;
  char func_type;
  if(args != NULL){
    arg = args->back.s_true;
  }
  while(arg != NULL){
    mode = 'i';

    if(arg->s_mode == T_FLOAT || arg->s_mode == T_FLOAT + T_ARRAY){
      mode = 'f';
    }
    printf("%d.\targ%c %d\n", nexttrip(), mode, arg->s_place);
    arg = arg->s_false;
    num_args++;
  }

  printf("%d.\tglobal %s\n", nexttrip(), f);
  p = NULL;
  func_type = 'i';
  p = lookup(f, 2);
  if(p != NULL){
    if(p->i_type == T_FLOAT){
      func_type = 'f';
    }
  }

  printf("%d.\tf%c %d %d\n", nexttrip(), func_type, currtrip()-1, num_args);
  arg = node(currtrip(), T_PROC, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  return arg;
}

/*
 * ccand - logical and
 */
struct sem_rec *ccand(struct sem_rec *e1, int m, struct sem_rec *e2)
{
  struct sem_rec *t;
  struct sem_rec *false_one;
  struct sem_rec *false_two;

  // Create false list
  //false_one = e1->s_false;
  //false_two = e2->s_false;
  //false_one->back.s_link = false_two;
  false_one = merge(e1->s_false, e2->s_false);

  backpatch(e1->back.s_true, m);
  t = node(currtrip(), T_LBL, (struct sem_rec *) NULL, false_one);
  t->back.s_true = e2->back.s_true;
  return t;
}

/*
 * ccexpr - convert arithmetic expression to logical expression
 */
struct sem_rec *ccexpr(struct sem_rec *e)
{
  struct sem_rec *t;
  t = con("0");
  t = rel("!=", e, t);
  return t;
}

/*
 * ccnot - logical not
 */
struct sem_rec *ccnot(struct sem_rec *e)
{
  struct sem_rec *t;
  t = node(currtrip(), T_LBL, (struct sem_rec *) NULL, e->back.s_true);
  t->back.s_true = e->s_false;
  return t;
}

/*
 * ccor - logical or
 */
struct sem_rec *ccor(struct sem_rec *e1, int m, struct sem_rec *e2)
{
  struct sem_rec *t;
  struct sem_rec *true_one;
  struct sem_rec *true_two;

  // Create false list
  //true_one = e1->back.s_true;
  //true_two = e2->back.s_true;
  //true_one->back.s_link = true_two;
  true_one = merge(e1->back.s_true, e2->back.s_true);

  backpatch(e1->s_false, m);
  t = node(currtrip(), T_LBL, (struct sem_rec *) NULL, e2->s_false);
  t->back.s_true = true_one;
  return t;

}

/*
 * con - constant reference in an expression
 */
struct sem_rec *con(char *x)
{
  struct sem_rec *t;
  printf("%d.\tcon %s\n", nexttrip(), x);
  t = node(currtrip(), T_INT, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  return t;
}

/*
 * dobreak - break statement
 */
struct sem_rec *dobreak()
{
  //fprintf(stderr, "sem: dobreak not implemented\n");
  //return ((struct sem_rec *) NULL);
  struct sem_rec *t;
  struct sem_rec *temp;
  //printf("%d.\tbr %d\n", nexttrip(), continue_label);
  //t = node(currtrip(), T_PROC, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  //return t;
  //fprintf(stderr, "sem: docontinue not implemented\n");
  //return ((struct sem_rec *) NULL);
  printf("%d.\tbr B%d\n", nexttrip(), currtrip());
  temp = node(currtrip(), T_PROC, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  t = node(currtrip(), T_PROC, temp, (struct sem_rec *) NULL);
  return t;

}

/*
 * docontinue - continue statement
 */
struct sem_rec *docontinue()
{
  struct sem_rec *t;
  struct sem_rec *temp;
  //printf("%d.\tbr %d\n", nexttrip(), continue_label);
  //t = node(currtrip(), T_PROC, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  //return t;
  //fprintf(stderr, "sem: docontinue not implemented\n");
  //return ((struct sem_rec *) NULL);
  printf("%d.\tbr B%d\n", nexttrip(), currtrip());
  temp = node(currtrip(), T_PROC, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  t = node(currtrip(), T_PROC, (struct sem_rec *) NULL, temp);
  return t;
}

/*
 * dodo - do statement
 */
struct sem_rec *dodo(int m1, struct sem_rec *s, int m2, struct sem_rec *e)
{

  //fprintf(stderr, "sem: dodo not implemented\n");
  //return ((struct sem_rec *) NULL);

  struct sem_rec *t;

  backpatch(e->back.s_true, m1);
  //s->back.s_link = e->s_false;
  t = node(currtrip(), T_LBL, (struct sem_rec *) NULL, e->s_false);
  return t;
}

/*
 * dofor - for statement
 */
struct sem_rec *dofor(int m1, struct sem_rec *e2, int m2, struct sem_rec *n,
                      int m3, struct sem_rec *s)
{
  // for ( expro ; m1 e2 ; m2 expro n ) m3 s
  struct sem_rec *t;
  backpatch(e2->back.s_true, m3);
  backpatch(n, m1);
  printf("%d.\tbr L%d\n", nexttrip(), m2);
  t = node(currtrip(), T_LBL, (struct sem_rec *) NULL, e2->s_false);
  return t;
}

/*
 * dogoto - goto statement
 */
struct sem_rec *dogoto(char *id)
{
  //fprintf(stderr, "sem: dogoto not implemented\n");
  //return ((struct sem_rec *) NULL);
  struct id_entry *p;
  struct sem_rec *t;
  p = NULL;
  extern int level;
  int temp_level = level;
  while(p == NULL && temp_level > 1){
    p = lookup(id, temp_level);
    temp_level--;
  }

  if(p!=NULL){
    printf("%d.\tbr %d\n", nexttrip(), p->i_defined);
    t = node(currtrip(), T_LBL, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
    return t;
  }

  return (struct sem_rec *) NULL;
}

/*
 * doif - one-arm if statement
 */
struct sem_rec *doif(struct sem_rec *e, int m, struct sem_rec *s)
{
  // e is cexpr in if(cexpr)
  // m label in if(cexpr) m stmt
  // s is stmt

  struct sem_rec *t;
  struct sem_rec *c_m;
  if(s->back.s_link != NULL){
    c_m = merge(e->s_false, s->back.s_link);
  }
  else
    c_m = e->s_false;
  backpatch(e->back.s_true, m);
  s->back.s_link = e->s_false;
  t = node(currtrip(), T_LBL, (struct sem_rec *) NULL, c_m);
  return t;
}

/*
 * doifelse - if then else statement
 */
struct sem_rec *doifelse(struct sem_rec *e, int m1, struct sem_rec *s1,
                         struct sem_rec *n, int m2, struct sem_rec *s2)
{
  // if ( e ) m1 s1 else n m2 s2
  struct sem_rec *t;
  backpatch(e->back.s_true, m1);
  backpatch(e->s_false, m2);
  //s->back.s_link = e->s_false;
  t = node(currtrip(), T_LBL, (struct sem_rec *) NULL, n);
  return t;
}

/*
 * doret - return statement
 */
struct sem_rec *doret(struct sem_rec *e)
{
  struct sem_rec *t;
  int return_type = e->s_mode;
  char ret = 'i';
  if(return_type == T_FLOAT){
    ret = 'f';
  }
  printf("%d.\tret%c %d\n", nexttrip(), ret, e->s_place);
  t = node(currtrip(), e->s_mode, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  return t;
}

/*
 * dostmts - statement list
 */
struct sem_rec *dostmts(struct sem_rec *sl, int m, struct sem_rec *s)
{
  if(sl != NULL){
    sl->back.s_link = s;
    if(sl->s_false != NULL)
      backpatch(sl->s_false, m);
  }
  return s;

}

/*
 * dowhile - while statement
 */
struct sem_rec *dowhile(int m1, struct sem_rec *e, int m2, struct sem_rec *s)
{
  //fprintf(stderr, "sem: dowhile not implemented\n");
  //return ((struct sem_rec *) NULL);
  struct sem_rec *t;
  //struct sem_rec *t_e;
  // while( m1 e) m2 s
  printf("%d.\tbr L%d\n", nexttrip(), m1);
  backpatch(e->back.s_true, m2);
  //t_e = e->s_false;
  //t = node(t_e->s_place, T_LBL, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  t = node(currtrip(), T_LBL, (struct sem_rec *) NULL, e->s_false);
  return t;
}

/*
 * endloopscope - end the scope for a loop
 */
void endloopscope(struct sem_rec *p)
{
  exit_block();
}

/*
 * exprs - form a list of expressions
 */
struct sem_rec *exprs(struct sem_rec *l, struct sem_rec *e)
{
  //fprintf(stderr, "sem: exprs not implemented\n");
  //return ((struct sem_rec *) NULL);

  if(l->back.s_true == NULL){
    l->back.s_true = l;
  }
  l->s_false = e;
  e->back.s_true = l->back.s_true;
  return(e);
}

/*
 * fhead - beginning of function body
 */
struct id_entry *fhead(struct id_entry *p)
{
  //printf("In fhead.......\n");
  //fprintf(stderr, "sem: fhead not implemented\n");
  //return ((struct id_entry *) NULL);
  extern int formalnum;
  extern char formaltypes[50];
  extern int localnum;
  extern char localtypes[50];
  extern localwidths[50];

  int temp_count = 0;
  int param_size = 0;

  for(temp_count = 0; temp_count < formalnum; temp_count++){
    //printf("temp_count: %d. formalnum: %d\n", temp_count, formalnum);
    //printf("%s\n", formaltypes);
    if(formaltypes[temp_count] == 'i'){
      param_size = 4;
    }
    else{
      param_size = 8;
    }
    printf("%d.\tformal %d\n", nexttrip(), param_size);
  }

  //printf("formal num: %d\n", formalnum);

  for(temp_count = 0; temp_count < localnum; temp_count++){
    if(localtypes[temp_count] == 'i'){
      param_size = 4;
    }
    else{
      param_size = 8;
    }
    printf("%d.\tlocalloc %d\n", nexttrip(), param_size);
  }

  return(p);
}

/*
 * fname - function declaration
 */
struct id_entry *fname(int t, char *id)
{
  struct id_entry *p;
  p = dclr(id, 0, 0);
  p = dcl(p, t, GLOBAL);
  printf("%d.\tfunc %s\n", nexttrip(), p->i_name);
  enterblock();
  return(p);
  //fprintf(stderr, "sem: fname not implemented\n");
  // return ((struct id_entry *) NULL);
}

/*
 * ftail - end of function body
 */
struct sem_rec *ftail(struct id_entry *p, struct sem_rec *s, int m)
{
  extern int formalnum;
  extern int localnum;
  formalnum = 0;
  localnum = 0;

  if(s != NULL && s->s_false != NULL)
    backpatch(s->s_false, m);
  printf("%d.\tfend\n", nexttrip());
  return ((struct sem_rec *) NULL);
}

/*
 * id - variable reference
 */
struct sem_rec *id(char *x)
{
  //fprintf(stderr, "sem: id not implemented\n");
  extern int level;
  struct id_entry *p;
  struct sem_rec *t;
  int temp_type;
  char type_char;
  p = NULL;
  int temp_level = level;

  while(p == NULL && temp_level > 1){
    p = lookup(x, temp_level);
    temp_level--;
  }

  temp_type = p->i_type;
  if(temp_type == T_INT || temp_type == T_INT + T_ARRAY){
    type_char = 'i';
  }
  else if(temp_type == T_FLOAT || temp_type == T_FLOAT + T_ARRAY){
    type_char = 'f';
  }

  if(p->i_scope == PARAM){
    printf("%d.\tparam %d\n", nexttrip(), p->i_offset);
  }
  else if(p->i_scope == LOCAL){
    printf("%d.\tlocal %d\n", nexttrip(), p->i_offset);
  }
  else if(p->i_scope == GLOBAL){
    printf("%d.\tglobal %s\n", nexttrip(), p->i_name);
  }


  t = node(currtrip(), temp_type, (struct sem_rec *) NULL, (struct sem_rec *) NULL);

  return (t);
}

/*
 * index - subscript
 */
struct sem_rec *index(struct sem_rec *x, struct sem_rec *i)
{
  //fprintf(stderr, "sem: index not implemented\n");
  //return ((struct sem_rec *) NULL);

  struct sem_rec *t;
  char mode = 'i';
  if(x->s_mode == T_FLOAT || x->s_mode == T_FLOAT + T_ARRAY){
    mode = 'f';
  }
  printf("%d.\t[]%c %d %d\n", nexttrip(), mode, x->s_place, i->s_place);
  t = node(currtrip(), x->s_mode, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  return t;
}

/*
 * labeldcl - process a label declaration
 */
void labeldcl(char *id)
{
  //fprintf(stderr, "sem: labeldcl not implemented\n");

  struct id_entry *p;
  p = dclr(id, T_LBL, 0);
  p->i_defined = currtrip() + 1;
}

/*
 * m - generate label and return next triple number
 */
int m()
{
  if(last_label != currtrip()+1){
    printf("  \tlabel L%d\n", currtrip()+1);
  }
  last_label = currtrip()+1;
  return(currtrip() + 1);
}

/*
 * n - generate goto and return backpatch pointer
 */
struct sem_rec *n()
{
  //fprintf(stderr, "sem: n not implemented\n");
  //return ((struct sem_rec *) NULL);
  struct sem_rec *t;
  printf("%d.\tbr B%d\n", nexttrip(), currtrip());
  t = node(currtrip(), T_PROC, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  return t;
}

/*
 * op1 - unary operators
 */
struct sem_rec *op1(char *op, struct sem_rec *x)
{
  char mode;
  struct sem_rec *t;
  //fprintf(stderr, "sem: op1 not implemented\n");
  //return ((struct sem_rec *) NULL);
  if(op[0] == '@' || op[0] == '-' || op[0] == '~'){
    if(x->s_mode == T_INT || x->s_mode == T_INT + T_ARRAY){
      mode = 'i';
    }
    else if(x->s_mode == T_FLOAT || x->s_mode == T_FLOAT + T_ARRAY){
      mode = 'f';
    }
    printf("%d.\t%s%c %d\n", nexttrip(), op, mode, x->s_place);
    t = node(currtrip(), x->s_mode, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
    // Holds link to declaration of dereferenced variable
    //t->s_false = x;
  }
  else{
    t = (struct sem_rec *) NULL;
  }

  return(t);
}

/*
 * op2 - arithmetic operators
 */
struct sem_rec *op2(char *op, struct sem_rec *x, struct sem_rec *y)
{
  //fprintf(stderr, "sem: op2 not implemented\n");
  //return ((struct sem_rec *) NULL);
  char mode;
  struct sem_rec *t;
  if(x->s_mode == T_INT || x->s_mode == T_INT + T_ARRAY){
    mode = 'i';
  }
  else if(x->s_mode == T_FLOAT || x->s_mode == T_FLOAT + T_ARRAY){
    mode = 'f';
  }
  printf("%d.\t%s%c %d %d\n", nexttrip(), op, mode, x->s_place, y->s_place);
  t = node(currtrip(), x->s_mode, (struct sem_rec *) NULL, (struct sem_rec *) NULL);

  return t;
}

/*
 * opb - bitwise operators
 */
struct sem_rec *opb(char *op, struct sem_rec *x, struct sem_rec *y)
{
  struct sem_rec *t;
  printf("%d.\t%si %d %d\n", nexttrip(), op, x->s_place, y->s_place);
  t = node(currtrip(), T_PROC, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  return t;
}

/*
 * rel - relational operators
 */
struct sem_rec *rel(char *op, struct sem_rec *x, struct sem_rec *y)
{
  //fprintf(stderr, "sem: rel not implemented\n");
  //return ((struct sem_rec *) NULL);
  struct sem_rec *t;
  struct sem_rec *true_patch;
  struct sem_rec *false_patch;
  char mode = 'i';
  int temp_label = y->s_place;
  if(x->s_mode == T_FLOAT){
    mode = 'f';
  }
  if(x->s_mode != y->s_mode){
    printf("%d.\tcv%c %d\n", nexttrip(), mode, y->s_place);
    temp_label = currtrip();
  }
  printf("%d.\t%s%c %d %d\n", nexttrip(), op, mode, x->s_place, temp_label);
  t = node(currtrip(), x->s_mode, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  printf("%d.\tbt %d B%d\n", nexttrip(), currtrip()-1, currtrip());
  true_patch = node(currtrip(), T_LBL, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  printf("%d.\tbr B%d\n", nexttrip(), currtrip());
  false_patch = node(currtrip(), T_LBL, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  t->back.s_true = true_patch;
  t->s_false = false_patch;
  return t;
}

/*
 * set - assignment operators
 */
struct sem_rec *set(char *op, struct sem_rec *x, struct sem_rec *y)
{
  //fprintf(stderr, "sem: set not implemented\n");
  //return ((struct sem_rec *) NULL);
  char mode = 'i';
  struct sem_rec *t;
  struct sem_rec *deref_x;
  // Link to definition
  //if(x->s_false != NULL)
  //  t = x->s_false;
  //else
  //  t = x;
  if(x->s_mode == T_FLOAT || x->s_mode == T_FLOAT + T_ARRAY){
    mode = 'f';
  }
  if(strcmp(op,"") != 0){
    //dereference x always
    deref_x = op1("@", x);
    t = op2(op, deref_x, y);
    printf("%d.\t=%c %d %d\n", nexttrip(), mode, x->s_place, t->s_place);
  }
  else{
    printf("%d.\t=%c %d %d\n", nexttrip(), mode, x->s_place, y->s_place);
  }
  //printf("%d.\t=%c %d %d\n", nexttrip(), mode, t->s_place, currtrip()-1);
  t = node(currtrip(), x->s_mode, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  return t;
}

/*
 * startloopscope - start the scope for a loop
 */
void startloopscope()
{
  enterblock();
}

/*
 * string - generate code for a string
 */
struct sem_rec *string(char *s)
{
  struct sem_rec *t;
  //fprintf(stderr, "sem: string not implemented\n");
  //return ((struct sem_rec *) NULL);
  printf("%d.\tstr %s\n", nexttrip(), s);
  //extern struct sem_rec **top;
  t = node(currtrip(), T_STR, (struct sem_rec *) NULL, (struct sem_rec *) NULL);
  return(t);
}
