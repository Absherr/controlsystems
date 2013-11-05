/*
 *  Created: 2013-10-30 16:13:59
 *   Author: Absherr
 */ 

// debug -> windows -> procesor view

// najprostsza petla
// START: RJMP START

// petla zwiekszajaca wartosc w rejestrze o jeden
/*
START: LDI R20, 1
.DEF ONE = R20
LDI R16, 0x00

LOOP:
	ADD R16, ONE
JMP LOOP
*/


/*
LDI R20, 0
LOOP:
   INC R16
   ADC R17, R20 // INC nie ustawia flagi C
JMP LOOP
*/

// zwiekszanie o jeden trzybajtowej liczby
/*
#include "m32def.inc"
LDI R20, 0
LDI R21, 1
.DEF ONE = R21
LDI R22, 0xFF
OUT DDRA, R22 // ustawienie calego portu A na wyjscie
LOOP:
   ADD R16, ONE // moze spowodowac ustawienie flagi Carry
   ADC R17, R20 
   ADC R18, R20 
   OUT PORTA, R18 // wypchniecie wartosci najwazniejszego bajtu wyniku na wyjscie
JMP LOOP
*/


// makro, ktore bedzie jako param przyjmowalo ilosc czasu w ms, ktory bedzie póŸniej oczekiwany
// 16M operacji na sekundê, obieg petli ma trwac jedna ms

/*
LDI R21, 1
.DEF ONE = R21

LDI R22, 0
.DEF ZERO = R22

.MACRO czekaj_ms
	LDI R18, @0
	LOOP:
		LDI R16, 0
		LOOP_MS: // 
			ADD R16, ONE
			ADC R17, ZERO
			BRCS END_MS
		JMP LOOP_MS // 3 cykle
		END_MS:
			DEC R18
			BREQ END
	END:
		NOP
.ENDM

*/

/*
#include "m32def.inc"
//LDI R20, 0
//LDI R21, 1
//.DEF ONE = R21
LDI R22, 0xFF
OUT DDRA, R22
LOOP_AAA:
   OUT PORTA, ZERO
   czekaj_ms 255
   OUT PORTA, ONE
   czekaj_ms 255
JMP LOOP_AAA
*/

START: 
	LDI R17, 255
	LDI R16, 254

LOOP:
	INC R16
	BRCS KONIEC
	SBC R17, R16
	JMP LOOP

KONIEC:
	JMP KONIEC