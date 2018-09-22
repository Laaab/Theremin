/*
 * 2_3-4.c
 *
 * Created: 2018/9/9 22:28:42
 * Author : cjiawei
 * PinMap:	PB0(8) - Echo
 *			PB1(9) - Trigger
 *			PD6(6) - PWM output
 *			PD7(7) - switch for continuous and discrete
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define TOP 20000
#define BOTTOM 5000

#define MAP(x, top, bottom, catagory) ((x)-bottom)*catagory/(top-bottom)

#define TRIM(x, top, bottom) (x)>=top?top-1:((x)<bottom?bottom:(x))

const uint8_t freq_map[]={59, 62, 70, 79, 88, 94, 105, 118}; // notes to comparator map table

volatile uint16_t t;	// rising edge of the result pulse
volatile uint32_t span; // difference between falling and rising edge

#define MEASURE_GAP 2
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
	DDRB |= 1 << DDB1;		// set trigger pin as output
	DDRD |= 1 << DDD6;		// set PD6 as output
	PORTD |= 1 << PORTD7;	// enable PD7 internal pull up
	
	OCR1A = 16*50;			// set length of trigger pulse
	TCCR0A |= 1 << COM0A0;	// 01 - toggle OC0A on compare match
	TCCR0A |= 1 << WGM01;	// 010 - CTC mode
	measure_start();
	TCCR1B |= 1 << CS10;				// start clock, no prescaling
	TCCR0B |= 1 << CS01 | 1 << CS00;	// 010 - counter clock divided by 64
	TIMSK1 |= 1 << ICIE1 | 1 << TOIE1;	// enable input capture, overflow interrupt
	sei();								// enable interrupts

	while (1)
	{
		// update frequency
		uint32_t data_freq= TRIM(span, TOP, BOTTOM);
		if(PIND & 1 << PIND7)	// if continuous
			OCR0A = MAP(data_freq, TOP, BOTTOM, 61) + 58; // map ADC result into comparator threshold (continuous)
		else
			OCR0A = freq_map[MAP(data_freq, TOP, BOTTOM, 8)]; // map ADC result into comparator threshold (discrete)
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
