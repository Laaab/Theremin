/*
 * GccApplication2.c
 *
 * Created: 2018/9/6 19:13:45
 * Author : 123
 * Pin Map: PD6(6) - output
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>


int main(void)
{
	DDRD |= 1<<DDD6;	//set PD6 as output
	TIMSK0 |= (1<<TOIE0);//enable timer0 overflow interrupt
	sei();				//enable interrupts
	
	TCCR0B |= (1<<CS01) | (1<<CS00); // timer clock get divided by 64
    while (1);
}

ISR(TIMER0_OVF_vect)	// timer0 overflow interrupt
{
	PORTD ^=(1<<PORTD6);	// toggle PD6
}
