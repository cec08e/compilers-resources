#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMMEMORY 16 /* Maximum number of data words in memory */
#define NUMREGS 8    /* Number of registers */

/* Opcode values for instructions */
#define R 0
#define LW 35
#define SW 43
#define BNE 4
#define HALT 63

/* Funct values for R-type instructions */
#define ADD 32
#define SUB 34

/* Branch Prediction Buffer Values */
#define STRONGLYTAKEN 3
#define WEAKLYTAKEN 2
#define WEAKLYNOTTAKEN 1
#define STRONGLYNOTTAKEN 0

typedef struct IFIDStruct {
    unsigned int instr;
    int PCPlus4;
} IFIDType;

typedef struct IDEXStruct {
    unsigned int instr;
    int PCPlus4;
    int readData1;
    int readData2;
    int immed;
    int rsReg;
    int rtReg;
    int rdReg;
    int branchTarget;

    int ALUOp;
    int ALUSrc;
    int RegDst;
    int Branch;
    int MemRead;
    int MemWrite; 
    int RegWrite;
    int MemToReg;
} IDEXType;

typedef struct EXMEMStruct {
    unsigned int instr;
    int aluResult;
    int writeDataReg;
    int writeReg;

    int Branch;
    int MemRead;
    int MemWrite;
    int RegWrite;
    int MemToReg; 
} EXMEMType;

typedef struct MEMWBStruct {
    unsigned int instr;
    int writeDataMem;
    int writeDataALU;
    int writeReg;

    int RegWrite;
    int MemToReg;
} MEMWBType;

typedef struct stateStruct {
    int PC;
    unsigned int instrMem[NUMMEMORY];
    int dataMem[NUMMEMORY];
    int regFile[NUMREGS];
    IFIDType IFID;
    IDEXType IDEX;
    EXMEMType EXMEM;
    MEMWBType MEMWB;
    int cycles;
    int stalls;
    int branches;
    int m_branches;  
} stateType;


void run();
void printState(stateType*);
void initState(stateType*);
unsigned int instrToInt(char*, char*);
int get_opcode(unsigned int);
void printInstruction(unsigned int);
void stallPipeline(stateType*, stateType*);
void forwardUnit(stateType*);
void controlUnit(stateType*, stateType*);
void aluUnit(stateType*, stateType*);
void stallForBranch(stateType*, stateType*);
void clearForBranch(stateType*);
void zeroOutIDEX(stateType*);

int main(){
    run();
    return(0); 
}

