    .global _start
_start:
    mov r0, #1234
    bl print_decimal
    //mov r1, #65

    /* exit syscall */
    mov r0, #0
    mov r7, #1
    swi #0


print_decimal:
    stmfd sp!, {r4, r5, lr}
    
    cmp r0, #0
    moveq r0, #'0'
    bleq putchar
    beq done

    mov r4, sp
    mov r5, sp
    sub sp, sp, #12

    rsblt r0, r0, #0
    movlt lr, #1
    movgt lr, #0

    ldr r1, =0x1999999a

loop: 
    umull r2, r3, r0, r1
    sub r2, r0, r3, lsl #3
    sub r2, r2, r3, lsl #1

    add r2, r2, #'0'
    strb r2, [r4, #-1]!

    movs r0, r3
    bne loop

    cmp lr, #0
    movne r0, #'-'
    blne putchar

write: 
    ldrb r0, [r4], #1
    bl putchar
    cmp r4, r5
    blt write

    add sp, sp, #12

done:
    ldmfd sp!, {r4, r5, lr}
    mov r0, #'\n'
    b putchar

/* argument is in r0 */
putchar:
    /*stmfd sp!, {r0, r1, r2, r7}*/

    ldr r1, =string
    str r0, [r1]
    mov r0, #1
    mov r2, #1
    mov r7, #4
    swi #0

    /*ldmfd sp!, {r0, r1, r2, r7}*/
    bx lr
    
    .data
string: .asciz ""
string2: .asciz ""
