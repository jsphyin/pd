#ifndef _STATE_H_
#define _STATE_H_

#include <stdint.h>
#include "memory.h"

typedef struct State{
    uint32_t gprs [32];
    uint32_t pc;
    uint32_t cr [3];
    uint32_t lr;
    Memory* mem;
} State;

extern State* newState(Memory* memory);

extern void run(State* state);

#endif
