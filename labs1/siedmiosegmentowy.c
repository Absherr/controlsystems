/*
 * Created: 2013-10-23 15:53:22
 *  Author: Absherr
 */ 

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	unsigned char digits[] = {
		0b11000000, 0b11111001, 0b10100100, 0b10110000, 0b10011001, 0b10010010, 0b10000010, 0b10000010, 0b11111000, 0b10000000, 0b10010000
		};
	
	DDRA = 0xFF;
    unsigned int counter = 0;
	while(1)
    {
		PORTA = digits[counter];
		counter = counter + 1 % 10;		
		_delay_ms(1000);
	}
}