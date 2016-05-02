#include "memory.h"
#include <stdlib.h>
#include <stdio.h>

/* Not built for speed but hopefully easier to understand */
/* one byte at a time is not a good idea */

#define N 527

typedef struct Entry {
    struct Entry* next;
    uint64_t addr;
    uint8_t data;
} Entry;

struct Memory {
    Entry *chains[N];    
};

Memory* newMemory() {
    Memory* p = (Memory*) malloc(sizeof(*p));
    if (p == NULL) {
        perror("create memory");
        exit(-1);
    }

    for (int i=0; i<N; i++) {
        p->chains[i] = NULL;
    }
    return p;
}

void freeMemory(Memory* p) {
    if (p == NULL) return;
    for (int i=0; i<N; i++) {
        Entry* q = p->chains[i];
        while (q != NULL) {
            Entry* next = q->next;
            free(q);
            q = next;
       }
    }
    free(p);
}

static Entry* get(Memory* p, uint64_t addr) {
    Entry** first = &p->chains[addr % N];
    Entry *prev = NULL;

    Entry* q = *first;
    while (q != NULL) {
        if (q->addr == addr) {
            if (prev != NULL) {
                /* MTF */
                prev->next = q->next;
                q->next = *first;
                *first = q;
            }
            return q;
        }
        prev = q;
        q = q->next;
    }

    q = (Entry*) malloc(sizeof(*q));
    if (q == NULL) {
        perror("allocate entry");
        exit(-1);
    }
    q->addr = addr;
    q->data = 0;
    q->next = *first;
    *first = q;
    return q;
}
    
void write8(Memory* mem, uint64_t addr, uint8_t data) {
    Entry* e = get(mem,addr);
    e->data = data;
}

void write16(Memory* mem, uint64_t addr, uint16_t data) {
    uint8_t temp = data >> 8;
    write8(mem,addr,temp);
    write8(mem,addr+1,data);
}

void write32(Memory* mem, uint64_t addr, uint32_t data) {
    uint16_t temp = data >> 16;
    write16(mem,addr,temp);
    write16(mem,addr+2,data);
}

void write64(Memory* mem, uint64_t addr, uint64_t data) {
    uint32_t temp = data >> 32;
    write32(mem,addr,temp);
    write32(mem,addr+4,data);
}

uint8_t read8(Memory* mem, uint64_t addr) {
    Entry *e = get(mem,addr);
    return e->data;
}

uint16_t read16(Memory* mem, uint64_t addr) {
    uint16_t result = read8(mem,addr);
    result = result << 8;
    uint16_t secondhalf = read8(mem,addr+1);
    result = (result|secondhalf);
    return result;
}

uint32_t read32(Memory* mem, uint64_t addr) {
    uint32_t result = read16(mem,addr);
    result = result << 16;
    uint32_t secondhalf = read16(mem,addr+2);
    result = (result|secondhalf);
    return result;
}

uint64_t read64(Memory* mem, uint64_t addr) {
    uint64_t result = read32(mem,addr);
    result = result << 32;
    uint64_t secondhalf = read32(mem,addr+4);
    result = (result|secondhalf);
    return result;
}

