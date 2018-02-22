ORG 0x100
.data
DD a
DD 8
lab:
DD 12
DB 16 DUP 1
a DEF 3 
b DEF 4
.text
LOAD R1, a*+b
ADD R1, R1, R1
.end
