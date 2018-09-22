/*
* 2_2.c
*
* Created: 2018/9/7 15:18:15
* Author : cjiawei
* Pin Map:	PB0(8) - Echo
*			PB1(9) - Trigger
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"

volatile uint16_t t;		// rising edge of the result pulse
volatile uint32_t span; // difference between falling and rising edge

volatile uint8_t state; // record measuring state
#define STATE_PRINT 0	// used to decide if the result has been printed

#define MEASURE_GAP 3
volatile uint8_t timeout = MEASURE_GAP; // minimum overflows between measurements
volatile uint8_t cross = 0;			// count the overflows between rising edge and falling edge

inline void measure_start(void)
{
	TCCR1B |= 1 << ICES1;		// set input capture to be rising edge
	if(TIFR1 & 1 << OCF1A)
		TIFR1 |= 1 << OCF1A;	// clear unwanted compare match
	TCCR1A |= 1 << COM1A0;
	TCCR1C |= 1 << FOC1A;		// manually set trigger pin high
	TIMSK1 |= 1 << OCIE1A;		// turn on comparator interrupt
}

int main(void)
{
	uart_init(); // initialize UART for stdio functions

	DDRB |= 1 << DDB1;		// set trigger pin as output
	
	OCR1A = 16*50;						// set length of trigger pulse
	measure_start();
	TCCR1B |= 1 << CS10;				// start clock, no prescaling
	TIMSK1 |= 1 << ICIE1 | 1 << TOIE1;	// enable input capture, overflow interrupt
	sei();								// enable interrupts

	while (1)
	{
		while (!(state & 1 << STATE_PRINT));	// loop until get data to print
		printf("Time:%.1fus, Distance:%.3fm\n", (float)span/16, 0.00034f*span/16/2);
		state &= ~(1 << STATE_PRINT);		// clear the state bit for data waiting to print
	}
}

ISR(TIMER1_COMPA_vect)	// timer1 comparator A interrupt
{
	TCCR1A &= ~(1 << COM1A0);	// turn off trigger pin automatic toggling
	TIMSK1 &= ~(1 << OCIE1A);	// turn off comparator interrupt
	timeout = MEASURE_GAP;			// reset measurement timeout counter
}

ISR(TIMER1_CAPT_vect)	// timer1 input capture interrupt
{
	if (TCCR1B & 1 << ICES1)	// if is rising edge
	{
		t = ICR1;	// save the beginning of the pulse
		cross = 0;	// reset the measurement crossing counter
	}
	else						// if is falling edge
	{
		span = (uint32_t)cross * 0xFFFF + ICR1 - t; // calculate the total result timeout width
		state |= 1 << STATE_PRINT;// set flags for new data
		timeout = 1;	// prepare for next measurement
	}
	TCCR1B ^= 1 << ICES1;	// toggle capture edge
}

ISR(TIMER1_OVF_vect)	// timer1 overflow interrupt
{
	cross++;	// increase crossing counter
	if (!(PINB & 1 << PINB0) && !timeout--) // decrease timeout counter until ready for next measurement
		measure_start();
}