void run(){
    int i;
    stateType state;
    stateType newState;
    initState(&state);

    /* Initialize branch prediction buffer to weakly not taken */
    int bpb[NUMMEMORY];
    for(i = 0; i < NUMMEMORY; i++)
        bpb[i] = WEAKLYNOTTAKEN;
    
    while (1) {

        printState(&state);

        if (get_opcode(state.MEMWB.instr) == HALT) {
            printf("Total number of cycles executed: %d\n", state.cycles);
            printf("Total number of stalls: %d\n", state.stalls);
            printf("Total number of branches: %d\n", state.branches);
            printf("Total number of mispredicted branches: %d\n", state.m_branches);
            exit(0);
        }

        newState = state;
        newState.cycles++;

        /* --------------------- IF stage --------------------- */

        newState.IFID.PCPlus4 = state.PC + 4;
        newState.IFID.instr = state.instrMem[state.PC/4];
	newState.PC = state.PC+4;

        /* --------------------- ID stage --------------------- */       
        if(state.MEMWB.RegWrite){
            if(state.MEMWB.MemToReg)
                newState.regFile[state.MEMWB.writeReg] = state.MEMWB.writeDataMem;
            else{
                newState.regFile[state.MEMWB.writeReg] = state.MEMWB.writeDataALU;
            }
        }

        /*newState.EXMEM.branchTarget = state.IDEX.PCPlus4 + (state.IDEX.immed<<2); */

	if (state.IFID.instr == 0){
	  /* Instruction is a noop */
	  zeroOutIDEX(&newState);
	  newState.IDEX.PCPlus4 = state.IFID.PCPlus4;
	  newState.IDEX.readData1 = newState.regFile[get_rs(state.IFID.instr)];
	  newState.IDEX.readData2 = newState.regFile[get_rt(state.IFID.instr)];
	  newState.IDEX.immed = get_immed(state.IFID.instr);
	  newState.IDEX.branchTarget = get_immed(state.IFID.instr);
	  newState.IDEX.rsReg = get_rs(state.IFID.instr);
	  newState.IDEX.rtReg = get_rt(state.IFID.instr);
	  newState.IDEX.rdReg = get_rd(state.IFID.instr);
	  
	}
        else if(state.IDEX.MemRead && (state.IDEX.rtReg == get_rt(state.IFID.instr) 
            || state.IDEX.rtReg == get_rs(state.IFID.instr))){
            stallPipeline(&newState, &state);
        }
	else{
            if(get_opcode(state.IFID.instr) == BNE){
                newState.branches += 1;
                if(bpb[(state.IFID.PCPlus4-4)/4] > 1){
                    stallForBranch(&newState, &state);
                }
            }
            newState.IDEX.instr = state.IFID.instr;
            newState.IDEX.PCPlus4 = state.IFID.PCPlus4;
            newState.IDEX.readData1 = newState.regFile[get_rs(state.IFID.instr)];
            newState.IDEX.readData2 = newState.regFile[get_rt(state.IFID.instr)];
            newState.IDEX.immed = get_immed(state.IFID.instr);
            newState.IDEX.branchTarget = get_immed(state.IFID.instr);
            newState.IDEX.rsReg = get_rs(state.IFID.instr);
            newState.IDEX.rtReg = get_rt(state.IFID.instr);
            newState.IDEX.rdReg = get_rd(state.IFID.instr);

            controlUnit(&newState, &state);
        }

        /* --------------------- EX stage --------------------- */

        newState.EXMEM.instr = state.IDEX.instr;
        
        forwardUnit(&state);
        
        newState.EXMEM.writeDataReg = state.IDEX.readData2;
        
        aluUnit(&newState, &state);
        if(state.IDEX.RegDst)
            newState.EXMEM.writeReg = state.IDEX.rdReg;
        else
            newState.EXMEM.writeReg = state.IDEX.rtReg;

        /*newState.EXMEM.Branch = state.IDEX.Branch;*/

        /* CHANGED TO BNE */

        if(state.IDEX.Branch && (newState.EXMEM.aluResult != 0)){
            /* Branch should have been taken */
            /* Check if it was taken */ 
            if(bpb[(state.IDEX.PCPlus4-4)/4] > WEAKLYNOTTAKEN){
                /* Branch was taken */
                bpb[(state.IDEX.PCPlus4-4)/4] = STRONGLYTAKEN; 
            }
            else{
                /* Branch was not taken! Clear IFID and IDEX and rewrite 
                PC with branch target. Update BPB */
                bpb[(state.IDEX.PCPlus4-4)/4] += 1;
                clearForBranch(&newState);
                newState.PC = state.IDEX.branchTarget;
                newState.m_branches += 1; 
                newState.stalls += 2;
            }
        }
        else if(state.IDEX.Branch && (newState.EXMEM.aluResult == 0)){
            /* Branch should not have been taken */
            /* Check if it was not taken */
            if(bpb[(state.IDEX.PCPlus4-4)/4] < WEAKLYTAKEN){
                /* Branch was not taken */
                bpb[(state.IDEX.PCPlus4-4)/4] = STRONGLYNOTTAKEN; 
            }
            else{
                /* Branch was taken! Clear IFID and IDEX and rewrite 
                PC with branch target. Update BPB */
                bpb[(state.IDEX.PCPlus4-4)/4] -= 1;
                clearForBranch(&newState);
                newState.PC = state.IDEX.PCPlus4;
                newState.m_branches += 1;    
                newState.stalls += 1;
            }
        }

        newState.EXMEM.MemRead = state.IDEX.MemRead;
        newState.EXMEM.MemWrite = state.IDEX.MemWrite;
        newState.EXMEM.MemToReg = state.IDEX.MemToReg;
        newState.EXMEM.RegWrite = state.IDEX.RegWrite;

        /* --------------------- MEM stage --------------------- */

        newState.MEMWB.instr = state.EXMEM.instr;
        newState.MEMWB.writeReg = state.EXMEM.writeReg;
        if(state.EXMEM.MemRead){
            newState.MEMWB.writeDataMem = state.dataMem[state.EXMEM.aluResult/4];
        }
        else if(state.EXMEM.MemWrite){
            newState.dataMem[state.EXMEM.aluResult/4] = state.EXMEM.writeDataReg;
        }
        
        newState.MEMWB.writeDataALU = state.EXMEM.aluResult;
        newState.MEMWB.MemToReg = state.EXMEM.MemToReg;
        newState.MEMWB.RegWrite = state.EXMEM.RegWrite;

        /* --------------------- WB stage --------------------- */

        state = newState;
    }
}

