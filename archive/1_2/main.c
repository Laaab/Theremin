/*
 * Lab2_1_2.c
 *
 * Created: 2018/9/7 10:15:53
 * Author : 123
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>


int main(void)
{
    DDRD |= 1<<DDD6;                // set PD6 as output
	OCR0A = 70;                     // total timer ticks
	TIMSK0 = 1<<OCIE0A;				// enable timer0 comparator A interrupt
	sei(); 
	
	TCCR0B = 1<<CS02;               // clk/256
    while (1);
}

ISR(TIMER0_COMPA_vect)
{
	PORTD ^=(1<<PORTD6);
	OCR0A += 71;
}
