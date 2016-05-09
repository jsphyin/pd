#include "state.h"
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

State* newState(Memory* memory) {
    State* s =(State*) malloc(sizeof(State));
    s->mem = memory;
    for(int i=0;i<32;i++){
        s->gprs[i]=0;
    }
    for(int i=0;i<3;i++){
        s-> cr[i]=0;
    }
    return s;
}

uint32_t extract(uint32_t total,int start,int end){
    uint32_t temp = total << start;
    //uint32_t cutoff = (uint32_t) temp;
    temp = temp >> (31-(end-start));
    return temp;
}

void statedump(State* s){
    for(int i=0;i<32;i++){
        printf("GPR%d:%lx\n",i,s->gprs[i]);
    }
    for(int i=0;i<3;i++){
        printf("CR%d:%lx\n",i,s->cr[i]);
    }
    printf("PC:%lx\n",s->pc);
    printf("LR:%lx\n",s->lr);
}

void run(State* s) {
    int run =1;
    while(run){
        uint32_t check = read32(s->mem,s->pc); 
	uint32_t cond = extract(check,0,3);
	uint32_t push1 = extract(check,0,16);
	uint32_t push2 = extract(check,18,23);
	uint32_t pop1 = extract(check,0,15);
	uint32_t pop2 = extract(check,17,23);
        uint32_t 27to26 = extract(check,4,5);
        uint32_t 24to21 = extract(check,7,10);
        uint32_t 27to21 = extract(check,4,10);
        uint32_t 27to20 = extract(check,4,9);
        uint32_t 7to4 = extract(check,24,27);
        uint32_t 24to20 = extract(check,7,11);
        uint32_t 27to25 = extract(check,4,6);
        uint32_t 27to24 = extract(check,4,7);
	if(27to20 == 18 && 7to4 == 1){//bx
	}
        else if(27to21 == 4 && 7to4 == 9){//umull
	}
	else if(27to21 == 0 && 7to4 == 9){//mul
	}
	else if(
    }
}
