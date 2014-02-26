#include "HD44780.h"
#include <util/delay.h>

#define D7 3
#define D6 2
#define D5 1
#define D4 0
#define E 4
#define RS 5

void WriteNibble(unsigned char nibbleToWrite)
{
	//zapis pojedynczej mlodszej czworki z parametru
	PORTB |= (1<<E);
	PORTB = (PORTB & 0xF0) | (nibbleToWrite & 0x0F);
	PORTB &= ~(1<<E);
}


void WriteByte(unsigned char dataToWrite)
{
	WriteNibble(dataToWrite >> 4);
	WriteNibble(dataToWrite);
	_delay_us(50);
}


void LCD_Command(unsigned char cmd)
{
	PORTB &= ~(1<<RS);
	WriteByte(cmd);
};

void LCD_Text(char* text)
{
	PORTB |= (1<<RS);
	
	while(*text != 0)
	{
		WriteByte(*text);
		text++;
	}
	
};

void LCD_Initalize(void)
{
	DDRB = 0xFF;
	PORTB = 0x00;
	_delay_ms(15);
	for(int i=0;i<3;++i)
	{
		WriteNibble(0b00000011);
		_delay_ms(5);
	}

	WriteNibble(0b00000010);

	_delay_ms(1);
	
	LCD_Command(0b00101000);
	_delay_ms(2);
	LCD_Command(0b00001000);
	_delay_ms(2);
	LCD_Command(0b00000001);
	_delay_ms(2);
	LCD_Command(0b00000110);
	_delay_ms(2);
	
	LCD_Command(0b00001111);
	_delay_ms(2);
	
	LCD_Command(0b10000001);
	
	_delay_ms(50);
}