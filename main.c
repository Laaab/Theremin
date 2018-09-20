/*
 * 3_3.c
 *
 * Created: 2018/9/11 15:52:38
 * Author : cjiawei
 * Pin Map:	PD3(3) - timer 2 PWM out (continuous amplitude)
 *			PD6(6) - timer 0 PWM out (pitch frequency)
 *			PC0(A0) - analog input (photo-resistor measurement)
 *			PB0(8) - Echo for ultrasonic rangefinder
 *			PB1(9) - Trigger for ultrasonic rangefinder
 *			PB2-4(10-12) - DAC output (discrete amplitude)
 *			PD7(7) - switch	(switch between continuous and discrete pitch)
 */ 

#include "theremin.h"

volatile uint16_t t;	// rising edge of the result pulse
volatile uint32_t span; // difference between falling and rising edge (distance result)
volatile uint16_t bright;	// voltage from photo-resistor

volatile uint8_t timeout = MEASURE_GAP; // minimum overflows between measurements
volatile uint8_t cross = 0;			// count the overflows between rising edge and falling edge

const uint8_t freq_map[]={58, 62, 70, 79, 88, 94, 105, 118}; // notes to comparator value mapping table

inline void measure_start(void)
{
	TCCR1B |= 1 << ICES1;		// set input capture to be rising edge
	if(TIFR1 & 1 << OCF1A)
	TIFR1 |= 1 << OCF1A;	// clear unwanted compare match
	TCCR1A |= 1 << COM1A0;		// turn on comparator auto toggle for timer1 A
	TCCR1C |= 1 << FOC1A;		// manually set trigger pin high for timer1 A
	TIMSK1 |= 1 << OCIE1A;		// turn on comparator interrupt
}

void amp_init(void)
{
	DDRB |= 0b1111 << DDB1;		// set trigger and DAC pins as output
	DDRD |= 1 << DDD3;		// set PWM pin as output
	
	ADMUX |= 1 << REFS0;	// use AVcc as ADC reference
	ADCSRA |= 1 << ADEN | 1 << ADSC | 1 << ADATE | 1 << ADIE	// enable ADC, begin conversion, enable auto trigger
		| 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0;	// set ADC prescaler as 128 to get better result

	OCR2B = 0xff;			// set duty to 0
	TCCR2A |= 1 << COM2B1 | 1 << WGM21 | 1 << WGM20;	// timer2: 11 - OC2B non-inverting output, 011 - fast PWM mode
	TCCR2B |= 1<<CS20;		// enable timer2, no prescaler
}

void freq_init(void)
{
	DDRD |= 1 << DDD6;		// set PD6 as output
	PORTD |= 1 << PORTD7;	// enable PD7 internal pull up

	TCCR0A |= 1 << COM0A0;	// timer0: 01 - toggle OC0A on compare match
	TCCR0A |= 1 << WGM01;	// timer0: 010 - CTC mode
	
	OCR1A = 16*50;						// set length of trigger pulse
	measure_start();
	
	TIMSK1 = 1 << ICIE1 | 1 << TOIE1;	// timer1: enable input capture, overflow interrupt
	
	TCCR1B |= 1 << CS10;				// timer1: start clock, no prescaling
	TCCR0B |= 1 << CS01 | 1 << CS00; // timer0: 010 - counter clock divided by 64
}

int main(void)
{
	uint16_t prev_data_amp = 0;
	uint32_t prev_data_freq = 0;
	
	freq_init();
	amp_init();
	sei();
	
    while (1) 
    {
		// update amplitude
		uint16_t data_amp = TRIM(bright, AMP_TOP, AMP_BOTTOM);
		if(data_amp != prev_data_amp)	// only update when necessary
		{
			prev_data_amp = data_amp;
			OCR2B = MAP(data_amp, AMP_TOP, AMP_BOTTOM, 256);	// set PWM duty for continuous DAC (inverted)
			PORTB = MAP(data_amp, AMP_TOP, AMP_BOTTOM, 8) << PORTB2; // output the discrete DAC value
		}
		
		// update frequency
		uint32_t data_freq= TRIM(span, FREQ_TOP, FREQ_BOTTOM);
		if(data_freq != prev_data_freq)	// only update when necessary
		{
			prev_data_freq = data_freq;
			if(PIND & 1 << PIND7)	// if continuous
				OCR0A = MAP(data_freq, FREQ_TOP, FREQ_BOTTOM, 61) + 58; // map distance into comparator threshold (continuous)
			else
				OCR0A = freq_map[MAP(data_freq, FREQ_TOP, FREQ_BOTTOM, 8)]; // map distance into comparator threshold (discrete)
		}
    }
}

ISR(TIMER1_COMPA_vect)	// timer1 comparator A interrupt (send trigger)
{
	TCCR1A &= ~(1 << COM1A0);	// turn off trigger pin automatic toggling
	TIMSK1 &= ~(1 << OCIE1A);	// turn off comparator interrupt
	timeout = MEASURE_GAP;		// reset measurement timeout counter
}

ISR(TIMER1_CAPT_vect)	// timer1 input capture interrupt (measure distance)
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

ISR(TIMER1_OVF_vect)	// timer1 overflow interrupt (watchdog)
{
	cross++;	// increase crossing counter
	if (!(PINB & 1 << PINB0) && !timeout--) // decrease timeout counter until ready for next measurement (when echo low)
		measure_start();
}

ISR(ADC_vect)
{
	bright = ADC;
}