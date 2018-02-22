.data
pocetakdata:
a DEF 10
b DEF 20
c DEF 30
DB c - b
DB lab2
lab1:
e DEF 40
DD e-a-b
DD 8 DUP ?
krajdata:
.text
LOADUB R1, #1
LOADUB R2, R1
LOADUB R3, $5
ADD R1, R1, R1
SUB R2, R1, R1
lab2:
STOREB R4, [R2+e+b-a-c]
JZ R1, krajdata
JMP krajdata-pocetakdata+lab1
MUL R8, R8, R9
PUSH R10
PUSH R11
JLEZ R1, [R8]
INT R12
.end
