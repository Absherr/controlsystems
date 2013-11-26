/*
 * labs4.c
 *
 * Created: 2013-11-13 16:03:54
 *  Author: Absherr
 */ 

# define F_CPU 1600000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
// przerwanie co 1ms
void setupTimer()
{
		TIMSK=0b00000010;
		OCR0=250;
		sei();
}

void setupPWM()
{
	TCCR0=0b00000011;
}

void setPWM(unsigned char power)
{
	OCR0 = power;
}

#define MAX_VALUE 250
#define MIN_VALUE 0

#define STRONGER 0
#define WEAKER 1

// okresla wartosc OCR0 (moze sie zmniejszac lub zwiekszac)
unsigned char counter = MAX_VALUE;
// kierunek okresla czy zmiane mocy �wiecenia diody
// direction = 0 => �weci coraz mocniej
// direction = 1 => �wieci coraz s�abiej
unsigned char direction = STRONGER;

// idea:
// ustawiamy timer na przerwanie co 1ms
// i teraz co ka�de przerwanie zmniejszamy OCR0 o jeden
// czyli przerwanie nast�puje coraz szybciej
// a w przerwaniu mrygramy diodom

ISR(TIMER0_COMP_vect)
{
	// wy��czamy przerwania
	cli();
	// zaswiecamy diod�
	PORTB = 0xFF;
	// chcemy zeby swiecila mocniej => musi �wieci� cz�ciej => coraz mniejsze OCR0
	if(direction == STRONGER)
	{
		counter--;
		if(counter == MIN_VALUE)
		{
			direction = WEAKER;
		} 
	}
	// chcemy zeby swiecila s�abiej => musi �wieci� rzadziej => coraz wieksze OCR0
	else if(direction == WEAKER)
	{
		counter++;
		if(counter == MAX_VALUE)
		{
			direction = STRONGER;
		}
	}
	// ustawiamy now� warto�� w OCR0
	OCR0 = counter;
	// gasimy diod�
	PORTB = 0x00;
	// w��czamy przerwania
	sei();
}

int main(void)
{
	setupTimer();
	setupPWM();
	DDRB = 0xFF;
    while(1)
    {}
}