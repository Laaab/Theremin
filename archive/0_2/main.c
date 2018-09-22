/*
 * GccApplication1.c
 *
 * Created: 2018/9/6 17:07:35
 * Author : 123
 * Pin Map: PB0(8) - button input
 *			PB5(13) - LED
 */ 

#include <avr/io.h>


int main(void)
{
	DDRB &= ~(1<<DDB0);	//set PB0 as input
	PORTB |= 1<<PORTB0;	//enable PB0 pull up
	DDRB |= 1<<DDB5;	//set PB5 as output
	
    while (1) 
    {
		if (PINB & 1<<PINB0)		//if PB0 read high
			PORTB &= ~(1<<PORTB5);	//write PB5 low
		else						//if PB0 read low
			PORTB|= 1<<PORTB5;		//write PB5 high
    }
}

