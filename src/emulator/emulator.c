#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "memory.h"
#include "loader.h"
#include "state.h"
/*
int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr,"usage: %s <executable>\n",argv[0]);
        exit(-1);
    }
    const char* fileName = argv[1];

    int fd = open(fileName,O_RDONLY);
    if (fd == -1) {
        perror("open file");
        exit(-1);
    }

    Memory* memory = newMemory();
    uint32_t e = load(fd,memory);
    uint32_t pc = read32(memory,e)
    
    State* s = newState(memory);
    s->pc = pc;
    s->gprs[13] = 0x7eadbeef;
    s->lr = 0;
    run(s);
    return 0;
}*/
int main() {
    return 0;
}
