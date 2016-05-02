#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "parser.h"

int howManyBytes = 32;

typedef struct Var {
    char *name;
    struct Var *next;
} Var;

static Var *head;
static int counter;

int find(char *id) {
    Var *currentVar = head;
    if (head == NULL) { // empty list
        return 0;
    }

    while (currentVar != NULL) {
        if (!strcmp(currentVar->name, id)) {
            return 1;
        }
        currentVar = currentVar->next;
    }
    return 0;
}

void set(char *id) {
    if (head == NULL) { // empty list
        head = (Var*) malloc(sizeof(Var));
        head->name = id;
        head->next = NULL;
    } else if (find(id)) {  // already exists in the list
        return;
    } else {
        Var *currentVar = head;
        while (currentVar->next != NULL) {
            currentVar = currentVar->next;
        }
        currentVar->next = (Var*) malloc(sizeof(Var));
        currentVar->next->name = id;
        currentVar->next->next = NULL;
    }
}

int findFormal(Formals *p, char *name) {
    if (p == NULL) {
        return -1;
    }
    Formals *currentFormal = p;

    for (int i = 0; i < p->n; i++) {
        if (!strcmp(currentFormal->first, name)) {
            return i;
        }
        currentFormal = currentFormal->rest;
    }
    return -1;
}

// forward declarations
void genStatement(Statement *, Fun *);
void genActuals(Actuals *, Fun *);

// GPR push and pop
void printPush(int reg) {
    printf("    PUSH {r%d}\n", reg);
    //printf("    ADD R13, R13, #-8\n");
    //printf("    STR R%d, [R13]\n", reg);
    //printf("    stdu %d, -8(1)\n", reg);
}

void printPop(int reg) {
    printf("    POP {r%d}\n", reg);
    //printf("    LDR R%d, [R13]\n", reg);
    //printf("    ADD R13, R13, #8\n");
    //printf("    ld %d, 0(1)\n", reg);
    //printf("    addi 1, 1, 8\n");
}


void evalExpression(Expression *p, Fun *f) {
    if (p == NULL) {
        return;
    }

    int offset = 0;
    if (p->kind == eCALL && p->callActuals != NULL) {
        offset = p->callActuals->n;
    }

    switch(p->kind) {
        case eVAR:
            if (findFormal(f->formals, p->varName) != -1) { // formals found
                int index = findFormal(f->formals, p->varName);
		        printf("    LDR R10, [R12, $%d]\n",(index + 1) * 8);
                //printf("    ld 8, %d(7)\n", (index + 1) * 8);
            } else {
	            printf("    LDR R10, =%s\n", p -> varName);
                //printf("    ld 8, %s@toc(2)\n", p->varName);
            }
            break;
        case eVAL:
            printf("    EOR R10, R10, R10\n");
            //printf("    ");
	    //UNFINISHED
            /*printf("    xor 8, 8, 8\n");    // clear r8
            printf("    oris 8, 8, %" PRIu64 "@h\n", p->val);   // move upper 16 bits
            printf("    ori 8, 8, %" PRIu64 "@l\n", p->val);    // move lower 16 bits*/
            break;
        case ePLUS:
            evalExpression(p->left, f);
            printPush(9);
	        printf("    MOV R9, R10\n");
            //printf("    mr 25, 8\n");
            evalExpression(p->right, f);
	        printf("    ADD R10, R10, R9\n");
            //printf("    add 8, 8, 25\n");
            printPop(9);
            break;
        case eMUL:
            evalExpression(p->left, f);
            printPush(9);
	        printf("    MOV R9, R10\n");
            // printf("    mr 26, 8\n");
            evalExpression(p->right, f);
	        printf("    MUL R10, R10, R9\n");
            //printf("    mulld 8, 8, 26\n");
            printPop(9);
            break;
        case eEQ:
            evalExpression(p->left, f);
            printPush(9);
	        printf("    MOV R9, R10\n");
            //printf("    mr 27, 8\n");
            evalExpression(p->right, f);
	        printf("    CMP R10, R9\n");
            printf("    MOVEQ R10, $1\n");
            printf("    MOVNE R10, $0\n");
            /*printf("    cmp 0, 8, 27\n");
            printf("    mfcr 16\n");
            printf("    rlwinm 8, 16, 3, 31, 31\n");    // rotate CR 3 bits*/
            printPop(9);
            break;
        case eNE:
            evalExpression(p->left, f);
            printPush(9);
            printf("    MOV R9, R10\n");
            evalExpression(p->right, f);
            printf("    CMP R9, R10\n");
            printf("    MOVNE R10, $1\n");
            printf("    MOVEQ R10, $0\n");
            /*printf("    mfcr 17\n");
            printf("    rlwinm 17, 17, 3, 0, 31\n");    // rotate CR 3 bits
            printf("    not 17, 17\n");   // flip bits
            printf("    andi. 8, 17, 1\n");    // and bits with 1*/
            printPop(9);
            break;
        case eLT:
            evalExpression(p->left, f);
            printPush(9);
            printf("    MOV R9, R10\n");
            evalExpression(p->right, f);
            printf("    CMP R9, R10\n");
            printf("    MOVLT R10, $1\n");
            printf("    MOVGE R10, $0\n");
            /*printf("    mfcr 18\n");
            printf("    rlwinm 8, 18, 2, 31, 31\n");    // rotate CR 2 bits */
            printPop(9);
            break;
        case eGT:
            evalExpression(p->left, f);
            printPush(9);
            printf("    MOV R9, R10\n");
            evalExpression(p->right, f);
            printf("    CMP R9, R10\n");
            printf("    MOVGT R10, $1\n");
            printf("    MOVLE R10 $0\n");
            /*printf("    mfcr 19\n");
            printf("    rlwinm 8, 19, 1, 31, 31\n");    // rotate CR 1 bit */
            printPop(9);
            break;
        case eCALL:
            printPush(14);  // save link register
            /*printf("    mflr 6\n"); // save link 
            printPush(6);*/
            genActuals(p->callActuals, f);
            printPush(12);   // create stack frame/save base ptr
            printf("    MOV R12, R13\n");
            /*printPush(7);   // create stack frame/save base ptr
            printf("    mr 7, 1\n"); */
            if (!strcmp(p->callName, "_start")) {
                printf("    BL _%s\n", p->callName);
            } else {
                printf("    BL %s\n", p->callName);
            }
            printPop(12);
            printf("    ADD R13, R13, $%d\n", offset * 8);  // restore stack ptr
            printPop(14);   // restore link
            //printf("    mtlr 6\n"); // restore link
            break;
    }
}

