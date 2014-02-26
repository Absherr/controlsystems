#include <avr/io.h>
#include <avr/interrupt.h>
#include "HD44780.h"


#define TASK_SIZE 10 // wielkosc tablicy zadan
#define NON_PERIODIC 0

// definicja poszczegolnych stanow automatu wczytujacego wyrazenie do obliczenia
#define FIRST_NUM_STATE 0 // stan reprezentujacy wprowadzanie pierwszej liczby
#define OPERATOR_STATE 1 // stan reprezentujacy wprowadzanie dzialania
#define SECOND_NUM_STATE 2 // wprowadzanie drugiej liczby
#define COUNT_STATE 3 // stan obliczajacy wynik
#define RESTART 4 // stan umozliwiajacy wprowadzanie kolejnego wyrazenia bo skonczeniu poprzedniego

#define OP_ADD 0 // operacja + 
#define OP_SUB 1 // - 
#define OP_MUL 2 // *
#define OP_DIV 3 // /

#define FALSE 0
#define TRUE 1

// poszczegolne wartosci w tablicy digits odpowiadają reprezentacji i-tej cyfry na wyswietlaczu siedmiosegmentowym 
unsigned char digits[] = {
	0b11000000, 0b11111001, 0b10100100, 0b10110000, 0b10011001, 0b10010010, 0b10000010, 0b11111000, 0b10000000, 0b10010000
};

int first = 0; // wartosc pierwszej liczby wyrazenia
int second = 0; // druga wartosc
unsigned char operator; // znak dzialania
uint8_t state = FIRST_NUM_STATE; // aktualny stan automatu

int display = 0; // indeks aktualnie wyswietlanej kolumny na siedmiosegmentowym
int result_lcd = 0;
int result_7 = 0;
uint8_t minus = FALSE;
uint8_t overflow = FALSE;
uint8_t error = FALSE;


// ZADANIA
typedef struct
{
        void (*function)(); // funkcja reprezentujaca zadanie
        uint16_t interval; // okresla czy zadanie jest periodyczne czy aperiodyczne
        uint16_t togo; // ile pozostalo do zakonczenia oczekiwania
        uint16_t ready; // czy jest gotowe do wykonania
} TASK;

TASK TASK_TABLE[TASK_SIZE]; // tablica zadan
uint8_t task_count = 0; // aktualna ilosc zadan w tablicy TASK_TABLE
ISR(TIMER0_COMP_vect)
{
        uint8_t i;
        TASK *tsk;
        for(i = 0; i<task_count; ++i)        // w przerwaniu dla kazdego zadania w liscie zadan
        {
                tsk = TASK_TABLE + i;
                if(tsk->interval != NON_PERIODIC)        // jesli zadanie jest periodyczne
                {
                        tsk->togo--;                                // zmniejsz licznik pozostaly do zakonczenia oczekiwania
                        if(tsk->togo == 0)                        // i jesli zakonczono oczekiwanie to ustaw licznik na wartosc poczatkowa i oznacz jako gotowe do wykonania
                        {
                                tsk->togo = tsk->interval;
                                tsk->ready++;
                        }
                }
                else if(tsk->togo > 0)                                // jesli jest aperiodyczne to zmniejsz licznik do konca oczekiwania i ewentualnie ustaw jako gotowe
                {        
                        tsk->togo--;
                        if(tsk->togo == 0)
                        {
                                tsk->ready++;
                        }
                }
        }
		
		display7();
}

void addTask(void (*f)(),uint32_t timeout, uint32_t isPeriodic)        // dodawanie zadan do tablicy zadan 
{
        if(task_count + 1<TASK_SIZE){        // jesli jest jeszcze miejsce w tablicy
                cli();                                        // wylaczamy obsluge przerwan
                TASK_TABLE[task_count].function=f;
                TASK_TABLE[task_count].togo=timeout;
                TASK_TABLE[task_count].ready=0;
                TASK_TABLE[task_count++].interval=(isPeriodic)?timeout:NON_PERIODIC;
                sei();                                        // wlaczamy przerwania
        }
}

