// w Z ma byc adres napisu
// w X ma byc adres wyniku

.include "m32def.inc"


LDI	R16, low(RAMEND)
OUT	SPL, R16
LDI	R16, high(RAMEND)
OUT	SPH, R16


NAPIS: .db "HELLO WORLD", 0

LDI ZL, LOW(NAPIS*2)
LDI ZH, HIGH(NAPIS*2)

// zroib cseg
LDS R15, Z

PROC:
	
	
	RET