/***********************************************************/
/*             CDA3101 Project 1 - Assembler               */
/*             Solution by Caitlin Carnahan                */
/*                    Spring 2016                          */
/***********************************************************/
#include <stdio.h>
#include <string.h>

#define LINE_MAX 50     /* Max number of characters in a line */
#define FILE_MAX 100    /* Max number of lines in an assembly file */
#define ADD 32          /* Funct value for add */
#define NOR 39          /* Funct value for nor */
#define SLL 0           /* Funct value for sll */
#define J 2             /* Opcode value for j */
#define ADDI 8          /* Opcode value for addi */
#define LUI 15          /* Opcode value for lui */
#define SW 43           /* Opcode value for sw */
#define LW 35           /* Opcode value for lw */
#define BNE 5           /* Opcode value for bne */
#define ORI 13          /* Opcode value for ori */

char asm_lines[FILE_MAX][LINE_MAX];  /* Array of lines in assembly file */
char symbols[FILE_MAX][LINE_MAX];    /* Array of symbolic labels in file */
int symbol_line[FILE_MAX];           /* Line number of corresponding symbolic label */
unsigned int addresses[FILE_MAX];    /* Line i maps to address in addresses[i] */

int first_pass();
void second_pass(int);
void print_rtype(char*, char*, char*, char*, unsigned int);
void print_jtype(unsigned int);
void print_itype(char*, char*, char*, unsigned int);
unsigned int get_reg_num(char*);
int pc_relative(char*, int );
unsigned int pseudodirect(char*, int );
unsigned int lower_16(char* );
unsigned int upper_16(char* );

int main(){
  
  /* Perform a first pass over the assembly file, associating labels with lines, and lines with addresses */
  /* Perform a second pass over the assembly file, translating assembly into machine instructions */
  second_pass(first_pass());
  return 0;
}

int first_pass(){
  /* In the first pass of the assembly file, we associate all symbolic labels with line numbers
     and we associate all line numbers with addresses. The first instruction is at address 0. */
  int i = 0, num_bytes, curr_address = 0, symbol_index = 0;
  char temp_label[50], temp_line[50], operation[50], operands[50], reg[5], label[15];
  

  while(fgets(asm_lines[i], LINE_MAX, stdin) != NULL){     /*Pick up a line and place in asm_lines*/
    
    addresses[i] = curr_address;                           /* Assign next available address to line */
    
    if(sscanf(asm_lines[i], "%[^:]:%[^:]", temp_label, temp_line)==2){           /* Check if line contains label */
      strcpy(symbols[symbol_index], temp_label);           /* Copy the label into the symbol table*/
      strcpy(asm_lines[i], temp_line);                     /* Remove label from line */
      symbol_line[symbol_index] = i;                       /* Associate the label with the current line number */
      symbol_index += 1;                                   /* Increment the next available symbol_index */
    }

    /* Check for la pseudo-instruction and replace with lui and ori */
    if(sscanf(asm_lines[i], "\tla\t$%[^,],%[^\n]", reg, label) == 2){
      /*sprintf(asm_lines[i], "\tlui\t$%s,%s", reg, label);*/
      sprintf(asm_lines[i], "\tlui\t$at,%s", label);
      curr_address += 4;
      i+=1;
      /*sprintf(asm_lines[i], "\tori\t$%s,$%s,%s", reg, reg, label);*/
      sprintf(asm_lines[i], "\tori\t$%s,$at,%s", reg, label);
      addresses[i] = curr_address;
    }

    /* Check if line contains instruction or mem allocation and update address if necessary */
    if(strcmp(asm_lines[i], "\t.text\n") != 0 && strcmp(asm_lines[i], "\t.data\n") != 0){
      if(sscanf(asm_lines[i], "\t%[^.\t]\t%[^.]", operation, operands) == 2){ 
	curr_address += 4;
      }
      else if(sscanf(asm_lines[i], "\t.space\t%d", &num_bytes) == 1){
	curr_address += num_bytes;
      }
      else
	curr_address += 4;
    }

    i += 1;                                                /* Increment the line number */
  }

  return i; 
}

