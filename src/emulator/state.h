#ifndef _STATE_H_
#define _STATE_H_

#include <stdint.h>
#include "memory.h"

typedef struct State{
    uint64_t gprs [32];
    uint64_t pc;
    uint64_t cr [3];
    uint64_t lr;
    Memory* mem;
} State;

extern State* newState(Memory* memory);

extern void run(State* state);

#endif
