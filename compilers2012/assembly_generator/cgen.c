/*************************************************************
 *                                                			 *
 *       Name: Caitlin Carnahan                   			 *
 *      Class: COP5621                                       *
 * Assignment: Asg 6 (Implementing an assembly generator)    *
 *    Compile: "gcc -o cgen cgen.c"                          * 
 *        Run: "./cgen < test.txt > test.s"                  *
 *                                                           *
 *************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXTRIPLESTR 100
#define SEGTEXT 0 
#define SEGDATA 1
#define MINSTACKSIZE 92
#define REGWINDOW 120

int SEGFLAG;
int *trip_reg_map;
int local_reg_map[8];
int float_reg_map[16];
int *temp_array;
int trip_size = 100;
int B_values[100];
int B_size = 100;
int L_values[100];
void check_segflag(int);
char *convert_reg_alpha(int);
void double_trip_reg_map();
int modify_sp_offset(int);
void assign_to_register(int);
void assign_to_float_register(int);

int main(){

  	trip_reg_map = malloc(trip_size*sizeof(int));
  	memset(trip_reg_map, 0, sizeof trip_reg_map);
	memset(local_reg_map, 0, sizeof local_reg_map);
	memset(float_reg_map, 0, sizeof float_reg_map);
  	memset(B_values, 0, sizeof B_values);
	SEGFLAG = 1;
	int tn, ln, size, formal, local, i, j, offset, con, arg1, arg2;
	int formal_count = 0;
	int arg_list[6] = {0};
	char mode, op;
	char reg1[2];
	char reg2[2];
	char fname[MAXTRIPLESTR];
	char S[MAXTRIPLESTR];
	char op2_0;
	char op2_1;
	// char op2[2];
	char branch_arg;
	// Maps local i to its size localvars[i]
	int localvars[20];
	// Maps local i to its offset in activ. rec. localvars_offset[i]
	int localvars_offset[20] = {0};
	int formal_offset[6] = {0};
	// Holds next available offset slot
	int local_sp_offset = 112;
	// Stores number of float registers assigned
	int num_floats = 0;
	// Holds number of local vars currently allocated
	int num_localvars = 0;
	int num_localvars_reffed = 0;
	int sp_offset = 0;

	while(fgets(S, MAXTRIPLESTR, stdin)){
		// Handle function declaration
		if(sscanf(S, "%d.\t func %s", &tn, fname) == 2){
			formal_count = 0;
			memset(formal_offset, 0, sizeof formal_offset);
			check_segflag(SEGTEXT);
			printf("\t.global %s\n", fname);
			//printf("\t.align\t8\n");
			printf("%s:\n", fname);
			
			/*
			if(strcmp(fname, "main") != 0){
			  sp_offset = modify_sp_offset(sp_offset);
			}
			else{
			  sp_offset = 28;
			}
			printf("\tsave\t%%sp,(-%d),%%sp\n", MINSTACKSIZE+sp_offset);*/
			printf("\tsave\t%%sp,(-%d),%%sp\n", REGWINDOW-(num_localvars_reffed*8));
			  //printf("\tsave\t%%sp,(-120),%%sp\n");
		}
		// Handle local variable allocation
		else if(sscanf(S, "%d.\t localloc %d", &tn, &size) == 2){
		    localvars[num_localvars] = size;
		    num_localvars++;
		    sp_offset = sp_offset + size; 
		}
		// Handle formal parameter allocation
		else if(sscanf(S, "%d.\tformal %d", &tn, &formal) == 2){
			formal_count++;
			if(formal == 4){
				printf("\tst\t%%i%d,[%%fp + %d]\n", formal_count-1, 64+(formal_count*4));
				for(i = 0; i < 6; i++){
					if(formal_offset[i] == 0){
						formal_offset[i] = 64+(formal_count*4);
						break;
					}
				}	
			}
			else{
				for(i = 0; i < 6; i++){
					if(formal_offset[i] == 0){
						formal_offset[i] = 64+(formal_count*4);
						break;
					}
				}
				printf("\tst\t%%i%d,[%%fp + %d]\n", formal_count-1, 64+(formal_count*4));	
				formal_count++;	
				printf("\tst\t%%i%d,[%%fp + %d]\n", formal_count-1, 64+(formal_count*4));	

			}
		}
		// Handle reference to declared locals
		// Returns the address of the local
		else if(sscanf(S, "%d.\t local %d", &tn, &local) == 2){
			assign_to_register(tn);
			if(localvars_offset[local] == 0){
				localvars_offset[local] = local_sp_offset;
				local_sp_offset = local_sp_offset - 8;
				num_localvars_reffed++;
			}
			strcpy(reg1, convert_reg_alpha(trip_reg_map[tn]));
			printf("\tadd\t%%sp,%d,%%%s\n", localvars_offset[local], reg1);
		  
		  //check_segflag(SEGDATA);
		  
		}
		// Handle reference to parameters passed into function
		else if(sscanf(S, "%d.\tparam %d", &tn, &arg1) == 2){	
			assign_to_register(tn);
			strcpy(reg1, convert_reg_alpha(trip_reg_map[tn]));
			printf("\tadd\t%%fp,%d,%%%s\n", formal_offset[arg1], reg1);
		}
		// Handle creation of constant value
		else if(sscanf(S, "%d.\t con %d", &tn, &con) == 2){
		  assign_to_register(tn);
		  strcpy(reg1, convert_reg_alpha(trip_reg_map[tn]));

			printf("\tmov\t%d,%%%s\n", con, reg1);
		}
		// Handle assignment operator
		else if(sscanf(S, "%d.\t =%c %d %d", &tn, &mode, &arg1, &arg2) == 4){
			strcpy(reg1, convert_reg_alpha(trip_reg_map[arg1]));
			strcpy(reg2, convert_reg_alpha(trip_reg_map[arg2]));
			
			if(mode == 'f'){
				printf("\tstd\t%%%s,[%%%s]\n", reg2, reg1);
			}
			else{
				printf("\tst\t%%%s,[%%%s]\n", reg2, reg1);
			}
		}
		// Handle label creation
		else if(sscanf(S, "\t label L%d", &ln) == 1){
		  memset(localvars, 0, sizeof localvars);
		  local_sp_offset = 120 - (8*(num_localvars_reffed+1));
			num_floats = 0;
			memset(local_reg_map, 0, sizeof local_reg_map);
			memset(float_reg_map, 0, sizeof float_reg_map); 
			printf("L%d:\n", ln);
		}
		// Handle precision conversion
		else if(sscanf(S, "%d.\tcv%c %d", &tn, &mode, &arg1) == 3){
			if(mode == 'f'){
				strcpy(reg1, convert_reg_alpha(trip_reg_map[arg1]));
				printf("\tst\t%%%s,[%%sp + %d]\n", reg1, local_sp_offset);
				printf("\tldd\t[%%sp + %d],%%f%d \n", local_sp_offset, num_floats*2);
				printf("\tfitod\t%%f%d,%%f%d \n", (num_floats*2), (num_floats*2));
				trip_reg_map[tn] = 32 + (2*num_floats); 
				num_floats = num_floats + 1;
				local_sp_offset = local_sp_offset - 8;
			}
			else{
			  strcpy(reg1, convert_reg_alpha(trip_reg_map[arg1]));
			  printf("\tst\t%%%s,[%%sp + %d]\n", reg1, local_sp_offset);
			  printf("\tld\t[%%sp + %d],%%f%d \n", local_sp_offset, num_floats*2);
			  printf("\tfdtoi\t%%f%d,%%f%d \n", (num_floats*2), (num_floats*2));
			  trip_reg_map[tn] = 32 + (2*num_floats);
			  num_floats = num_floats + 1;
			  local_sp_offset = local_sp_offset - 8;
			}
		}
		// Handle string declaration
		else if(sscanf(S, "%d.\tstr \"%[^\"]\"", &tn, fname) == 2){
			check_segflag(SEGDATA);
			//printf("\t.align\t8\n");
			printf("LS%d:\n", ln);
			printf("\t.asciz\t\"%s\"\n", fname);
			check_segflag(SEGTEXT);
			//printf("\t.align\t8\n");
			assign_to_register(tn);
			strcpy(reg1, convert_reg_alpha(trip_reg_map[tn]));
			printf("\tsethi\t%%hi(LS%d),%%%s\n", ln, reg1);
			printf("\tor\t%%%s,%%lo(LS%d),%%%s\n", reg1, ln, reg1);
		}
		// Handle dereference operator
		else if(sscanf(S, "%d.\t@%c %d", &tn, &mode, &arg1) == 3){
			if(mode == 'i'){
				strcpy(reg1, convert_reg_alpha(trip_reg_map[arg1]));
				assign_to_register(tn);
				strcpy(reg2, convert_reg_alpha(trip_reg_map[tn]));
				printf("\tld\t[%%%s],%%%s\n", reg1, reg2);
			}
			else{
				strcpy(reg1, convert_reg_alpha(trip_reg_map[arg1]));
				assign_to_float_register(tn);
				strcpy(reg2, convert_reg_alpha(trip_reg_map[tn]));
				printf("\tldd\t[%%%s],%%%s\n", reg1, reg2);
				num_floats = num_floats + 1;
			}
		}
		// Handle creation of argument for passing into function
		else if(sscanf(S, "%d.\targ%c %d", &tn, &mode, &arg1) == 3){
			for(i = 0; i < 6; i++){
				if(arg_list[i] == 0){
					arg_list[i] = trip_reg_map[arg1];
					break;
				}
			}
		}
		// Handle reference to global symbol
		else if(sscanf(S, "%d.\tglobal %s", &tn, fname) == 2){
		  assign_to_register(tn);
			strcpy(reg1, convert_reg_alpha(trip_reg_map[tn]));
			printf("\tsethi\t%%hi(%s),%%%s\n", fname, reg1);
			printf("\tor\t%%%s,%%lo(%s),%%%s\n", reg1, fname, reg1);
			//printf("\tset\t%s,%%%s\n", fname, reg1);
		}
		// Handle function call of triple arg1 with arg2 arguments
		else if(sscanf(S, "%d.\tf%c %d %d", &tn, &mode, &arg1, &arg2) == 4){
			int args_to_call = 0;
			for(i = 0; i < arg2; i++){
				strcpy(reg1, convert_reg_alpha(arg_list[i]));
				if(reg1[0]!='f'){
					reg2[0] = 'o';
					reg2[1] = (char)('0' + i);
					printf("\tmov\t%%%s,%%%s\n", reg1, reg2);
					args_to_call++;
				}
				else{
					printf("\tstd\t%%%s,[%%sp + %d]\n", reg1, local_sp_offset);
					reg2[0] = 'o';
					reg2[1] = (char)('0' + i);
					printf("\tld\t[%%sp + %d],%%%s\n", local_sp_offset, reg2);
					reg2[1] = (char)('0'+ i + 1);
					printf("\tld\t[%%sp + %d],%%%s\n", local_sp_offset+4, reg2);
					i = i + 1;
					num_floats = num_floats + 1;
					local_sp_offset = local_sp_offset - 8; 
					args_to_call = args_to_call + 2;
				}
			}
			strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
			printf("\tcall\t%%%s,%d\n", reg1, args_to_call);
			printf("\tnop\n");
			memset(arg_list, 0, sizeof arg_list);
			trip_reg_map[tn] = 8;
		}
		// Handle operation of arg1 and arg2
		else if(sscanf(S, "%d.\t%c%c %d %d", &tn, &op, &mode, &arg1, &arg2) == 5){
			switch(op){
            	case '+': 
            		if(mode == 'i'){
            			strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
            			strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
            			printf("\tadd\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
            			trip_reg_map[tn] = trip_reg_map[arg1];
            		}
            		else{
			  strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
			  strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
			  printf("\tfaddd\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
			  trip_reg_map[tn] = trip_reg_map[arg1];
            		}
                    break;
            	case '-': 
		  if(mode == 'i'){
		    strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
		    strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
		    printf("\tsub\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
		    trip_reg_map[tn] = trip_reg_map[arg1];

		  }
		  else{
		    strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                    strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                    printf("\tfsubd\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
                    trip_reg_map[tn] = trip_reg_map[arg1];

		  }
                    break;
                case '*':
		  if(mode == 'i'){
		    strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
		    strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
		    printf("\tsmul\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
		    trip_reg_map[tn] = trip_reg_map[arg1];
		  }
		  else{
		    strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                    strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                    printf("\tfmuld\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
                    trip_reg_map[tn] = trip_reg_map[arg1];
		  }
                	break;
                case '/':
                	if(mode == 'i'){
			  strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
			  strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
			  printf("\tudiv\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
			  trip_reg_map[tn] = trip_reg_map[arg1];

			  //printf("\tmov\t%%%s,%%o0\n", reg1);
			  //printf("\tmov\t%%%s,%%o1\n", reg2);
			  //printf("\tcall\t.div\n");
			  //printf("\tnop\n");
			  //printf("\tmov\t%o0,%%%s",reg1); 
                	}
                	else{	
                		strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
            			strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                		printf("\tfdivd\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
                		trip_reg_map[tn] = trip_reg_map[arg1];
                	}
                	break;
		case '>':
		  if(mode == 'i'){
		    strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                    strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                    printf("\tsub\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
                    trip_reg_map[tn] = trip_reg_map[arg1];

		  }
		  else{
		    strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                    strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                    printf("\tfsubd\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
                    trip_reg_map[tn] = trip_reg_map[arg1];

		  }
		  break;
		case '<':
		  if(mode == 'i'){
			    strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
			    strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
			    printf("\tsub\t%%%s,%%%s,%%%s\n", reg2, reg1, reg1);
			    trip_reg_map[tn] = trip_reg_map[arg1];

		  }
		  else{
			    strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
			    strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
			    printf("\tfsubd\t%%%s,%%%s,%%%s\n", reg2, reg1, reg1);
			    trip_reg_map[tn] = trip_reg_map[arg1];

		  }
		  break;
	       case '|':
		 strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                 strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                 printf("\tor\t%%%s,%%%s,%%%s\n", reg1, reg2, reg2);
                 trip_reg_map[tn] = trip_reg_map[arg2];

		 break;
	       case '^':
		 strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                 strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                 printf("\txor\t%%%s,%%%s,%%%s\n", reg1, reg2, reg2);
                 trip_reg_map[tn] = trip_reg_map[arg2];

		 break;
	       case '&':
		 strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
		 strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
		 printf("\tand\t%%%s,%%%s,%%%s\n", reg1, reg2, reg2);
		 trip_reg_map[tn] = trip_reg_map[arg2];

	     	  break;
	       case '%':
		   strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
		   strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
		   printf("\tsdiv\t%%%s,%%%s,%%%s\n", reg1, reg2, reg2);
		   printf("\tsub\t%%%s,%%%s,%%%s\n", reg1, reg2, reg2);
		   trip_reg_map[tn] = trip_reg_map[arg2];
	    	  break;

            	default: 
            		printf("Error determining op.\n");
        	}
		}

		// Handle indexing
                else if(sscanf(S, "%d.\t[]%c %d %d", &tn, &mode, &arg1, &arg2) == 4){
                  if(mode == 'i'){
                    strcpy(reg1, convert_reg_alpha(trip_reg_map[arg1]));
                    strcpy(reg2, convert_reg_alpha(trip_reg_map[arg2]));
                    //printf("\tsll\t%%%s,2,%%%s\n", reg2, reg2);


		    printf("\tsll\t%%%s,2,%%%s\n", reg2, reg2 );
		    printf("\tadd\t%%%s,%%%s,%%%s\n",reg2, reg1, reg2);
                    //printf("\tld\t[%%%s+%%%s],%%%s\n", reg1, reg2, reg2);
		    //printf("\tld\t%%%s,%%%s\n", reg1, reg2);
		    trip_reg_map[tn] = trip_reg_map[arg2];
                  }
                  else{
                    strcpy(reg1, convert_reg_alpha(trip_reg_map[arg1]));
                    strcpy(reg2, convert_reg_alpha(trip_reg_map[arg2]));
		    // make it load to fp register
                    printf("\tsll\t%%%s,3,%%%s\n", reg2, reg2);
                    printf("\tadd\t%%%s,%%%s,%%%s\n",reg2, reg1, reg2);
                    //printf("\tldd\t[%%%s+%%%s],%%%s\n", reg2, reg1, reg2);
                    trip_reg_map[tn] = trip_reg_map[arg2];

                  }
                }


		// Handle two character ops
		else if(sscanf(S, "%d.\t%c%c%c %d %d", &tn, &op2_0, &op2_1, &mode, &arg1, &arg2) == 6){
		  switch(op2_0){
		  case '=':
		    if(mode == 'i'){
		      strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
		      strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
		      printf("\tsub\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
		      printf("\tsmul\t%%%s,%%%s,%%%s\n", reg1, reg1, reg1);
		      //printf("\tmov\t%d,%%f30\n", 0);
		      printf("\tsub\t%%%s,%%%s,%%%s\n", reg2, reg1, reg1);
		      trip_reg_map[tn] = trip_reg_map[arg1];

		    }
		    else{
		      strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
		      strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
		      printf("\tfsubd\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
		      printf("\tfmuld\t%%%s,%%%s,%%%s\n", reg1, reg1, reg1);
		      printf("\tfsubd\t%%%s,%%%s,%%%s\n", reg2, reg1, reg1);
		      trip_reg_map[tn] = trip_reg_map[arg1];
		    }
                    break;

		  case '!':
		    //printf("op2 is: %c%c\n", op2_0, op2_1);
		    //printf("mode is: %c\n", mode);
                    if(mode == 'i'){
                      strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                      strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                      printf("\tsub\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
                      printf("\tsmul\t%%%s,%%%s,%%%s\n", reg1, reg1, reg1);
                      trip_reg_map[tn] = trip_reg_map[arg1];
                    }
                    else{
                      strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                      strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                      printf("\tfsubd\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
                      printf("\tfmuld\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
                      trip_reg_map[tn] = trip_reg_map[arg1];
                    }

                    break;
		  case '<':
		    if(op2_1 == '<'){
		      strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                      strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                      printf("\tsll\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
		      trip_reg_map[tn] = trip_reg_map[arg1];
		    }
		    else{

		      if(mode == 'i'){
			strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
			strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
			printf("\tsub\t%%%s,%%%s,%%%s\n", reg2, reg1, reg1);
			printf("\tadd\t%%%s,1,%%%s\n", reg1, reg1);
			trip_reg_map[tn] = trip_reg_map[arg1];

		      }
		      else{
			strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
			strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
			printf("\tfsubd\t%%%s,%%%s,%%%s\n", reg2, reg1, reg1);
			printf("\tfaddd\t%%%s,1,%%%s\n", reg1, reg1);
			trip_reg_map[tn] = trip_reg_map[arg1];
		      }

		    }
                    break;
		  case '>':
		    if(op2_1 == '>'){
		      strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                      strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                      printf("\tsrl\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
                      trip_reg_map[tn] = trip_reg_map[arg1];
		    }
		    else{

		      if(mode == 'i'){
                        strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                        strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                        printf("\tsub\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
                        printf("\tadd\t%%%s,1,%%%s\n", reg1, reg1);
                        trip_reg_map[tn] = trip_reg_map[arg1];

                      }
                      else{
                        strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                        strcpy(reg2,convert_reg_alpha(trip_reg_map[arg2]));
                        printf("\tfsubd\t%%%s,%%%s,%%%s\n", reg1, reg2, reg1);
                        printf("\tfaddd\t%%%s,1,%%%s\n", reg1, reg1);
                        trip_reg_map[tn] = trip_reg_map[arg1];
                      }
		    }
                    break;
		  default:
		    printf("Error determining op2.\n");
		  }
		}

		// Handle inversion
		else if(sscanf(S, "%d.\t~%c %d", &tn, &mode, &arg1) == 3){
		  strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
		  printf("\torn\t%%g0,%%%s,%%%s", reg1, reg1);
		  trip_reg_map[tn] = trip_reg_map[arg1];
                }
		// Handle negation
		else if(sscanf(S, "%d.\t-%c %d", &tn, &mode, &arg1) == 3){
		  if(mode == 'i'){
                    strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
                    printf("\tsub\t%%g0,%%%s,%%%s\n", reg1, reg1);
                    trip_reg_map[tn] = trip_reg_map[arg1];

                  }
		  else{
		      strcpy(reg2,convert_reg_alpha(trip_reg_map[arg1]));
		      printf("\tfsubd\t%%g0,%%%s,%%%s\n", reg1, reg1);
		      trip_reg_map[tn] = trip_reg_map[arg1]; 
		  }
                }
		// Handle conditional branches
		else if(sscanf(S, "%d.\tbt %d B%d", &tn, &arg1, &arg2) == 3){
				for(i = 0; i < B_size; i++){
					if(B_values[i] == arg2){
						break;
					}
					else if(B_values[i] == 0){
						B_values[i] = arg2;
						break;
					}
				}
				strcpy(reg1,convert_reg_alpha(trip_reg_map[arg1]));
				if(reg1[0]!='f'){
				  printf("\tcmp\t%%%s,0\n",reg1);
				}
				else{
				  for(i = 0; i < 8; i++){
				    if(local_reg_map[i] == 0){
				      local_reg_map[i] = tn;
				      if(tn < trip_size){
					trip_reg_map[tn] = 16 + i;
				      }
				      else{
					double_trip_reg_map();
					trip_reg_map[tn] = 16 + i;
				      }
				      break;
				    }
				  }

				  printf("\tmov\t%d,%%l%d\n", con, i);
				  //strcpy(reg1, convert_reg_alpha(trip_reg_map[arg1]));
				  printf("\tst\t%%l%d,[%%sp + %d]\n", i, local_sp_offset);
				  printf("\tld\t[%%sp + %d],%%f%d \n", local_sp_offset, num_floats*2);
				  printf("\tfitod\t%%f%d,%%f%d \n", (num_floats*2), (num_floats*2));
				  trip_reg_map[tn] = 32 + (2*num_floats);
				  num_floats = num_floats + 1;
				  local_sp_offset = local_sp_offset - 8;

				  printf("\tfcmpd\t%%%s,%%f%d\n",reg1,(num_floats-1)*2);

				}
				//printf("\tsubcc\t%%%s,%%g0,%%g0\n", reg1);
				printf("\tbg\tB%d\n", arg2);
				printf("\tnop\n");
				
		}
		// Handle unconditional branches
		else if(sscanf(S, "%d.\tbr %c%d", &tn, &branch_arg, &arg1) == 3){
		  if(branch_arg == 'B'){
				for(i = 0; i < B_size; i++){
					if(B_values[i] == arg1){
						break;
					}
					else if(B_values[i] == 0){
						B_values[i] = arg1;
						break;
					}
				}
				printf("\tba\tB%d\n", arg1);
				printf("\tnop\n");
		  }
		  else{
		    printf("\tba\tL%d\n", arg1);
                    printf("\tnop\n");
		  }
		}
		// Handle allocation of global fname with arg1 bytes
		else if(sscanf(S, "%d.\talloc %s %d", &tn, fname, &arg1) == 3){
		  check_segflag(SEGDATA);
		  printf("\t.global\t%s\n", fname);
		  printf("\t.align\t8\n");
		  printf("%s:\n", fname);
		  /*for(i = 0; i < (arg1/4); i++){
		    printf("\t.word\t%d\n", i);
		    }*/
		  printf("\t.skip\t%d\n", arg1);
		  //printf("\t.common\t%s,%d,%d\n", fname, arg1, 8);
		  check_segflag(SEGTEXT);
		}
		// Handle label assignment
		else if(sscanf(S, "%d.\tB%d=L%d", &tn, &arg1, &arg2) == 3){
		  		for(i = 0; i < B_size; i++){
					if(B_values[i] == arg1){
						L_values[i] = arg2;
						break;
					}
					else if(B_values[i] == 0){
						printf("B value not found to set label.\n");
						break;
					}
				}
		}
		// Handle return function
		else if(sscanf(S, "%d.\tret%c %d", &tn, &mode, &arg1) == 3){
		  strcpy(reg1, convert_reg_alpha(trip_reg_map[arg1]));
		  if(mode == 'f'){
		    printf("\tldd\t[%%%s],%%f0\n", reg1);
		  }
		  else{
		    printf("\tmov\t%%%s,%%i0\n", reg1);
		  }
		  printf("\tret\n");
		  printf("\trestore\n");
                }
		// Handle end of function
		else if(sscanf(S, "%d.\tfend", &tn) == 1){
			printf("\tret\n");
			printf("\trestore\n");
		}
		// Handle invalid triple being found
		else{
			printf("Error: invalid triple found.\n");
		}
	}

	for(i = 0; i < B_size; i++){
		if(B_values[i] == 0){
			break;
		}
		printf("B%d:\n", B_values[i]);
		printf("\tba\tL%d\n", L_values[i]);
		printf("\tnop\n");
	}
	return 0;
}

void check_segflag(int seg){
	if(SEGFLAG != seg){
		SEGFLAG = seg;
	}
	if(SEGFLAG == SEGTEXT){
	  printf("\t.seg\t\"text\"\n");
        }
	else
	  printf("\t.seg\t\"data\"\n");

}

char *convert_reg_alpha(int reg){
	static char return_reg[2];
	static int mod_reg;
	if(reg < 8){
		return_reg[0] = 'g';
		mod_reg = reg;
		return_reg[1] = (char)(mod_reg + '0');
		return(return_reg);
	}
	else if(reg < 16){
		return_reg[0] = 'o';
		mod_reg = reg - 8;
		return_reg[1] = (char)(mod_reg + '0');
		return(return_reg);
	}
	else if(reg < 24){
		return_reg[0] = 'l';
		mod_reg = reg - 16;
		return_reg[1] = (char)(mod_reg + '0');
		return(return_reg);
	}
	else if(reg < 32){
		return_reg[0] = 'i';
		mod_reg = reg - 24;
		return_reg[1] = (char)(mod_reg + '0');
		return(return_reg);
	}
	else if(reg < 63){
		return_reg[0] = 'f';
		mod_reg = reg - 32; 
		return_reg[1] = (char)(mod_reg + '0');
		return(return_reg);
	}
}

// Double the size of the triple to register map
void double_trip_reg_map(){
  int i;
	temp_array = malloc(2 * trip_size * sizeof(int));
	memset(temp_array, 0, sizeof temp_array);
	for(i = 0; i < trip_size; i++){
		temp_array[i] = trip_reg_map[i];
	}
	free(trip_reg_map);
	trip_reg_map = temp_array;
	trip_size = 2*trip_size;
}

int modify_sp_offset(int offset){
  if(offset/8 != 0){
    offset = offset + (offset%8); 
  }
  return offset;
}


void assign_to_register(int tn){
  // Modify for spill into other regs                                                            
  int i;
  for(i = 0; i < 8; i++){                                                                        
    if(local_reg_map[i] == 0){                                                             
      local_reg_map[i] = tn;                                                         
      if(tn < (trip_size-10)){                                                            
	trip_reg_map[tn] = 16 + i;                                             
      }                                                                              
      else{                                                                          
	double_trip_reg_map();                                                 
	trip_reg_map[tn] = 16 + i;                                             
      }                                                                              
      break;                                                                         
    }                                                                                      
  }
  // cODE HERE FOR REG SPILL
                 
}

void assign_to_float_register(int tn){
  int i;
  for(i = 0; i < 16; i++){
    if(float_reg_map[i] == 0){
      float_reg_map[i] = tn;
      if(tn < (trip_size-10)){
	trip_reg_map[tn] = 32 + (2*i);
      }
      else{
	double_trip_reg_map();
	trip_reg_map[tn] = 32 + (2*i);
      } 
      break;
    }
  }
}
