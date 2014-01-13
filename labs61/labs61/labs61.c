/*
 * labs61.c
 *
 * Created: 2014-01-08 17:15:15
 *  Author: Absherr
 */ 


#include <avr/io.h>

void USART_Init( unsigned int baud )
{
	/* Set baud rate */
	UBRRH = (unsigned char)(baud>>8);
	UBRRL = (unsigned char)baud;
	/* Enable receiver and transmitter */
	UCSRB = (1<<RXEN)|(1<<TXEN);
	/* Set frame format: 8data, 2stop bit */
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
	
}

void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) )
	;
	/* Put data into buffer, sends the data */
	UDR = data;
}

unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSRA & (1<<RXC)) )
	;
	/* Get and return received data from buffer */
	return UDR;
}

unsigned char USART_ReadUCSRC( void )
{
	unsigned char ucsrc;
	/* Read UCSRC */
	ucsrc = UBRRH;
	ucsrc = UCSRC;
	return ucsrc;
}

#include "avr/interrupt.h"
int main(void)
{
	USART_Init(103);
	
	unsigned char ucsrc;
    while(1)
    {
		/* Read UCSRC */
		ucsrc = USART_Receive();
		USART_Transmit(ucsrc);
		//TODO:: Please write your application code 
    }
}