void second_pass(int num_lines){

  int i = 0;  /* Current line in assembly file */
  int instr_num = 0, immed;
  char operation[50], operands[50], rs[5], rt[5], rd[5], label[15];
  
  /* Iterate through the lines in the file */
  for(i = 0; i < num_lines; i++){

    /* Check to make sure that line is an instruction, not a directive */
    if(sscanf(asm_lines[i], "\t%[^.\t]\t%[^.]", operation, operands) == 2){

      /* Print out the address of the instruction in hex */
      printf("0x%.8X: ",addresses[i]);

      /* Translate the instruction and print in binary */

      /* Check if operation is R-type */
      if(strcmp(operation, "add") == 0 || strcmp(operation, "nor") == 0 || strcmp(operation, "sll") == 0){
	if(sscanf(operands, "$%[^,],$%[^,],$%[^,]", rd, rs, rt) == 3){
	  print_rtype(operation, rs, rt, rd, immed);
	}
	else if(sscanf(operands, "$%[^,],$%[^,],%d", rd, rt, &immed) == 3){
	  print_rtype(operation, rs, rt, rd, immed);
	}
      }
      else if(strcmp(operation, "j") == 0){           /* J-type */
	if(sscanf(operands, "%[^$,\n]", label) == 1){
	  print_jtype(pseudodirect(label, i));
	}
      }
      else{        /* I-type */
	if(sscanf(operands, "$%[^,],$%[^,],%d", rt, rs, &immed)==3){      /* addi */
	  print_itype(operation, rs, rt, immed);
	}
	else if(sscanf(operands, "$%[^,],$%[^,],%s", rs, rt, label) == 3){
	  if(strcmp(operation, "bne") == 0){
	    immed = pc_relative(label, i);
	    print_itype(operation, rs, rt, immed);

	  }
	  else if(strcmp(operation, "ori") == 0){
	    immed = lower_16(label); 
	    print_itype(operation, rt, rs, immed);
	  }
	  else{
	    print_itype(operation, rs, rt, immed);
	  }
	}
	else if(sscanf(operands, "$%[^,],%d($%[^)])", rt, &immed, rs) == 3){   /* lw and sw*/
	  print_itype(operation, rs, rt, immed);
	}
	else if(sscanf(operands, "$%[^,],%d", rt, &immed) == 2){
	  print_itype(operation, "0", rt, immed);
	}	  
	else if(sscanf(operands, "$%[^,],%s", rt, label) == 2){
	  immed = upper_16(label);
	  print_itype(operation, "0", rt, immed);
	}
      }
      instr_num += 1;
    }

  }

}

void print_rtype(char* op, char* rs, char* rt, char* rd, unsigned int shamt){
  int opcode = 0;
  int source1 = get_reg_num(rs), source2 = get_reg_num(rt), dest = get_reg_num(rd); 
  unsigned int funct;
  unsigned int instr = 0;

  if(strcmp(op, "add") == 0){
    funct = ADD;
    shamt = 0;
  }
  else if(strcmp(op, "nor") == 0){
    funct = NOR;
    shamt = 0;
  }
  else if(strcmp(op, "sll") == 0){
    source1 = 0;
    funct = SLL;
  }

  instr = (opcode << 26) + (source1 << 21) + (source2 << 16) + (dest << 11) + (shamt << 6) + funct;
  printf("0x%.8X\n", instr);

}


void print_jtype(unsigned int targaddr){

  unsigned int instr;
  instr = (J << 26) + (targaddr);
  printf("0x%.8X\n", instr);
  
}
void print_itype(char* op, char* rs, char* rt, unsigned int immed){
  unsigned int instr;
  unsigned int opcode, source1, source2;
  
  if(strcmp(op, "addi") == 0){
    opcode = ADDI;
    source1 = get_reg_num(rs);
    source2 = get_reg_num(rt);
  }
  else if(strcmp(op, "lui") == 0){
    opcode = LUI;
    source1 = get_reg_num(rs);
    source2 = get_reg_num(rt);
  }
  else if(strcmp(op, "lw") == 0){
    opcode = LW;
    source1 = get_reg_num(rs);
    source2 = get_reg_num(rt);
  }
  else if(strcmp(op, "sw") == 0){
    opcode = SW;
    source1 = get_reg_num(rs);
    source2 = get_reg_num(rt);
  }
  else if(strcmp(op, "bne") == 0){
    opcode = BNE;
    source1 = get_reg_num(rs);
    source2 = get_reg_num(rt); 
  }
  else if(strcmp(op, "ori") == 0){
    opcode = ORI;
    source1 = get_reg_num(rs);
    source2 = get_reg_num(rt);
  }

  instr = (opcode << 26) + (source1 << 21) + (source2 << 16) + ((immed <<16)>>16);
  printf("0x%.8X\n", instr);
 
  
}

int pc_relative(char* label, int line){
  /* Label is symbolic representation of instruction to jump to */
  /* line is the current instruction line */
  int i;
  
  /* Find the label in the symbol table and pull its associated line */
  for(i = 0; i < FILE_MAX; i++){
    if(strcmp(label, symbols[i])==0){
      break;
    }
  }

  return symbol_line[i] - line; 
}

unsigned int pseudodirect(char* label, int line){
   /* Label is symbolic representation of instruction to jump to */
   /* line is the current instruction line */
   int i;

   /* Find the label in the symbol table and pull its associated line */
   for(i = 0; i < FILE_MAX; i++){
     if(strcmp(label, symbols[i])==0){
       break;
     }
   }

   

   return (addresses[symbol_line[i]] << 4) >> 6;
 }
 

unsigned int get_reg_num(char* reg){
  if(reg[0] == 't'){
    return 8 + (reg[1] - '0');
  }
  else if(reg[0] == 's'){
    return 16 + (reg[1] - '0');
  }
  else if(reg[0] == 'a'){
    return 1;
  }
  
  return 0;
}

unsigned int lower_16(char* label){
  int i;
  for(i = 0; i < FILE_MAX; i++){
    if(strcmp(label, symbols[i])==0){
      break;
    }
  }
  return (addresses[symbol_line[i]] << 16) >> 16;
  
}

unsigned int upper_16(char* label){
  int i;
  for(i = 0; i < FILE_MAX; i++){
    if(strcmp(label, symbols[i])==0){
      break;
    }
  }
  return (addresses[symbol_line[i]]) >> 16;

}
