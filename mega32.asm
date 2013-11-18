// Komentarz jednoliniowy
/* Komentarz
 * w kilku
 * liniach */


;Wylaczenie listowania kodu.
.NOLIST

;Dolaczenie zewnetrnych plikÛw
#include "m32def.inc"
;wlaczenie listowania - musi byc zaznaczona
;odpowiednia opcja w projekcie
.LIST
;rozwijanie makr w pliku z listingiem
.LISTMAC

;Preprocesor znany z jezyka C 
#define __TEST__
#ifdef __TEST__
;wyprowadzenie komunikatu do okna kompilacji
.warning "__TEST__ already defined!"
#else
.warning "__TEST__ not defined yet!"
;Wyúwietla komunikat jaka blad kompilacji
#error   "Nazwa kontrolera " __PART_NAME__
#endif


;EEPROM DATA SEGMENT
.ESEG
.org 0
; Wszystkie dane w obrebie segmentu beda zaprogramowane w pamieci eeprom

serial: 	.db 0x12,\
            	0x34,\
				0x56,\
				0x78

wrd:		.dw 0x1234, 0x5678
	
binary:		.db 0b0101_1010		;zapis biinarny	
decimal:	.db 90				;zapis dziesietny	
octal:		.db 0132			;zapis oktalny (Ûsemkowy)



;SRAM DATA
.DSEG
;w segmencie danych nie ma mozliwosci inicjowania zmiennych wartosciami

tab: .BYTE 16			;tablica bajtowa o rozmiarze 16 bajtow


;DEFS
.DEF tmp=R16			;assigns R16 register to tmp symbol
.EQU maxByte=255

;HARDCODED CODE DATA


.CSEG
;stale zdefiniowane w segmencie kodu zapisane zostana w pamieci flash
.org FLASHEND - 0x100			;stale nalezy umieszczac pod koniec 
								;pamieci w celu unikniecia kolizji z kodem
bitmask:  	.db 0b1010_1010, 0b0101_0101
wbitmask: 	.dw 0b10101010_01010101
dbitmask: 	.dd 0x12345678
qbitmask: 	.dq 0x123456789abcdef0

;Napis nie jest domyslnie zakonczony zerem - trzeba je dododac samemu
hw:			.db "Hello World!",0
; definicja makra
.MACRO makro
     LDI R18,@0
.ENDM

.org 0
	jmp    START
	jmp	NO_IRQ ;INT0addr	= 0x0002	; External Interrupt Request 0
	jmp	NO_IRQ ;INT1addr	= 0x0004	; External Interrupt Request 1
	jmp	NO_IRQ ;INT2addr	= 0x0006	; External Interrupt Request 2
	jmp	NO_IRQ ;OC2addr	    = 0x0008	; Timer/Counter2 Compare Match
	jmp	NO_IRQ ;OVF2addr	= 0x000a	; Timer/Counter2 Overflow
	jmp	NO_IRQ ;ICP1addr	= 0x000c	; Timer/Counter1 Capture Event
	jmp	NO_IRQ ;OC1Aaddr	= 0x000e	; Timer/Counter1 Compare Match A
	jmp	NO_IRQ ;OC1Baddr	= 0x0010	; Timer/Counter1 Compare Match B
	jmp	NO_IRQ ;OVF1addr	= 0x0012	; Timer/Counter1 Overflow
	jmp	NO_IRQ ;OC0addr	    = 0x0014	; Timer/Counter0 Compare Match
	jmp	NO_IRQ ;OVF0addr	= 0x0016	; Timer/Counter0 Overflow
	jmp	NO_IRQ ;SPIaddr	    = 0x0018	; Serial Transfer Complete
	jmp	NO_IRQ ;URXCaddr	= 0x001a	; USART, Rx Complete
	jmp	NO_IRQ ;UDREaddr	= 0x001c	; USART Data Register Empty
	jmp	NO_IRQ ;UTXCaddr	= 0x001e	; USART, Tx Complete
	jmp	NO_IRQ ;ADCCaddr	= 0x0020	; ADC Conversion Complete
	jmp	NO_IRQ ;ERDYaddr	= 0x0022	; EEPROM Ready
	jmp	NO_IRQ ;ACIaddr	    = 0x0024	; Analog Comparator
	jmp	NO_IRQ ;TWIaddr	    = 0x0026	; 2-wire Serial Interface
	jmp	NO_IRQ ;SPMRaddr	= 0x0028	; Store Program Memory Ready

