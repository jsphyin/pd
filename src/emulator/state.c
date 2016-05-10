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

uint32_t conditionpassed(State* s, uint32_t cond){
    switch(cond){
        case 0://eq
            return s->cr[1] == 1; 
        case 1://ne
            return s->cr[1] == 0;
        case 10://ge
            return s->cr[0] == s->cr[3];
        case 11://lt
            return s->cr[0] != s->cr[3];
        case 12://gt
            return ((s->cr[1] == 0) && (s->cr[0] == s->cr[3]));
        case 13://le
            return ((s->cr[1] == 1) && (s->cr[0] != s->cr[3]));
        case 14://always(unconditional
            return 1;
        default:
            return 0;
    }
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
        uint32_t two7to26 = extract(check,4,5);
        uint32_t two4to21 = extract(check,7,10);
        uint32_t two7to21 = extract(check,4,10);
        uint32_t two7to20 = extract(check,4,9);
        uint32_t sevto4 = extract(check,24,27);
        uint32_t two4to20 = extract(check,7,11);
        uint32_t two7to25 = extract(check,4,6);
        uint32_t two7to24 = extract(check,4,7);
        if(push1 == 119386 && push2 == 0){//push
        }
        else if(pop1 == 59581 && pop2 == 0){//pop
        }
        else if(two7to20 == 18 && sevto4 == 1){//bx
            uint32_t rm = extract(check,28,31);
            if(conditionpassed(s, cond)){
                s->pc = s->gprs[rm] & 0xFFFFFFFE;
            }
        }
        else if(two7to21 == 4 && sevto4 == 9){//umull
        }
        else if(two7to21 == 0 && sevto4 == 9){//mul
        }
        else if(!two7to26 && two4to20 == 21){//cmp
        }
        else if(!two7to26 && two4to21 == 13){//mov and other forms
        }
        else if(!two7to26 && two4to21 == 4){//add
        }
        else if(!two7to26 && two4to21 == 2){//sub
        }
        else if(!two7to26 && two4to21 == 3){//rsblt
        }
        else if(two7to24 == 15){//swi
        }
        else if(two7to25 == 4){//stmfd and ldmfd
        }
        else if(two7to25 == 5){//branches
            uint32_t L = extract(check,7,7);
            if(conditionpassed(s, cond)){
                if(L)
                    s->lr = s->pc + 4;
                uint32_t tempextend = extract(check,8,31);
                tempextend = tempextend << 8;
                int32_t extend = (int32_t) tempextend >> 8;
                extend = extend << 2;
                s->pc = s->pc + extend;
            }
        }
        else if (two7to26 == 1){//ldr and str
        }
    }
}