void setupTimer()        // inicjalizacja timera
{
        TIMSK=0b00000010;
        TCCR0=0b00000011;
        OCR0=250;
        sei();
}
void init_kernel()        // czyszczenie tablicy TASK_TABLE
{
        uint8_t i;
        task_count = 0;
        for(i=0;i<TASK_SIZE; ++i)
        {
                TASK_TABLE[i].function = 0;
                TASK_TABLE[i].interval = 0;
                TASK_TABLE[i].togo = 0;
                TASK_TABLE[i].ready = 0;
        }
}

void schedule()                // zarzadzanie zadaniami
{        
        uint8_t i;
        while(task_count)        // dopoki sa zadania w TASK_TABLE
        {
                for(i = 0; i < task_count; ++i) {
                        if(TASK_TABLE[i].ready != 0) {                // jesli odliczanie do wykonania zadania sie zakonczylo 
                                TASK_TABLE[i].function();        // to wykonaj zadanie
                                cli();
                                TASK_TABLE[i].ready--;                // flaga ready jest zerowana
                                sei();
                                break;
                        }
                }
        }
}


int is_operator(int a)
{
	if(a==43 || a == 45 || a == 42 || a == 47)
	{
		return TRUE;
	}
	return FALSE;
}

int ascii_to_operator(int a)
{
	if(a==43) return OP_ADD;
	if(a==45) return OP_SUB;
	if(a==42) return OP_MUL;
	if(a==47) return OP_DIV;
}

int calculate(int first, int operator, int second){
	if(operator == OP_ADD) return first+second;
	if(operator == OP_SUB) return first-second;
	if(operator == OP_MUL) return first*second;
	if(operator == OP_DIV) return first/second; // dzielenie przez zero jest zablokowane w innym miejscu
	return 0;
}

int char_to_int(char c)
{
	return (int)(c-48); // 48 to przesuniecie wynikajace z tablicy ASCII
}

void USART_Init( unsigned int baud )
{
	/* Set baud rate */
	UBRRH = (unsigned char)(baud>>8);
	UBRRL = (unsigned char)baud;
	/* Enable receiver and transmitter */
	UCSRB = (1<<RXEN)|(1<<TXEN);
	/* Set frame format: 8data, 2stop bit */
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
	
	UCSRB |= (1 << RXCIE); // Enable the USART Recieve Complete interrupt (USART_RXC)
}

// identyfikuja ktory wiersz i kolumna zostaly wcisniete
int row = -1;
int col = -1;

unsigned char keyboard[] = {
	'7','8','9', '+',
	'4','5','6', '*',
	'1','2','3', 'S',
	'0','=','C', 'R'
	}; 
 
uint8_t button_pressed = FALSE;

uint8_t displayed_amount = 0;
uint8_t was_number = 0;


