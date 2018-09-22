/*
 * 3_1.c
 *
 * Created: 2018/9/9 23:35:18
 * Author : cjiawei
 * PinMap:	PC0(A0) - analog input
 *			PB2-4(10-12) - DAC output
 */ 

#include <avr/io.h>
#include "uart.h"

#define TOP 700
#define BOTTOM 400

#define MAP(x, top, bottom, catagory) ((x)-bottom)*catagory/(top-bottom)
#define TRIM(x, top, bottom) (x)>=top?top-1:((x)<bottom?bottom:(x))

volatile uint16_t bright=0;
volatile uint8_t c=0;
int main(void)
{
	DDRB |= 0b111 << DDB2;	// set DAC pins as output
	uart_init();
	
	ADMUX |= 1 << REFS0;				// use AVcc as ADC reference
	ADCSRA |= 1 << ADEN | 1 << ADSC | 1 << ADATE	// enable ADC, begin conversion, enable auto trigger
		| 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0;	// set ADC prescaler as 128 to get better result
	
	while (1)
	{
		printf("%hu\n", ADC);
		uint16_t data_amp = TRIM(ADC, TOP, BOTTOM);
		PORTB = MAP(data_amp, TOP, BOTTOM, 8) << PORTB2;
	}
}
