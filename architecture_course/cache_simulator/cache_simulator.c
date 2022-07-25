#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void simulate();
int read_through(int*, short*, short*, unsigned int);
int read_back(int*, short*, short*, short*, unsigned int, int*);
int write_through(int*, short*, short*, unsigned int);
int write_back(int*, short*, short*, short*, unsigned int, int*);

int get_index(unsigned int); 
int get_tag(unsigned int);

int BLOCK_SIZE = 0;
int NUM_SETS = 0;
int BLOCKS_PER_SET = 0;
int MISS_PENALTY = 0;


int NUM_OFFSET = 0;  /*Number of offset bits*/
int NUM_INDEX = 0;

int main(int argc, char *argv[]){

	int i;

	if(argc != 7){
		printf("Usage: %s <args> < <tracefile>\n", argv[0]);
		printf("\t-b: block size in bytes.\n");
		printf("\t-s: number of sets.\n");
		printf("\t-n: associativity of cache.\n");
		/*printf("\t-m: miss penalty in cycles.\n");*/
		printf("Block size and number of sets must be postive integer powers of two.");
		return 0;
	}

	/* Parse command line arguments and echo back to user */
	for(i = 1; i < 6; i++){
		if(strcmp(argv[i], "-b") == 0){
			BLOCK_SIZE = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "-s") == 0){
			NUM_SETS = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "-n") == 0){
			BLOCKS_PER_SET = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "-m") == 0){
			MISS_PENALTY = atoi(argv[i+1]);
		}
	}

	if(BLOCK_SIZE < 1 || NUM_SETS < 1|| BLOCKS_PER_SET < 1){
		printf("Block size, number of sets, and associativity must be positive integers. \n"); 
		return 0;
	}

	NUM_OFFSET = ceil(log(BLOCK_SIZE)/log(2));
	NUM_INDEX = ceil(log(NUM_SETS)/log(2));


	printf("Block size: %d\n", BLOCK_SIZE);
	printf("Number of sets: %d\n", NUM_SETS);
	printf("Associativity: %d\n", BLOCKS_PER_SET);
	/*printf("Miss penalty (in cycles): %d\n", MISS_PENALTY);*/
	printf("Number of offset bits: %d\n", NUM_OFFSET);
	printf("Number of index bits: %d\n", NUM_INDEX);
	printf("Number of tag bits: %d\n", 32-(NUM_OFFSET+NUM_INDEX));

	simulate();

	return 0; 

}