void serve_keyboard() //realizowane w zadaniu
{
	PORTA = 0x00;
	DDRA = 0b00001111;
	PORTA = 0b11110000;
	for(int i=0; i<16;++i);
	
	//PORTA = PINC;
	if(PINA == 0b11100000)
		row = 3;
	else if(PINA == 0b11010000)
		row = 2;
	else if(PINA == 0b10110000)
		row = 1;
	else if(PINA == 0b01110000)
		row = 0;
	else
		row = -1;
	
	PORTA = 0x00;
	DDRA = 0b11110000;
	PORTA = 0b00001111;
	for(int i=0; i<16;++i);
	
	if(PINA == 0b00001110)
		col = 3;
	else if(PINA == 0b00001101)
		col = 2;
	else if(PINA == 0b00001011)
		col = 1;
	else if(PINA == 0b00000111)
		col = 0;
	else
		col = -1;
	int index_pressed = row*4 + col;
	
	if(col != -1 && row != -1)
	{
		if(button_pressed==FALSE)
		{
			if(state == RESTART)
			{
					LCD_Command(0b00000001);
					first=0;
					second=0;
					operator = -1;
					displayed_amount = 0;
					state = FIRST_NUM_STATE;
					result_lcd = 0;
					result_7 = 0;
					was_number= 0;
					minus = FALSE;
					overflow = FALSE;
					return;
			}
			//result = index_pressed;
			// obsluga
			button_pressed = TRUE;
			//int index_pressed = row*4 + col;	
			int key_pressed = keyboard[index_pressed];
			
			if(state == FIRST_NUM_STATE)
			{
				// jezeli przycisnieta byla cyfra
				if(key_pressed >= 48 && key_pressed <= 57 )
				{
					was_number=1;
					first *= 10;
					first += char_to_int(key_pressed);
					// to ja wyswietlamy
					char d[2] = {(char)key_pressed, 0};
					LCD_Text(d);
					displayed_amount++;
				}
				// jezeli to nie cyfra to zmieniamy stan
				else
				{
					if(was_number == 0)
						return;
					state = OPERATOR_STATE;
					operator = 9;
				}
			}
			if(state==OPERATOR_STATE)
			{
				// jezeli liczba przycisnieta to zmieniany stan
				if(key_pressed>=48 && key_pressed<=57)
				{
					state = SECOND_NUM_STATE;
					was_number = 0;
				}
				else
				{
					if(operator == 9) // jezeli dopiero weszlismy do tego stanu
					{
						operator = ascii_to_operator(key_pressed);
						// to wyswietlamy
						char d[2] = {(char)key_pressed, 0};
						LCD_Text(d);
						displayed_amount++;
					}
					else // jesli byl juz jakis operator
					{
						// to cofamy o jeden
						LCD_Command(0x10);
						// + *
					
						char d[2] = {(char)key_pressed, 0};
						if(key_pressed == '+')
						{
							if(operator == OP_ADD) 
							{
								operator = OP_SUB;
								d[0] = '-';
							}
							else
							{
								operator = OP_ADD;
								d[0] = '+';
							}
						}
						if(key_pressed == '*')
						{
							if(operator == OP_MUL)
							{
								operator = OP_DIV;
								d[0] = '/';
							}
							else
							{
								operator = OP_MUL;
								d[0] = '*';
							}
						}
						// wyswietlamy
						LCD_Text(d);
					}
				}
			}
			if(state==SECOND_NUM_STATE)
			{
				if(key_pressed>=48 && key_pressed<=57)
				{
					was_number = 1;
					second *= 10;
					second += char_to_int(key_pressed);
					
					char d[2] = {(char)key_pressed, 0};
					LCD_Text(d);
					displayed_amount++;
					
				}
				else if(key_pressed == '=')
				{
					if(was_number == 0) return;
					char d[2] = {'=', 0};
					LCD_Text(d);
					displayed_amount++;
					char space[2] = {' ', 0};
					
					for(int i=0;i<40-displayed_amount; ++i)
					{
						LCD_Text(space);	
					}
					
					overflow = FALSE;
					minus = FALSE;
					error = FALSE;
					
					if(operator == OP_DIV && second == 0)
					{
						result_lcd = 0;
						result_7 = 0;
						error = TRUE;
					}
					else
					{
						result_lcd = calculate(first, operator, second);
						result_7 = calculate(first, operator, second);
						int copy = result_lcd;
						char intAsCharArray[16];
						int pointer = 0;
						for(int j=0;j<16;++j) intAsCharArray[j] = ' ';
						if(result_lcd<0)
						{
							copy *= -1;
						}
						while(copy>0)
						{
							intAsCharArray[14 - pointer] = (char)((copy % 10) + 48);
							copy /= 10;
							pointer++;
						}
						if(result_lcd<0)
						{
							intAsCharArray[14 - pointer] = '-';
						}
						intAsCharArray[15] = 0;
						
						LCD_Text(intAsCharArray);
						state = RESTART;
						
						if(result_7<0)
						{
							minus = TRUE;
							result_7 *= -1;
						}
						if(result_7>9999)
						{
							result_7 = 9999;
							overflow = TRUE;
						}
					}
				}
			}
		}
	}
	else
	{
		button_pressed = FALSE;
	}

}