void genActuals(Actuals *p, Fun *f) {
    if (p == NULL || p->n == 0) {
        return;
    }
    genActuals(p->rest, f);
    evalExpression(p->first, f);
    printPush(12);   // push args onto stack in reverse order
}

void genAssignment(Statement *p, Fun *f) {
    set(p->assignName); // store varnames in a linked list
    evalExpression(p->assignValue, f);
    printPush(9);
    printf("    LDR R9, =%s\n", p->assignName);
    printf("    STR R10, [R9]\n");
    //printf("    std 8, %s@toc(2)\n", p->assignName);
}

void genPrint(Expression *p, Fun *f) {
    evalExpression(p, f);
    printf("    mr 15, 8\n");   // provide arguments
    printf("    mflr 24\n");
    printPush(24);  // push lr onto the stack
    printf("    bl print\n");
    printPop(24);   // pop lr
    printf("    mtlr 24\n");
}

void genIf(Statement *p, Fun *f) {
    int count = ++counter;
    evalExpression(p->ifCondition, f);
    printf("if%d:\n", count);
    
    printf("    beq else%d\n", count);
    genStatement(p->ifThen, f);
    printf("    b done%d\n", count);
    printf("else%d:\n", count);
    genStatement(p->ifElse, f);
    printf("done%d:\n", count);
}

void genWhile(Statement *p, Fun *f) {
    int count = ++counter;
    printf("while%d:\n", count);
    evalExpression(p->whileCondition, f);
    printf("    cmpi 0, 8, 0\n");
    printf("    beq done%d\n", count);
    genStatement(p->whileBody, f);
    printf("    b while%d\n", count);
    printf("done%d:\n", count);
}

void genBlock(Block *p, Fun *f) {
    if (p == NULL || p->n == 0) {
        return;
    }
    genStatement(p->first, f);
    genBlock(p->rest, f);
}

void genReturn(Statement *p, Fun *f) {
    evalExpression(p->returnValue, f);  // return value is in r8
    if (!strcmp(f->name, "main")) {
        printf("    b exit\n");
    } else {
        printf("    blr\n");
    }
}

void genStatement(Statement *p, Fun *f)
{
    if (p == NULL) {
        return;
    }
    switch (p->kind) {
        case sAssignment:
            genAssignment(p, f);
            break;
        case sPrint:
            genPrint(p->printValue, f);
            break;
        case sIf:
            genIf(p, f);
            break;
        case sWhile:
            genWhile(p, f);
            break;
        case sBlock:
            genBlock(p->block, f);
            break;
        case sReturn:
            genReturn(p, f);
            break;
        case sBreak:
            break;
    }
}

void genFun(Fun *p)
{
    if (p == NULL) {
        return;
    }
    
    if (strcmp(p->name, "main") == 0) {
        printf("    .global %s\n", p->name);
        printf("%s:\n", p->name);
        genStatement(p->body, p);
        printf("    b exit\n"); // special case for main
    } else {
        printf("    .global %s_\n", p->name);
        printf("%s_:\n", p->name);
        genStatement(p->body, p);
        printf("    blr\n");
    }
}

void genFuns(Funs *p)
{
    if (p == NULL) {
        return;
    }
    genFun(p->first);
    genFuns(p->rest);
}

int main(int argc, char *argv[]) {
    Funs *p = parse();

    printf("    .text\n");
    genFuns(p);

    printf(".section \".toc\", \"aw\"\n");  // set up global vars
    while (head != NULL) {
        printf("%s:\n", head->name);
        printf("    .quad 0\n");
        head = head->next;
    }
    printf("stackBottom:\n");

    printf(".section \".opd\", \"aw\"\n");
    printf("    .global entry\n");
    printf("entry :\n");
    printf("    .quad main\n");
    printf("    .quad .TOC.@tocbase\n");
    printf("    .quad 0\n");

    return 0;
}
