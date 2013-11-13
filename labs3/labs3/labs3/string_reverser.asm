.include "m32def.inc"

// inicjacja stosu
LDI	R16, low(RAMEND)
OUT	SPL, R16
LDI	R16, high(RAMEND)
OUT	SPH, R16

// miejsce wynikowe
.DSEG
result: .BYTE 100
   
.CSEG
// napis do odwrocenia
.ORG 300
NAPIS: .DB "HELLO WORLD", 0

// kopiowanie adresu napisu do Z
LDI ZL, LOW(NAPIS*2)
LDI ZH, HIGH(NAPIS*2)

// adres wynikowy do Y
LDI YL, LOW(result)
LDI YH, HIGH(result)

CALL REVERSE

// w Z jest napis we flashu
// w Y ma byc wynik
REVERSE:
	// robimy backup wszystkiego, czego bedziemy korzystac
	PUSH R16
	PUSH R17
	PUSH R18
	PUSH XL
	PUSH XH
	PUSH YL
	PUSH YH
	PUSH ZL
	PUSH ZH

	// tmp i tmp2 beda przechowywac odczytywane literki przy kopiowaniu jak i przy odwracaniu
	.DEF tmp = R16
	.DEF tmp2 = R17
	// len bedzie liczyl dlugosc napisu, a przy odwracaniu bedzie pamietal, ile swapow pozostalo do zrobienia
	.DEF len = R18

	// X bedzie oznaczal poczatek napisu
	MOV XL, YL
	MOV XH, YH
	
	COPY_LOOP:
		LPM tmp, Z+ // lecimy po kolejnych literach napisu
		SUBI tmp, 0 // sprawdzamy czy nie sa bajtem 0
		BREQ REVERSING
		ST Y+, tmp // jak nie sa, to zapisujemy do do kolejnych komorek
		INC len // i zliczamy je przy okazji
		JMP COPY_LOOP
	REVERSING:
		// w tym momencie mamy ustawiony rejestr len na dlugosc napisu
		// X wskazuje na pierwszy bajt napisu
		// Y wskazuje na 0

		SBIW Y, 1 ; przesuwamy Y na ostatni znak
		LSR len ; majac n znakowy napis, musimy zrobic floor(n/2) operacji zamiany znakow
		REVERSING_LOOP:

			// SWAP
			LD tmp, X ; znaki od poczatku
			LD tmp2, Y ; i znaki od konca
			ST X, tmp2 ;  zamieniamy z soba
			ST Y, tmp ;    po kolei

			SBIW Y, 1 // pointer konca przesuwamy w lewo
			ADIW X, 1 // pointer poczatku przesuwamy w prawo

			SUBI len, 1 // zmniejszamy licznik operacji, ktore pozostaly
			BREQ END

			JMP REVERSING_LOOP
	END:
		// sciagamy wszystko swoje ze stosu
		POP ZH
		POP ZL
		POP YH
		POP YL
		POP XH
		POP XL
		POP R18
		POP R17
		POP R16
		// i wracamy RETem
		RET