void zeroOutIDEX(stateType* newState){
  newState->IDEX.ALUOp = 0;
  newState->IDEX.ALUSrc = 0;
  newState->IDEX.RegDst = 0;
  newState->IDEX.Branch = 0;
  newState->IDEX.MemRead = 0;
  newState->IDEX.MemWrite = 0;
  newState->IDEX.RegWrite = 0;
  newState->IDEX.MemToReg = 0;
  newState->IDEX.instr = 0;
  /*newState->IDEX.PCPlus4 = 0;
  newState->IDEX.readData1 = 0;
  newState->IDEX.readData2 = 0;
  newState->IDEX.branchTarget = 0;
  newState->IDEX.immed = 0;
  newState->IDEX.rsReg = 0;
  newState->IDEX.rtReg = 0;
  newState->IDEX.rdReg = 0;*/
}

void stallForBranch(stateType* newState, stateType* state){
  /*newState->IFID.PCPlus4 = 0;*/
    newState->IFID.instr = 0; 
    newState->PC = get_immed(state->IFID.instr); 
    newState->stalls += 1;   
}

void clearForBranch(stateType* newState){
    newState->IFID.instr = 0;
    /*newState->IFID.PCPlus4 = 0;*/

    newState->IDEX.ALUOp = 0;
    newState->IDEX.ALUSrc = 0;
    newState->IDEX.RegDst = 0;
    newState->IDEX.Branch = 0;
    newState->IDEX.MemRead = 0;
    newState->IDEX.MemWrite = 0; 
    newState->IDEX.RegWrite = 0;
    newState->IDEX.MemToReg = 0;
    newState->IDEX.instr = 0;
    /*newState->IDEX.PCPlus4 = 0;
    newState->IDEX.branchTarget = 0;
    newState->IDEX.readData1 = 0;
    newState->IDEX.readData2 = 0;
    newState->IDEX.immed = 0;
    newState->IDEX.rsReg = 0;
    newState->IDEX.rtReg = 0;
    newState->IDEX.rdReg = 0;
    */
    

} 

void stallPipeline(stateType* newState, stateType* state){
    newState->IDEX.ALUOp = 0;
    newState->IDEX.ALUSrc = 0;
    newState->IDEX.RegDst = 0;
    newState->IDEX.Branch = 0;
    newState->IDEX.MemRead = 0;
    newState->IDEX.MemWrite = 0; 
    newState->IDEX.RegWrite = 0;
    newState->IDEX.MemToReg = 0;
    
    /*newState->IDEX.instr = 0;
    newState->IDEX.PCPlus4 = 0;
    newState->IDEX.branchTarget = 0;
    newState->IDEX.readData1 = 0;
    newState->IDEX.readData2 = 0;
    newState->IDEX.immed = 0;
    newState->IDEX.rsReg = 0;
    newState->IDEX.rtReg = 0;
    newState->IDEX.rdReg = 0;
    */

    newState->IDEX.instr = 0;
    newState->IDEX.PCPlus4 = state->IFID.PCPlus4;
    newState->IDEX.readData1 = newState->regFile[get_rs(state->IFID.instr)];
    newState->IDEX.readData2 = newState->regFile[get_rt(state->IFID.instr)];
    newState->IDEX.immed = get_immed(state->IFID.instr);
    newState->IDEX.branchTarget = get_immed(state->IFID.instr);
    newState->IDEX.rsReg = get_rs(state->IFID.instr);
    newState->IDEX.rtReg = get_rt(state->IFID.instr);
    newState->IDEX.rdReg = get_rd(state->IFID.instr);
    

    newState->IFID.PCPlus4 = state->PC;
    newState->IFID.instr = state->IFID.instr; 
    newState->PC = state->PC;    

    newState->stalls += 1;

}

