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

uint32_t extract(uint32_t total,int start,int end) {
    uint32_t temp = total << start;
    //uint32_t cutoff = (uint32_t) temp;
    temp = temp >> (31-(end-start));
    return temp;
}

void statedump(State* s) {
    for(int i=0;i<32;i++){
        printf("GPR%d:%lx\n",i,s->gprs[i]);
    }
    for(int i=0;i<3;i++){
        printf("CR%d:%lx\n",i,s->cr[i]);
    }
    printf("PC:%lx\n",s->pc);
    printf("LR:%lx\n",s->lr);
}

uint32_t conditionPassed(State* s, uint32_t cond) {
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

uint32_t addressingMode(State* s, uint32_t inst) {
    uint32_t immAd = extract(inst, 4, 6);
    if (immAd == 1) {
        uint32_t immed8 = extract(inst, 24, 31);
        uint32_t rotate_imm = extract(inst, 20, 23);
    }
}

void run(State* s) {
    int run = 1;
    while (run) {
        uint32_t check = read32(s->mem, s->pc); 
        uint32_t cond = extract(check, 0, 3);
        uint32_t push1 = extract(check, 0, 16);
        uint32_t push2 = extract(check, 18, 23);
        uint32_t pop1 = extract(check, 0, 15);
        uint32_t pop2 = extract(check, 17, 23);
        uint32_t two7to26 = extract(check, 4, 5);
        uint32_t two4to21 = extract(check, 7, 10);
        uint32_t two7to21 = extract(check, 4, 10);
        uint32_t two7to20 = extract(check, 4, 9);
        uint32_t sevto4 = extract(check, 24, 27);
        uint32_t two4to20 = extract(check, 7, 11);
        uint32_t two7to25 = extract(check, 4, 6);
        uint32_t two7to24 = extract(check, 4, 7);
        if (push1 == 119386 && push2 == 0) {    // push

        } else if (pop1 == 59581 && pop2 == 0) {    // pop

        } else if (two7to20 == 18 && sevto4 == 1) { // bx
            uint32_t rm = extract(check, 28, 31);
            if (conditionPassed(s, cond)){
                s->pc = (s->gprs[rm] & 0xFFFFFFFE);
            }

        } else if (two7to21 == 4 && sevto4 == 9) {  // umull

        } else if (two7to21 == 0 && sevto4 == 9) {  // mul

        } else if (!two7to26 && two4to20 == 21) {   // cmp
            if (conditionPassed(check, cond)) {
                uint32_t alu_out
            }

        } else if (!two7to26 && two4to21 == 13) {   // mov and other forms
            // need to check I bit + bit[7] and bit[4] of shifter operand
            // (otherwise not a move)
            if (extract(check, 11, 11) == 0 && extract(check, 24, 24) == 1
                    && extract(check, 27, 27) == 1) {
                return;
            }

            uint32_t rd = extract(check, 16, 19);
            uint32_t shifterOperand = addressingMode(s, check);

            if (conditionPassed(check, cond)) {
                s->gprs[rd] = shifterOperand;
                if (extract(check, 11, 11) && rd == 15) {   // movs
                    s->cr = s->SPSR;    // PLACEHOLDER
                } else if (extract(check, 11, 11)) {
                    // SET FLAGS
                }
            }
        } else if (!two7to26 && two4to21 == 4) {    // add 
            // need to check I bit, bit[7] and bit[4] of shifter operand
            // (otherwise not an add)
            if (extract(check, 11, 11) == 0 && extract(check, 24, 24) == 1
                    && extract(check, 27, 27) == 1) {
                return;
            }

            uint32_t rn = extract(check, 12, 15);
            uint32_t rd = extract(check, 16, 19);
            uint32_t shifterOperand = addressingMode(s, check);

            if (conditionPassed(check, cond)) {
                s->gprs[rd] = s->gprs[rn] + shifterOperand;
                if (extract(check, 11, 11) && rd == 15) {   // if S bit is set, update cr
                    s->cr = s->SPSR;    // PLACEHOLDER
                } else if (extract(check, 11, 11)) {
                    // SET FLAGS
                }
            }

        } else if (!two7to26 && two4to21 == 2) {    // sub
            // need to check I bit, bit[7] and bit[4] of shifter operand
            // (otherwise not a sub)
            if (extract(check, 11, 11) == 0 && extract(check, 24, 24) == 1
                    && extract(check, 27, 27) == 1) {
                return;
            }

            uint32_t rn = extract(check, 12, 15);
            uint32_t rd = extract(check, 16, 19);
            uint32_t shifterOperand = addressingMode(s, check);

            if (conditionPassed(check, cond)) {
                rd = rn - shifterOperand;
                if (extract(check, 11, 11) && rd == 15) {
                    s->cr = s->SPSR;    // PLACEHOLDER
                } else if (extract(check, 11, 11)) {
                    // SET FLAGS
                }
            }

        } else if (!two7to26 && two4to21 == 3) {    // rsblt
            if(conditionPassed(s,cond)){
                uint32_t rn = extract(check,12,15);
                uint32_t rd = extract(check,16,19);
                uint32_t shifter = extract(check,20,31);
                s->gprs[rd] = shifter - s->gprs[rn];
            }
        } else if (two7to24 == 15) {//swi
            int sctype = s->gprs[7];
            if (sctype == 1) {  // exit
                run = 0;
            }
            else if (sctype == 4) { // print
                printf("%c",read8(s->mem,s->gprs[1]));
                s->pc += 4;
            }
        } else if (two7to25 == 4) {//stmfd and ldmfd

        } else if (two7to25 == 5) {//branches
            uint32_t L = extract(check,7,7);
            if (conditionPassed(s, cond)) {
                if (L)
                    s->lr = s->pc + 4;
                }
                uint32_t tempextend = extract(check,8,31);
                tempextend = tempextend << 8;
                int32_t extend = (int32_t) tempextend >> 8;
                extend = extend << 2;
                s->pc = s->pc + extend;
            }
        } else if (two7to26 == 1) {//ldr and str
            if(extract(check,11,11)){//ldr and ldrb
                if(!extract(check,10,10)){//ldr
                    
                }
                else{//ldrb
                }
            }
            else{
            }
        }
    }
}
