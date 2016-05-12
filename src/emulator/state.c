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
    for (int i=0;i<32;i++) {
        printf("GPR%d:%x\n", i, s->gprs[i]);
    }
    for (int i=0;i<3;i++) {
        printf("CR%d:%x\n", i, s->cr[i]);
    }
    printf("PC:%x\n",s->pc);
    printf("LR:%x\n",s->lr);
}

uint32_t conditionPassed(State* s, uint32_t cond) {
    switch(cond){
        case 0: //eq
            return s->cr[1] == 1; 
        case 1: //ne
            return s->cr[1] == 0;
        case 10:    //ge
            return s->cr[0] == s->cr[3];
        case 11:    //lt
            return s->cr[0] != s->cr[3];
        case 12:    //gt
            return ((s->cr[1] == 0) && (s->cr[0] == s->cr[3]));
        case 13:    //le
            return ((s->cr[1] == 1) && (s->cr[0] != s->cr[3]));
        case 14:    //always(unconditional
            return 1;
        default:
            return 0;
    }
}

uint32_t addressingMode(State* s, uint32_t inst, uint32_t* shift_c_out) {
    uint32_t immAd = extract(inst, 4, 6);
    if (immAd == 1) {
        // uint32_t immed8 = extract(inst, 24, 31);
        //uint32_t rotate_imm = extract(inst, 20, 23);
    }
    return 0;
}

uint32_t memAddress(State* s, uint32_t inst){
    uint32_t addtype = extract(inst,4,8);
    uint32_t typebit = extract(inst,10,10);
    uint32_t ubit = extract(inst,8,8);
    uint32_t rn = extract(inst,12,15);
    uint32_t offset = extract(inst,20,31);
    uint32_t cond = extract(inst, 0, 3);
    uint32_t result = 0;
    if(addtype == 5 && !typebit){//immediate offset
        if(ubit){
            result = s->gprs[rn] + offset;
        } else{
            result = s->gprs[rn] - offset;
        }
    }
    else if (addtype == 4 && !typebit){//immediate post-indexed
        result = s->gprs[rn];
        if(conditionPassed(s,cond)){
            if(ubit)
                s->gprs[rn] = s->gprs[rn] + offset;
            else
                s->gprs[rn] = s->gprs[rn] - offset;
        }
    }
    else if (addtype == 5 && typebit){//immediate pre-indexed
        if(ubit)
            result = s->gprs[rn] + offset;
        else
            result = s->gprs[rn] - offset;
        if(conditionPassed(s,cond))
            s->gprs[rn] = result;
    }
    return result;
}

uint32_t rotate(uint32_t value, int shift){
    return (value >> shift) | (value << (32 - shift));
}

uint32_t checkSign(uint32_t value) {
    return ((value >> 31) & 0x1);
}

uint32_t checkOverflow() {
    return 0;
}