void forwardUnit(stateType* state){

    /* fix to forward form memtoreg multiplexor */ 
    if(state->EXMEM.RegWrite && state->EXMEM.writeReg == state->IDEX.rsReg){
        state->IDEX.readData1 = state->EXMEM.aluResult;
    }
    else if(state->MEMWB.RegWrite && state->MEMWB.writeReg == state->IDEX.rsReg){
        if(state->MEMWB.MemToReg){
            state->IDEX.readData1 = state->MEMWB.writeDataMem;
        }
        else
            state->IDEX.readData1 = state->MEMWB.writeDataALU;
    }

    if(state->EXMEM.RegWrite && state->EXMEM.writeReg == state->IDEX.rtReg){
        state->IDEX.readData2 = state->EXMEM.aluResult;
    }
    else if(state->MEMWB.RegWrite && state->MEMWB.writeReg == state->IDEX.rtReg){
        if(state->MEMWB.MemToReg)
            state->IDEX.readData2 = state->MEMWB.writeDataMem;
        else
            state->IDEX.readData2 = state->MEMWB.writeDataALU;
    }

}

void controlUnit(stateType* newState, stateType* state){

    if(get_opcode(state->IFID.instr) == R){
        newState->IDEX.ALUOp = 2;
        newState->IDEX.ALUSrc = 0;
        newState->IDEX.RegDst = 1;
        newState->IDEX.Branch = 0;
        newState->IDEX.MemRead = 0;
        newState->IDEX.MemWrite = 0; 
        newState->IDEX.RegWrite = 1;
        newState->IDEX.MemToReg = 0;
    }
    else if(get_opcode(state->IFID.instr) == LW){
        newState->IDEX.ALUOp = 0;
        newState->IDEX.ALUSrc = 1;
        newState->IDEX.RegDst = 0;
        newState->IDEX.Branch = 0;
        newState->IDEX.MemRead = 1;
        newState->IDEX.MemWrite = 0; 
        newState->IDEX.RegWrite = 1;
        newState->IDEX.MemToReg = 1;
    }   
    else if(get_opcode(state->IFID.instr) == SW){
        newState->IDEX.ALUOp = 0;
        newState->IDEX.ALUSrc = 1;
        newState->IDEX.RegDst = 0;
        newState->IDEX.Branch = 0;
        newState->IDEX.MemRead = 0;
        newState->IDEX.MemWrite = 1; 
        newState->IDEX.RegWrite = 0;
        newState->IDEX.MemToReg = 0;
    }   
    else if(get_opcode(state->IFID.instr) == BNE){
        newState->IDEX.ALUOp = 1;
        newState->IDEX.ALUSrc = 0;
        newState->IDEX.RegDst = 0;
        newState->IDEX.Branch = 1;
        newState->IDEX.MemRead = 0;
        newState->IDEX.MemWrite = 0; 
        newState->IDEX.RegWrite = 0;
        newState->IDEX.MemToReg = 0;
    }
    else{
        newState->IDEX.ALUOp = 0;
        newState->IDEX.ALUSrc = 0;
        newState->IDEX.RegDst = 0;
        newState->IDEX.Branch = 0;
        newState->IDEX.MemRead = 0;
        newState->IDEX.MemWrite = 0; 
        newState->IDEX.RegWrite = 0;
        newState->IDEX.MemToReg = 0;

    }  
}

void aluUnit(stateType *newState, stateType *state){

    if(state->IDEX.ALUOp == 0){
        /* Perform an add operation */
      if(state->IDEX.instr == 0)
	newState->EXMEM.aluResult = state->IDEX.readData1 + state->IDEX.readData2;
      else
        newState->EXMEM.aluResult = state->IDEX.readData1 + state->IDEX.immed;
    }
    else if(state->IDEX.ALUOp == 1){
        /* Perform a sub operation */
        newState->EXMEM.aluResult = state->IDEX.readData1 - state->IDEX.readData2;
    }
    else{
        if(get_funct(state->IDEX.immed) == ADD){
            /* Perform an add operation */
            newState->EXMEM.aluResult = state->IDEX.readData1 + state->IDEX.readData2;
        }
        else{
            /* Perform a sub operation */ 
            newState->EXMEM.aluResult = state->IDEX.readData1 - state->IDEX.readData2;
        }

    }
}


