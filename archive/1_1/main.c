/*
 * 1_1.c
 *
 * Created: 2018/9/7 8:35:52
 * Author : cjiawei
 * Pin Map: PB5(13) - LED
 *			PB0(8) - button
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000L
#include <util/delay.h>

int main(void)
{
	CLKPR = 1<<CLKPCE; // enable system clock pre-scaler
	CLKPR = 1<<CLKPS1; // 0100 - clock divided by 16 (for better noise canceling)
	 
	DDRB |= 1<<DDB5; // set PB5(13) as output
	PORTB |= 1<<PORTB0; // enable PB0 internal pull up
	
	_delay_us(5); // wait for io ports to settle down
	
	if(!(PINB & 1<<PINB0)) // if button pressed - low
	{
		PORTB |= 1<<PORTB5; // turn on the LED
		TCCR1B |= 1<<ICES1; // set input capture edge to be rising
	}
	
	TCCR1B |= 1<<ICNC1; // enable input capture noise canceling
	TIMSK1 |= 1<<ICIE1; // enable input capture interrupt	sei(); // global enable interrupts 
    while (1); // do nothing here and let the interrupt to handle the rest
}

ISR(TIMER1_CAPT_vect)
{
	PORTB ^= 1<<PORTB5; // toggle LED on/off
	TCCR1B ^= 1<<ICES1; // toggle capture edge falling/rising
}