void run(State* s) {
    int run = 1;
    while (run) {
        uint32_t inst = read32(s->mem, s->pc); 
        uint32_t cond = extract(inst, 0, 3);
        uint32_t push1 = extract(inst, 0, 16);
        uint32_t push2 = extract(inst, 18, 23);
        uint32_t pop1 = extract(inst, 0, 15);
        uint32_t pop2 = extract(inst, 17, 23);
        uint32_t two7to26 = extract(inst, 4, 5);
        uint32_t two4to21 = extract(inst, 7, 10);
        uint32_t two7to21 = extract(inst, 4, 10);
        uint32_t two7to20 = extract(inst, 4, 9);
        uint32_t sevto4 = extract(inst, 24, 27);
        uint32_t two4to20 = extract(inst, 7, 11);
        uint32_t two7to25 = extract(inst, 4, 6);
        uint32_t two7to24 = extract(inst, 4, 7);
        /*if (push1 == 119386 && push2 == 0) {    // push (stmdb)

          } else if (pop1 == 59581 && pop2 == 0) {    // pop (ldmia)

          } else */if (two7to20 == 18 && sevto4 == 1) { // bx
              uint32_t rm = extract(inst, 28, 31);
              if (conditionPassed(s, cond)) {
                  s->pc = (s->gprs[rm] & 0xFFFFFFFE);
              }

          } else if (two7to21 == 4 && sevto4 == 9) {  // umull
              uint32_t rdHi = extract(inst, 12, 15);
              uint32_t rdLo = extract(inst, 16, 19);
              uint32_t rs = extract(inst, 20, 23);
              uint32_t rm = extract(inst, 28, 31);

              if (conditionPassed(s, cond)) {
                  s->gprs[rdHi] = ((s->gprs[rm] * s->gprs[rs]) >> 32) & 0xFFFFFFFF;
                  s->gprs[rdLo] = (s->gprs[rm] * s->gprs[rs]) & 0xFFFFFFFF;
                  if (extract(inst, 11, 11)) {
                      s->cr[0] = (s->gprs[rdHi] >> 31) & 0x1;
                      s->cr[1] = (s->gprs[rdHi] == 0 && s->gprs[rdLo] == 0)
                          // other two flags are unaffected
                  }
              }

          } else if (two7to21 == 0 && sevto4 == 9) {  // mul
              uint32_t rd = extract(inst, 12, 15);
              uint32_t rs = extract(inst, 20, 23);
              uint32_t rm = extract(inst, 28, 31);

              if (conditionPassed(s, cond)) {
                  s->gprs[rd] = s->gprs[rm] * s->gprs[rs];
                  if (extract(inst, 11, 11)) {    // set flags
                      s->cr[0] = checkSign(s->gprs[rd]);
                      s->cr[1] = s->gprs[rd] == 0;
                      // other two flags are unaffected
                  }
              }

          } else if (!two7to26 && two4to20 == 21) {   // cmp
              if (extract(inst, 11, 11) == 0 && extract(inst, 24, 24) == 1
                      && extract(inst, 27, 27) == 1) {
                  return;
              }

              uint32_t shifterOperand = addressMode(s, inst, shift_c_out);
              uint32_t rn = extract(inst, 12, 15);

              if (conditionPassed(s, cond)) { // set flags
                  uint64_t aluOut = s->gprs[rn] - shifterOperand;
                  s->cr[0] = checkSign(aluOut);
                  s->cr[1] = aluOut == 0
                      s->cr[2] = aluOut > 0xFFFFFFFF;
                  // check overflow
                  s->cr[3] = (checkSign(s->[rn]) == 0 && checkSign(shifterOperand) == 1
                          && checkSign(aluOut) == 1) || (checkSign(s->[rn]) == 1
                          && checkSign(shifterOperand) == 0 && checkSign(aluOut) == 0);
              }

          } else if (!two7to26 && two4to21 == 13) {   // mov and other forms
              // need to check I bit + bit[7] and bit[4] of shifter operand
              // (otherwise not a move)
              if (extract(inst, 11, 11) == 0 && extract(inst, 24, 24) == 1
                      && extract(inst, 27, 27) == 1) {
                  return;
              }

              uint32_t rd = extract(inst, 16, 19);
              uint32_t *shift_c_out;
              uint32_t shifterOperand = addressingMode(s, inst, shift_c_out);

              if (conditionPassed(s, cond)) {
                  s->gprs[rd] = shifterOperand;
                  if (extract(inst, 11, 11) && rd == 15) {   // movs
                      //s->cr = s->SPSR;    // PLACEHOLDER
                  } else if (extract(inst, 11, 11)) { // set flags
                      s->cr[0] = checkSign(s->gprs[rd]);
                      s->cr[1] = s->gprs[rd] == 0;
                      s->cr[2] = &shift_c_out;
                      // overflow flag unaffected
                  }
              }
          } else if (!two7to26 && two4to21 == 4) {    // add 
              // need to check I bit, bit[7] and bit[4] of shifter operand
              // (otherwise not an add)
              if (extract(inst, 11, 11) == 0 && extract(inst, 24, 24) == 1
                      && extract(inst, 27, 27) == 1) {
                  return;
              }

              uint32_t rn = extract(inst, 12, 15);
              uint32_t rd = extract(inst, 16, 19);
              uint32_t *shift_c_out;
              uint32_t shifterOperand = aaddressingMode(s, inst, shift_c_out);

              if (conditionPassed(s, cond)) {
                  s->gprs[rd] = s->gprs[rn] + shifterOperand;
                  uint32_t carry = s->gprs[rn] + shifterOperand;
                  if (extract(inst, 11, 11) && rd == 15) {   // if S bit is set, update cr
                      //s->cr = s->SPSR;    // PLACEHOLDER
                  } else if (extract(inst, 11, 11)) { // set flags
                      s->cr[0] = checkSign(s->gprs[rd]);
                      s->cr[1] = s->gprs[rd] == 0;
                      s->cr[2] = carry > 0xFFFFFFFF;
                      // check overflow
                      s->cr[3] = checkSign(s->gprs[rn]) == 0 && checkSign(shifterOperand) == 0
                          && checkSign(s->gprs[rd] == 1) || (checkSign(s->gprs[rn]) == 1
                                  && checkSign(shifterOperand) && checkSign(s->gprs[rd]) == 0);
                  }
              }

          } else if (!two7to26 && two4to21 == 2) {    // sub
              // need to check I bit, bit[7] and bit[4] of shifter operand
              // (otherwise not a sub)
              if (extract(inst, 11, 11) == 0 && extract(inst, 24, 24) == 1
                      && extract(inst, 27, 27) == 1) {
                  return;
              }

              uint32_t rn = extract(inst, 12, 15);
              uint32_t rd = extract(inst, 16, 19);
              uint32_t *shift_c_out;
              uint32_t shifterOperand = addressMode(s, inst, shift_c_out);

              if (conditionPassed(s, cond)) {
                  s->gprs[rd] = s->gprs[rn] - shifterOperand;
                  uint64_t carry = s->gprs[rn] - shifterOperand;
                  if (extract(inst, 11, 11) && rd == 15) {
                      //s->cr = s->SPSR;    // PLACEHOLDER
                  } else if (extract(inst, 11, 11)) {
                      s->cr[0] = checkSign(s->gprs[rd]);
                      s->cr[1] = s->gprs[rd] == 0;
                      s->cr[2] = carry > 0xFFFFFFFF;
                      s->cr[3] = (checkSign(s->[rn]) == 0 && checkSign(shifterOperand) == 1
                              && checkSign(s->gprs[rd]) == 1) || (checkSign(s->[rn]) == 1
                              && checkSign(shifterOperand) == 0 && checkSign(s->gprs[rd]) == 0);
                  }
              }

          } else if (!two7to26 && two4to21 == 3) {    // rsblt
              if (conditionPassed(s,cond)) {
                  uint32_t rn = extract(inst,12,15);
                  uint32_t rd = extract(inst,16,19);
                  uint32_t shifter = extract(inst,20,31);
                  s->gprs[rd] = shifter - s->gprs[rn];
              }

          } else if (two7to24 == 15) {//swi
              int sctype = s->gprs[7];
              if (sctype == 1) {  // exit
                  run = 0;
              } else if (sctype == 4) { // print
                  printf("%c",read8(s->mem,s->gprs[1]));
                  s->pc += 4;
              }
          } else if (two7to25 == 4 && !extract(inst,9,9)) { //stmfd and ldmfd
              if(conditionPassed(s, cond)){
                  if(!extract(inst,11,11)){//stm

                  }
                  else{//ldm
                  }
              }
          } else if (two7to25 == 5) { //branches
              uint32_t L = extract(inst,7,7);
              if (conditionPassed(s, cond)) {
                  if (L) {
                      s->lr = s->pc + 4;
                  }
                  uint32_t tempextend = extract(inst,8,31);
                  tempextend = tempextend << 8;
                  int32_t extend = (int32_t) tempextend >> 8;
                  extend = extend << 2;
                  s->pc = s->pc + extend;
                  continue;
              }

          } else if (two7to26 == 1) { // ldr and str
              if(conditionPassed(s,cond)){
                  uint32_t address = memAddress(s,inst);
                  uint32_t rd = extract(inst,16,19);

                  if(extract(inst,11,11)){    // ldr and ldrb
                      if(!extract(inst,10,10)){   // ldr
                          uint32_t rotatebit = extract(address,30,31);
                          uint32_t value = 0;
                          switch(rotatebit){
                              case 0:
                                  value = read32(s->mem,address);
                                  break;
                              case 1:
                                  value = rotate(read32(s->mem,address),8);
                                  break;
                              case 2:
                                  value = rotate(read32(s->mem,address),16);
                                  break;
                              case 3:
                                  value = rotate(read32(s->mem,address),24);
                                  break;

                          }
                          s->gprs[rd] = value;
                      } else {    //ldrb
                          s->gprs[rd] = read8(s->mem,address);
                      }
                  } else {
                      if(!extract(inst,9,9)){//str
                          write32(s->mem,address,s->gprs[rd]);
                      } else{
                          write8(s->mem,address, extract(s->gprs[rd],24,31));
                      }

                  }
              }
          }
    s->pc += 4;
    }