void printState(stateType *statePtr)
{
    int i;
    printf("\n********************\nState at the beginning of cycle %d:\n", statePtr->cycles+1);
    printf("\tPC = %d\n", statePtr->PC);
    printf("\tData Memory:\n");
    for (i=0; i<(NUMMEMORY/2); i++) {
        printf("\t\tdataMem[%d] = %d\t\tdataMem[%d] = %d\n", 
            i, statePtr->dataMem[i], i+(NUMMEMORY/2), statePtr->dataMem[i+(NUMMEMORY/2)]);
    }
    printf("\tRegisters:\n");
    for (i=0; i<(NUMREGS/2); i++) {
        printf("\t\tregFile[%d] = %d\t\tregFile[%d] = %d\n", 
            i, statePtr->regFile[i], i+(NUMREGS/2), statePtr->regFile[i+(NUMREGS/2)]);
    }
    printf("\tIF/ID:\n");
    printf("\t\tInstruction: ");
    printInstruction(statePtr->IFID.instr);
    printf("\t\tPCPlus4: %d\n", statePtr->IFID.PCPlus4);
    printf("\tID/EX:\n");
    printf("\t\tInstruction: ");
    printInstruction(statePtr->IDEX.instr);
    printf("\t\tPCPlus4: %d\n", statePtr->IDEX.PCPlus4);
    printf("\t\tbranchTarget: %d\n", statePtr->IDEX.branchTarget);
    printf("\t\treadData1: %d\n", statePtr->IDEX.readData1);
    printf("\t\treadData2: %d\n", statePtr->IDEX.readData2);
    printf("\t\timmed: %d\n", statePtr->IDEX.immed);
    printf("\t\trs: %d\n", statePtr->IDEX.rsReg);
    printf("\t\trt: %d\n", statePtr->IDEX.rtReg);
    printf("\t\trd: %d\n", statePtr->IDEX.rdReg);
    printf("\tEX/MEM:\n");
    printf("\t\tInstruction: ");
    printInstruction(statePtr->EXMEM.instr);
    printf("\t\taluResult: %d\n", statePtr->EXMEM.aluResult);
    printf("\t\twriteDataReg: %d\n", statePtr->EXMEM.writeDataReg);
    printf("\t\twriteReg:%d\n", statePtr->EXMEM.writeReg);
    printf("\tMEM/WB:\n");
    printf("\t\tInstruction: ");
    printInstruction(statePtr->MEMWB.instr);
    printf("\t\twriteDataMem: %d\n", statePtr->MEMWB.writeDataMem);
    printf("\t\twriteDataALU: %d\n", statePtr->MEMWB.writeDataALU);
    printf("\t\twriteReg: %d\n", statePtr->MEMWB.writeReg);
}

void initState(stateType *statePtr)
{
    unsigned int dec_inst;
    int data_index = 0;
    int inst_index = 0;
    char line[130];
    char instr[6];
    char args[130];
    char* arg; 

    statePtr->PC = 0;
    statePtr->cycles = 0;
    statePtr->stalls = 0;
    statePtr->branches = 0;
    statePtr->m_branches = 0;
    memset(statePtr->dataMem, 0, 4*NUMMEMORY);
    memset(statePtr->instrMem, 0, 4*NUMMEMORY);
    memset(statePtr->regFile, 0, 4*NUMREGS);

    while(fgets(line, 130, stdin)){
       
        if(sscanf(line, "\t.%s %s", instr, args) == 2){
	   
            arg = strtok(args, ",");
            while(arg != NULL){
                statePtr->dataMem[data_index] = atoi(arg);
                data_index += 1;
                arg = strtok(NULL, ","); 
            }  
        }
        else if(sscanf(line, "\t%s %s", instr, args) == 2){
	    
            dec_inst = instrToInt(instr, args);
            statePtr->instrMem[inst_index] = dec_inst;
            inst_index += 1;
        }
	
    } 

    statePtr->IFID.instr = 0;
    statePtr->IFID.PCPlus4 = 0;
    statePtr->IDEX.instr = 0;
    statePtr->IDEX.PCPlus4 = 0;
    statePtr->IDEX.branchTarget = 0;
    statePtr->IDEX.readData1 = 0;
    statePtr->IDEX.readData2 = 0;
    statePtr->IDEX.immed = 0;
    statePtr->IDEX.rsReg = 0;
    statePtr->IDEX.rtReg = 0;
    statePtr->IDEX.rdReg = 0;
    statePtr->IDEX.ALUOp = 0;
    statePtr->IDEX.ALUSrc = 0;
    statePtr->IDEX.RegDst = 0;
    statePtr->IDEX.Branch = 0;
    statePtr->IDEX.MemRead = 0;
    statePtr->IDEX.MemWrite = 0; 
    statePtr->IDEX.RegWrite = 0;
    statePtr->IDEX.MemToReg = 0;
    statePtr->EXMEM.instr = 0;
    statePtr->EXMEM.aluResult = 0;
    statePtr->EXMEM.writeDataReg = 0;
    statePtr->EXMEM.writeReg = 0;
    statePtr->EXMEM.Branch = 0;
    statePtr->EXMEM.MemRead = 0;
    statePtr->EXMEM.MemWrite = 0; 
    statePtr->EXMEM.RegWrite = 0;
    statePtr->EXMEM.MemToReg = 0;
    statePtr->MEMWB.instr = 0;
    statePtr->MEMWB.writeDataMem = 0;
    statePtr->MEMWB.writeDataALU = 0;
    statePtr->MEMWB.writeReg = 0;
    statePtr->MEMWB.RegWrite = 0;
    statePtr->MEMWB.MemToReg = 0;
}

