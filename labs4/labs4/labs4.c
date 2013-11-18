/*
 * labs4.c
 *
 * Created: 2013-11-13 16:03:54
 *  Author: Absherr
 */ 

// zad1
// proc1: setup_TIMEOUT inicjalizacja timera z przerwaniem co okreslonoa ilosc mikrosekund
// i zlicza ilosc mikrosekund
// proc2: setup_PWM licznik w trybie PWM
// proc3: set_PWM(0 ... 255) - okresla jasnosc
// port PB3 to OC0 -> do diody

// avr/interrupt tam sa identyfikatory przerwan
// timer_ovf_vect
// ISR(timer_ovf_vect){ // code }
// TIMSK - ktore przerwania
// TCCR0 - prescaler i tryby pracy
// OCR0 - konkretna wartosc w OCR0
// fast PWM wystarczy do tego zadania

// 
#include <avr/io.h>

void setupTimer(unsigned char amount)
{
	//TCCR0 |= (1<<CS00);
	//OCR0 = 0;
	
	TIMSK |= (1<<OCIE0);
	TCCR0 |= (1<<CS01) | (1<<WGM01);
	
	OCR0 = amount << 1;
	sei();
}

void setupPWM()
{
	TCCR0 |=  (1<<WGM00) | (1<<WGM01) | (1<<CS00) | (1<<COM01);
}

void setPWM(int power)
{
	OCR0 = power;
}

#include <util/delay.h>
#include <avr/interrupt.h>

int main(void)
{
	setupTimer(100);
	setupPWM();
	//sei();	
	DDRB = 0xFF;
	unsigned char counter = 0;
    while(1)
    {
		setPWM(counter);
		_delay_ms(100);
		counter++;
		
        //TODO:: Please write your application code 
    }
}