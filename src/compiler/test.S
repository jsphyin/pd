    .text
    .global _start
_start:
    mov r10, $104
    ldr r9, =x
    str r10, [r9]
    ldr r10, =x
    mov r0, r10

    bl putchar
    /*push {r7, lr}*/
    /* exit syscall */
    mov r0, $0
    mov r7, $1
    swi $0

putchar:
    mov r1, r0
    mov r0, $1
    mov r2, $1
    mov r7, $4
    svc $0
    
    bx lr

    .data
x:  .word 0
string: .asciz "Hello World\n"
string1: .asciz "a\n"