unsigned int instrToInt(char* inst, char* args){

    int opcode, rs, rt, rd, shamt, funct, immed;
    unsigned int dec_inst;
    
    if((strcmp(inst, "add") == 0) || (strcmp(inst, "sub") == 0)){
        opcode = 0;
        if(strcmp(inst, "add") == 0)
            funct = ADD;
        else
            funct = SUB; 
        shamt = 0; 
        rd = atoi(strtok(args, ",$"));
        rs = atoi(strtok(NULL, ",$"));
        rt = atoi(strtok(NULL, ",$"));
        dec_inst = (opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) + (shamt << 6) + funct;
    } else if((strcmp(inst, "lw") == 0) || (strcmp(inst, "sw") == 0)){
        if(strcmp(inst, "lw") == 0)
            opcode = LW;
        else
            opcode = SW;
        rt = atoi(strtok(args, ",$"));
        immed = atoi(strtok(NULL, ",("));
        rs = atoi(strtok(NULL, "($)"));
        dec_inst = (opcode << 26) + (rs << 21) + (rt << 16) + immed;
    } else if(strcmp(inst, "bne") == 0){
        opcode = 4;
        rs = atoi(strtok(args, ",$"));
        rt = atoi(strtok(NULL, ",$"));
        immed = atoi(strtok(NULL, ","));
        dec_inst = (opcode << 26) + (rs << 21) + (rt << 16) + immed;   
    } else if(strcmp(inst, "halt") == 0){
        opcode = 63; 
        dec_inst = (opcode << 26);
    } else if(strcmp(inst, "noop") == 0){
         dec_inst = 0;
    }
    return dec_inst;
}

int get_rs(unsigned int instruction){
    return( (instruction>>21) & 0x1F);
}

int get_rt(unsigned int instruction){
    return( (instruction>>16) & 0x1F);
}

int get_rd(unsigned int instruction){
    return( (instruction>>11) & 0x1F);
}

int get_funct(unsigned int instruction){
    return(instruction & 0x3F);
}

int get_immed(unsigned int instruction){
    return(instruction & 0xFFFF);
}

int get_opcode(unsigned int instruction){
    return(instruction>>26);
}

int get_targaddr(unsigned int instruction){
    return(instruction & 0x3FFFFFF);
}

void printInstruction(unsigned int instr)
{
    char opcodeString[10];
    if(instr == 0){
      printf("NOOP \n");
    } else if (get_opcode(instr) == R) {
        if(get_funct(instr)!=0){
            if(get_funct(instr) == ADD)
                strcpy(opcodeString, "add");
            else
                strcpy(opcodeString, "sub");
            printf("%s $%d,$%d,$%d\n", opcodeString, get_rd(instr), get_rs(instr), get_rt(instr));
        }
        else{
            printf("NOOP\n");
        }
    } else if (get_opcode(instr) == LW) {
        printf("%s $%d,%d($%d)\n", "lw", get_rt(instr), get_immed(instr), get_rs(instr));
    } else if (get_opcode(instr) == SW) {
        printf("%s $%d,%d($%d)\n", "sw", get_rt(instr), get_immed(instr), get_rs(instr));
    } else if (get_opcode(instr) == BNE) {
        printf("%s $%d,$%d,%d\n", "bne", get_rs(instr), get_rt(instr), get_immed(instr));
    } else if (get_opcode(instr) == HALT) {
        printf("%s\n", "halt");
    }
}