void simulate(){
	/* Create two caches -- one for write-through and
	the other for write-back */
	int i, wt_hit = 0, wt_miss = 0, wb_hit = 0, wb_miss = 0;
	int wt_mem_ref = 0, wb_mem_ref = 0; 
	char type; 
	unsigned int address; 
	char line[40];
	int cache_size = NUM_SETS*BLOCKS_PER_SET; 
	int *wt_cache = malloc(sizeof(int)*cache_size);
	int *wb_cache = malloc(sizeof(int)*cache_size);

	short *wt_valid = malloc(sizeof(short)*cache_size);
	short *wb_valid = malloc(sizeof(short)*cache_size);

	short *wt_use = malloc(sizeof(short)*cache_size);
	short *wb_use = malloc(sizeof(short)*cache_size);

	short *wb_dirty = malloc(sizeof(short)*cache_size);

	memset(wt_use, 2, sizeof(short)*cache_size);
	memset(wb_use, 2, sizeof(short)*cache_size);
	//printf("size of wtuse si %d \n", sizeof(*wt_use));

	/*
	for(i = 0; i < cache_size; i++){
	  wt_use[i] = 5;
	  wb_use[i] = 5;
	}
	*/


	memset(wt_valid, 0, sizeof(short)*cache_size);
	memset(wb_valid, 0, sizeof(short)*cache_size); 
	memset(wb_dirty, 0, sizeof(short)*cache_size); 

	while(fgets(line, 40, stdin)){
		/* Reading trace file line by line */
		if(sscanf(line, "%c %u", &type, &address)){
			printf("Type is %c and address is %u\n", type, address);
			printf("Address is %u and tag is %d and index is %d\n", address, get_tag(address), get_index(address));
			if(type == 'R'){
				/* Read access being performed */
				if(read_through(wt_cache, wt_valid, wt_use, address)){
					wt_hit++;
					//printf("Read hit.\n");
				}
				else{
				  //printf("Read miss\n");
					wt_miss++;
					wt_mem_ref++;
				}

				if(read_back(wb_cache, wb_valid, wb_use, wb_dirty, address, &wb_mem_ref)){
					wb_hit++;
					printf("Read hit \n");
				}
				else{
					wb_miss++;
					printf("Read miss \n");
					wb_mem_ref++;
				}
			}
			else if(type == 'W'){
				/* Write access being performed */
				if(write_through(wt_cache, wt_valid, wt_use, address)){
				  /*printf("Write hit\n");*/
					wt_hit++;
					wt_mem_ref++; /* Update memory as well */
				}
				else{
					/*printf("Write miss\n");*/
					wt_miss++;
					wt_mem_ref++; /*Write through to memory */
				}
				if(write_back(wb_cache, wb_valid, wb_use, wb_dirty, address, &wb_mem_ref)){
					wb_hit++; /* Do not update memory */
					printf("write hit \n");
				}
				else{
					wb_miss++;
					wb_mem_ref++; /* Grab block from memory */
					printf("write miss \n");
				}
			}
			else{
				printf("Invalid line in trace file.\n");
			}

		}
		else{
			printf("Invalid line in trace file.\n");
		}
		
		for(i = 0; i < cache_size; i++){
			wt_use[i]++;
			wb_use[i]++;
		}

	}

	printf("\n****************************************\n");
	printf("Write-through with No Write Allocate\n");
	printf("****************************************\n");
	printf("Total number of references: %d\n", wt_hit+wt_miss);
	printf("Hits: %d\n", wt_hit);
	printf("Misses: %d\n", wt_miss);
	printf("Memory References: %d\n", wt_mem_ref);
	/*printf("Average access time: %.3f cycles\n", 1 + MISS_PENALTY*(1.0*wt_miss/(wt_miss+wt_hit)));*/
	/* Average access time */

	printf("\n****************************************\n");
	printf("Write-back with Write Allocate\n");
	printf("****************************************\n");
	printf("Total number of references: %d\n", wb_hit+wb_miss);
	printf("Hits: %d\n", wb_hit);
	printf("Misses: %d\n", wb_miss);
	printf("Memory References: %d\n", wb_mem_ref);
	/*printf("Average access time: %.3f cycles\n", 1 + MISS_PENALTY*(1.0*wb_miss/(wb_miss+wb_hit)));*/

}

int read_through(int *cache, short *valid, short *use, unsigned int address){
	int index = get_index(address);
	int tag = get_tag(address);
	int i;
	int LRU_use = 0;
	int base = index*BLOCKS_PER_SET;
	int LRU_index = base; 


	//printf("Index is %d\n", index);
	
	/* The block should be found at set index. */
	/* Now, find the block in the set which has a matching tag */
	for(i = 0; i < BLOCKS_PER_SET; i++){
	  //printf("base is %d\n", base);
	  //printf("base+%d: valid is %d and tag is %d\n", i, valid[base+i], cache[base+i]);
		if(valid[base+i] == 1 && cache[base+i] == tag){
			use[base+i] = 0;
			return 1;
		}
		//printf("Use of block %d is %d\n", i, use[base+i]);
		if(use[base+i] > LRU_use){
			LRU_use = use[base+i];
			LRU_index = base + i; 
		}
	}

	if( i == BLOCKS_PER_SET ) {
		/* MISS - Replace least recently used. */
		/* LRU is LRU_index */
		/*printf("Least Recently Used index is %d\n", LRU_index);*/
		cache[LRU_index] = tag;
		valid[LRU_index] = 1;
		use[LRU_index] = 0; 
	}
	return 0;

}


