    .text
    .global _start
_start:
    bl putchar
    /*push {r7, lr}*/
    /* exit syscall */
    mov r0, $0
    mov r7, $1
    swi $0

putchar:
    mov r0, $1
    ldr r1, =string1
    //mov r1, $66
    mov r2, $12
    mov r7, $4
    svc $0
    
    bx lr
    /*pop {r7, pc}*/
    
    /* exit syscall */
    /*mov r0, $0
    mov r7, $1
    swi $0*/

    .data
string: .asciz "Hello World\n"
string1: .asciz "a\n"