.org INT_VECTORS_SIZE
START:
	; inicjowanie stosu
	ldi tmp,low(RAMEND)	; Load low byte address of end of RAM into register R16
	out	SPL,tmp			; Initialize stack pointer to end of internal RAM
	ldi tmp,high(RAMEND); Load high byte address of end of RAM into register R16	
	out SPH, tmp		; Initialize high byte of stack pointer to end of internal RAM
	
	makro 5				; wywolanie makra
	
	ldi  r16, 0x0f
	mov  r1, r16
	push r1
	push r1
	pop  r1
    
	ldi r30, low(bitmask*2)		;pobieranie danych z eeprom
	ldi r31, high(bitmask*2)	;adres musi byc pomnozony 2 razy
	ldi r28, low(tab)			;pobieranie danych z pamieci operacyjnej
	ldi r29, high(tab)			;mnozenie nie jest koneiczne
	ldi r21, 0
	ldi r22, 0x20

; instrukcje kopiowania i transmisji danych
	LDI  r16, 100
	;LDI R1, 100
	MOV  R1, R16
	MOVW R2, R0
	;MOVW R3, R0
	STS  tab, R3
	LDS  R0, tab
	LD   R4, X+			;Podobne adresowanie
	LD   R5, X+			;dla rejestrow Y i Z
	LD   R6, X			;
	LD   R6, -X			;
	CLR  R28
	CLR  R29
	LDD  R6, Y+2		;Podobnie dla Z (brak dla X)
						;Do czego s≥uøy?
    LDI  R16, 0x0f
	CLR  R30
	CLR  R31
	ST	Z+, R16
	ST  Z, R16
	ST -Z, R6
	STD Z+2, R1
	LDI ZL, LOW(qbitmask*2)
	LDI ZH, HIGH(qbitmask*2)
	LPM
	LPM R1, Z+
	LPM R1, Z

	
	PUSH R1
	POP R2
	IN  R2, SPH
; Instrukcje operacji na bitach

	SEC 				; Set Carry C Å© 1 C 1
	CLC 				; Clear Carry C Å© 0 C 1
	SEN 				; Set Negative Flag N Å© 1 N 1
	CLN 				; Clear Negative Flag N Å© 0 N 1
	SEZ 				; Set Zero Flag Z Å© 1 Z 1
	CLZ 				; Clear Zero Flag Z Å© 0 Z 1
	SEI 				; Global Interrupt Enable I Å© 1 I 1
	CLI 				; Global Interrupt Disable I Å© 0 I 1
	SES 				; Set Signed Test Flag S Å© 1 S 1
	CLS 				; Clear Signed Test Flag S Å© 0 S 1
	SEV 				; Set TwoÅfs Complement Overflow V Å© 1 V 1
	CLV 				; Clear TwoÅfs Complement Overflow V Å© 0 V 1
	SET 				; Set T in SREG T Å© 1 T 1
	CLT 				; Clear T in SREG T Å© 0 T 1
	SEH 				; Set Half Carry Flag in SREG H Å© 1 H 1
	CLH 				; Clear Half Carry Flag in SREG H Å© 0 H 1
	BSET SREG_N			; Set flag in SREG
	BCLR SREG_N			; Clear flag in SREG
	LDI  R16, 0xF0
	BST  R16, 4 		; Bit Store from Register to T T Å© Rr(b) T 1
	BLD  R16, 3 		; Bit load from T to Register Rd(b) Å© T None 1
	SWAP R16			; Swap Nibbles

	ROL R16				; Rotate Left Through Carry 
	ROR R16				; Rotate Right Through Carry 
	ASR R16				; Arithmetic Shift Right 
	LSL R16				; Logical Shift Left 
	LSR R16				; Logical Shift Right


LOOP:
	lpm R0, Z+
	writeToEEPROM R21, R22, R0
	; what about macros labels?
	;writeToEEPROM R21, R22, R0
	eor r0,r0
	call loadFromEEPROM	; wywolanie procedury
	lds r0, 2
	ld  r0, Y+ 
	jmp LOOP

;function
loadFromEEPROM:
	sbic EECR, EEWE
	rjmp loadFromEEPROM
	push r12			; save register
	in   r12, SREG		; copy status register
	cli					; disable interrupts
	out  EEARH, r21		; load addres
	out  EEARL, r22
	sbi  EECR, EERE		; load data to write
	in   r0, EEDR
	; what about enabling interrupts?
	out  SREG, r12		; restore status register
	pop  r12			; restore register
	ret

NO_IRQ:
	
	reti

