/*
 * Scheduler.c
 *
 * Created: 2013-11-20 15:31:54
 *  Author: Absherr
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>


#define TASK_SIZE 10
#define NON_PERIODIC 0

typedef struct
{
	void (*function)();
	uint16_t interval;
	uint16_t togo;
	uint16_t ready;
} TASK;

TASK TASK_TABLE[TASK_SIZE];
uint8_t task_count = 0;
ISR(TIMER0_COMP_vect)
{
	uint8_t i;
	TASK *tsk;
	for(i = 0; i<task_count; ++i)
	{
		tsk = TASK_TABLE + i;
		if(tsk->interval != NON_PERIODIC)
		{
			tsk->togo--;
			if(tsk->togo == 0)
			{
				tsk->togo = tsk->interval;
				tsk->ready++;
			}
		}
		else if(tsk->togo > 0)
		{	
			tsk->togo--;
			if(tsk->togo == 0)
			{
				tsk->ready++;
			}
		}
	}
}

void addTask(void (*f)(),uint32_t timeout, uint32_t isPeriodic)
{
	if(task_count + 1<TASK_SIZE){
		cli();
		TASK_TABLE[task_count].function=f;
		TASK_TABLE[task_count].togo=timeout;
		TASK_TABLE[task_count].ready=0;
		TASK_TABLE[task_count++].interval=(isPeriodic)?timeout:NON_PERIODIC;
		sei();
	}
}

void setupTimer()
{
	TIMSK=0b00000010;.
	TCCR0=0b00000011;
	OCR0=250;
	sei();
}
void init_kernel()
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

void schedule()
{	
	uint8_t i;
	while(task_count)
	{
		for(i = 0; i < task_count; ++i) {
			if(TASK_TABLE[i].ready != 0) {
				TASK_TABLE[i].function();
				cli();
				TASK_TABLE[i].ready--;
				sei();
				break;
			}
		}
	}
}

void pierwsza()
{
	PORTA ^= (1<<PINA0);
}
void druga()
{
	PORTA ^= (1<<PINA1);
}
void trzecia()
{
	PORTA ^= (1<<PINA2);
}
void czwarta()
{
	PORTA ^= (1<<PINA3);
}



int main(void)
{
	DDRA = 0xFF;
	
	init_kernel();
	addTask(*pierwsza, 2000, 1);
	addTask(*druga, 1000, 1);
	addTask(*trzecia, 500, 1);
	addTask(*czwarta, 250, 1);
	
	setupTimer();
	
	schedule();
	return 0;
}