// przetwarzanie danych wyslanych przez USART w przerwaniu
ISR(USART_RXC_vect)
{
	char ReceivedByte;
	ReceivedByte = UDR; // otrzymana wartosc jest zapisywana jako receivedByte
	if(ReceivedByte == 13)
		UDR = '\n'; // odsylamy znak nowej linii
	UDR = ReceivedByte; // odsylanie echa przez USART
	
	// obliczanie wartosci wyrazenia przy uzyciu automatu
	if(state==FIRST_NUM_STATE)
	{
		if(ReceivedByte>=48 && ReceivedByte<=57)
		{
			first *= 10; // liczba jako char[] jest zamiania na tym int
			first += char_to_int(ReceivedByte);
		}
		else
		{
			state = OPERATOR_STATE;
		}
	}
	if(state==OPERATOR_STATE)
	{
		operator = ascii_to_operator(ReceivedByte);
		state = SECOND_NUM_STATE;
	}
	if(state==SECOND_NUM_STATE)
	{
		if(ReceivedByte>=48 && ReceivedByte<=57)
		{
			second *= 10; // zamiana char[] na int
			second += char_to_int(ReceivedByte);
		}
		else if(ReceivedByte == 13)
		{
			overflow = FALSE;
			minus = FALSE;
			error = FALSE;
			
			if(operator == OP_DIV && second == 0)
			{
				result_7 = 0;
				result_lcd = 0;
				error = TRUE;
			}
			else
			{
				result_7 = calculate(first, operator, second);
				result_lcd = calculate(first, operator, second);
				
				if(result_7<0) // jesli mamy wynik ujemny to wyswietlamy dodatni i ustawiamy flage 'minus'
				{
					minus = TRUE;
					result_7 *= -1;
				}
				if(result_7>9999) // przepelnienie ustawia wynik na siedmiosegmentowym na 9999, ustawia flage 'overflow' ale nie zmienia wyniku na LCD
				{
					result_7 = 9999;
					overflow = TRUE;
				}
			}
			// powrot do pierwszego stanu
			state = FIRST_NUM_STATE;
			first=0;
			second=0;
		}
	}
}

// wyswietlanie aktualnego wyniku na wys. siedmiosegmentowym oraz swiecenie diodami (odpowiednie flagi)
void display7() // realizowane w przerwaniu
{
	PORTB &= ~(1<<6); // wykorzystujemy dwa wolne bity na porcie B, na którym aktualnie jest juz obsluga LCD
	PORTB &= ~(1<<7);
	
 	if(minus)
	{
		PORTB |= (1<<6);
	}
	if(overflow)
	{
		PORTB |= (1<<7);
	}
	if(error)
	{
	    // brakuje juz portu do obslugi kolejnej diody
	}
	
	// czyscimy
	PORTD = 0x00; // numer kolumny z wys. siedmiosegmentowego do wyswietlania
	PORTC = 0x00; // cyrfa wyswietlana na siedmiosegmentowym

	PORTD = PORTD | (1 << 5) | (1<<4) | (1<<3) | (1<<2);
	
	if(display == 0)
	{
		PORTD &= ~(1<<5);
		PORTC = digits[result_7/1000];
	}
	else if(display == 1)
	{
		PORTD &= ~(1<<4);
		PORTC = digits[(result_7-(result_7/1000)*1000)/100];
	}
	else if(display == 2)
	{
		PORTD &= ~(1<<3);
		PORTC = digits[(result_7-(result_7/100)*100)/10];
	}
	else if(display == 3)
	{
		PORTD &= ~(1<<2);
		PORTC = digits[result_7%10];
	}
	
	display++; 
	if (display%4 == 0)
	{
		display = 0;
	}
}


int main(void)
{
	DDRB = 0xFF;
	
	LCD_Initalize();
	LCD_Command(0b00000001); // wlaczamy wyswietlacz
	
	USART_Init(103);
	sei();
    init_kernel();		
	DDRC = 0xFF;
	DDRD = 0xFF;
	
	addTask(*serve_keyboard, 50, 1);

    setupTimer();
    schedule();
    return 0;
}