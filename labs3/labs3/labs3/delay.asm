 /*
 * delay.asm
 *
 *  Created: 2013-11-12 22:44:10
 *   Author: Absherr
 */ 

#include "m32def.inc"

.macro delay_ms
	// jedynka do iteracji w petlach
	LDI R22, 1
	.DEF ONE = R22
	
	// tutaj pamietamy ile ms mamy zaczekac
	LDI R16, @0
	.DEF ms_amount = R16

	// tutaj zliczamy ms, ktore juz zaczekalismy
	LDI R17, 0
	.DEF ms_counter = R17

	LOOP:
		ADD ms_counter, ONE
		// ms_loop czeka okolo 1ms
		ms_loop
		// sprawdzamy czy juz odczekalismy odpowiednia ilosc ms
		CP ms_counter, ms_amount
		BREQ END_LOOP
		JMP LOOP
	END_LOOP:
.endm

// ms_loop
// wywyoluje ona kilka razy internal_loop i dopelnia jakimis operacjami do trochêmniejniz16k cykli
// okolo 15995 cykli
.macro ms_loop
	// zlicza tutejsze iteracje
	LDI R18, 0
	.DEF ms_loop_counter = R18
	ms_loop_loop:
		ADD ms_loop_counter, ONE
		// chcemy wywolac internal_loop 13 razy
		CPI ms_loop_counter, 13 // + 1
		BREQ ms_loop_end // +1/+2
		internal_loop
		JMP ms_loop_loop
	ms_loop_end:
		// do zadanej ilosci cykli brakuje: okolo 470 cykli
		LDI ms_loop_counter, 0 // +1
		// wiec zrobimy 92 razy taka petle
		filling_LOOP:
			ADD ms_loop_counter, ONE // +1
			CPI ms_loop_counter, 92
			BREQ filling_END // +1/+2
			JMP filling_LOOP // +3
		filling_END:
			// i jeszcze dopelnimy NOPami
			NOP NOP NOP NOP	NOP NOP
.endm

// jeden call tego daje okolo 1280 cykle
.macro internal_loop
	LDI R19, 0 // +1
	.DEF internal_loop_counter = R19
	int_LOOP:
		ADD internal_loop_counter, ONE // +1
		BRCS int_END // +1/+2
		JMP int_LOOP // +3
	int_END:
		NOP
.endm

delay_ms 10
// w tym miejscu jest 160 001 cykli
// czyli o 1 cykl na 10ms za duzo
NOP