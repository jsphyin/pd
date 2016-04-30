    .syntax unifed
    .global main
main:
    push {ip, lr}

    ldr r0, =message
    bl printf

    mov r0, #0

    pop {ip, pc}

message:
    .asciz "Hello, world.\n"
