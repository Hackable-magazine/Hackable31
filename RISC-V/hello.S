.section .text
.balign 4
.globl main

.equ GPIO_BASE, 0x10012000
.equ GPIO_VALUE, 0x000
.equ GPIO_OUTPUT, 0x008
.equ GPIO_OUTPUT_VALUE, 0x00C

.equ RED,   1<<22
.equ GREEN, 1<<19
.equ BLUE,  1<<21

.equ LEDCONFIG, (RED | GREEN | BLUE)
.equ LEDCOLOR, ~GREEN
.equ LONGWAIT,  5000000
.equ SHORTWAIT, 1000000

ledConfig:
    addi sp, sp, -16
    sw   ra, 12(sp)
    lw t0, 0(s1)
    or t0, s3, t0
    sw t0, 0(s1)
    lw ra, 12(sp)  
    addi sp, sp, 16
    ret

setColor:
    addi sp, sp, -16
    sw   ra, 12(sp)
    xori t2, s3, -1
    lw t0, 0(s2)
    and t0, t2, t0
    or t0, a0, t0
    sw t0, 0(s2)    
    lw ra, 12(sp)  
    addi sp, sp, 16 
    ret

offLed:
    addi sp, sp, -16
    sw   ra, 12(sp)
    li t0, LEDCONFIG
    sw t0, 0(s2)    
    lw ra, 12(sp)  
    addi sp, sp, 16 
    ret

simpleWait:
    addi sp, sp, -16
    sw   ra, 12(sp)
    addi t0, x0, 1
waitloop:
    addi t0, t0, 1
    bltu t0, a0, waitloop
    lw ra, 12(sp)
    addi sp, sp, 16
    ret

main:
    li s1, GPIO_OUTPUT + GPIO_BASE
    li s2, GPIO_OUTPUT_VALUE + GPIO_BASE
    li s3, LEDCONFIG

    jal ra, ledConfig

mainloop:
    li a0, LEDCOLOR
    jal ra, setColor

    li a0, SHORTWAIT
    jal ra, simpleWait

    jal ra, offLed

    li a0, SHORTWAIT
    jal ra, simpleWait

    li a0, LEDCOLOR
    jal ra, setColor

    li a0, SHORTWAIT
    jal ra, simpleWait

    jal ra, offLed

    li a0, LONGWAIT
    jal ra, simpleWait

    j mainloop
