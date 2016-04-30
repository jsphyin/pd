    .data
msg:
    .ascii "Hello, ARM World!\n"
    len = . - msg
string: .asciz "The number is: $d\n"

    .text
    .global _start
_start:
    /* write syscall 
    mov r0, #1
    ldr r1, =msg
    ldr r2, =len
    mov r7, #4
    swi #0 */

    /* testing print/global variables */
    mov r0, #1
    mov r3, #123

    ldr r1, =msg
    /*str r3, [r1]*/
    ldr r2, =len
    mov r7, #4
    swi #0

    /* printf ? 
    ldr r0, =string
    mov r1, #1024
    bl printf */

    /* exit syscall */
    mov r0, #0
    mov r7, #1
    swi #0
