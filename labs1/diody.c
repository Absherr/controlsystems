/*
 * Created: 2013-10-23 15:53:22
 *  Author: Absherr
 */ 

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	DDRA = 0xFF;
	//unsigned char i = 0;
    while(1)
    {
		for(int i=0;i<100;++i)
		{
			PORTA = 0xFF;
			_delay_ms(i);
			PORTA = 0x00;
			_delay_ms(100-i);
		}
		
		PORTA = 0xFF;
		_delay_ms(100);
		for(int i=0;i<100;++i)
		{
			PORTA = 0x00;
			_delay_ms(i);
			PORTA = 0xFF;
			_delay_ms(100-i);
		}
		
		PORTA = 0x00;
		_delay_ms(100);
	}
}
