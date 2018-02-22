ORG 0x0
.data
DD sp 
DD 2 DUP ? 
DD greska
DD tajmer
DD tastatura
DD 30 DUP ?
.text.0
out_reg DEF 0x80
in_reg DEF 0x84
.global START
START:
LOAD R0, #0
LOAD R1, #1
LOAD R2, #2
LOAD R3, #3
ADD R4, R3, R2
PUSH R1
PUSH R2
PUSH R3
MUL R6, R2, R3
POP R3
POP R2
POP R1
SUB R7, R6, R0
JMP START
.text.1
greska:
RET
.text.2
tastatura:
RET
.text.3
tajmer:
RET
.bss
sp:
DB 0x100 DUP ?
.end
