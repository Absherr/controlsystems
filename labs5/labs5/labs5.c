#include <avr/io.h>
#include <avr/interrupt.h>

unsigned char digits[] = {
	0b11000000, 0b11111001, 0b10100100, 0b10110000, 0b10011001, 0b10010010, 0b10000010, 0b10000010, 0b11111000, 0b10000000, 0b10010000
};


#define TASK_SIZE 10
#define NON_PERIODIC 0

typedef struct
{
	void (*function)(); // funkcja reprezentujaca zadanie
	uint16_t interval;        // okresla czy zadanie jest periodyczne czy aperiodyczne
	uint16_t togo;                // ile pozostalo do zakonczenia oczekiwania
	uint16_t ready;                // czy jest gotowe do wykonania
} TASK;

TASK TASK_TABLE[TASK_SIZE];        // tablica zadan
uint8_t task_count = 0;                // aktualna ilosc zadan w tablicy TASK_TABLE
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
void init_kernel()        // zapelnianie tablicy TASK_TABLE
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

unsigned char display = 0;

void change()
{
	// czyscimy 
	PORTB = 0x00;
	PORTA = 0x00;
	if(display == 0)
	{
		PORTB = 0b11111110;
		PORTA = digits[1];
	
	}
	else if(display == 1)
	{
		PORTB = 0b11111101;
		PORTA = digits[2];
	}
	else if(display == 2)
	{
		PORTB = 0b11111011;
		PORTA = digits[3];
	}
	else if(display == 3)
	{
		PORTB = 0b11110111;
		PORTA = digits[4];
	}
	
	display ++;
	if(display % 4 == 0)
		display = 0;
}


int main(void)
{
	DDRA = 0xFF;
	DDRB = 0xFF;
	
	init_kernel();
	addTask(*change, 4, 1);
	
	setupTimer();
	
	schedule();
	return 0;
}