int read_back(int *cache, short *valid, short *use, short *dirty, unsigned int address, int* mem_ref){
	int index = get_index(address);
	int tag = get_tag(address);
	int i;
	int LRU_use = 0;
	int base = index*BLOCKS_PER_SET;
	int LRU_index = base; 


	
	/* The block should be found at set index. */
	/* Now, find the block in the set which has a matching tag */

	for(i = 0; i < BLOCKS_PER_SET; i++){
	  printf("base+%d: valid is %d and tag is %d\n", i, valid[base+i], cache[base+i]); 
          //printf("use is %d \n", use[base+i]);
		if(valid[base+i] == 1 && cache[base+i] == tag){
			use[base+i] = 0;
			return 1;
		}
		if(use[base+i] > LRU_use){
			LRU_use = use[base+i];
			LRU_index = base + i; 
		}
	}

	if( i == BLOCKS_PER_SET ) {
		/* MISS - Replace least recently used. */
		/* LRU is LRU_index */

	  //printf("LRU is %d n", LRU_index);
		if(valid[LRU_index] && dirty[LRU_index]){
			/* We are replacing a legit block */
			(*mem_ref)++; 
		}
		cache[LRU_index] = tag;
		valid[LRU_index] = 1;
		use[LRU_index] = 0; 
		dirty[LRU_index] = 0;

	}
	return 0;

}

int write_through(int* cache, short* valid, short* use, unsigned int address){
	int index = get_index(address);
	int tag = get_tag(address);
	int i;
	int LRU_use = 0;
	int base = index*BLOCKS_PER_SET;
	int LRU_index = base;


	/*printf("Index is %d\n", index);*/


	for(i = 0; i < BLOCKS_PER_SET; i++){
		if(valid[base+i] == 1 && cache[base+i] == tag){
			use[base+i] = 0;
			return 1;
		}
		/*
		if(use[index+i] > LRU_use){
			LRU_use = use[index+i];
			LRU_index = index + i; 
		}
		*/
	}

	/*
	if( i == BLOCKS_PER_SET ) {
		cache[LRU_index] = tag;
		valid[LRU_index] = 1;
		use[LRU_index] = 0; 
	}
	*/
	return 0;

}

int write_back(int* cache, short* valid, short* use, short* dirty, unsigned int address, int* mem_ref){
	int index = get_index(address);
	int tag = get_tag(address);
	int i;
	int LRU_use = 0;
	int base = index*BLOCKS_PER_SET;
	int LRU_index = base;


	/*printf("Index is %d\n and base is %d\n", index, base);*/

	for(i = 0; i < BLOCKS_PER_SET; i++){
	  printf("base+%d: valid is %d and tag is %d\n", i, valid[base+i], cache[base+i]);
		if(valid[base+i] == 1 && cache[base+i] == tag){
			use[base+i] = 0;
			dirty[base+i] = 1;
			return 1;
		}
		if(use[base+i] > LRU_use){
			LRU_use = use[base+i];
			LRU_index = base + i;
		}
	}

	if( i == BLOCKS_PER_SET ) {
		/* MISS - Replace least recently used. */
		/* LRU is LRU_index */
		if(valid[LRU_index] && dirty[LRU_index]){
			/* We are replacing a legit block */
			(*mem_ref)++; 
		}
		cache[LRU_index] = tag;
		valid[LRU_index] = 1;
		use[LRU_index] = 0;
		dirty[LRU_index] = 1; 
	}
	return 0;
}


int get_index(unsigned int address){
	return (address>>NUM_OFFSET)%NUM_SETS;
}

int get_tag(unsigned int address){
	return (address>>(NUM_INDEX+NUM_OFFSET));
}
