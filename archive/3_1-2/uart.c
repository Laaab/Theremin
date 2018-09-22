/*
 * uart.c
 *
 * Created: 2018/9/8 10:06:58
 *  Author: cjiawei
 */ 

#define F_CPU 16000000L
#define BAUD 9600
#define UBR (F_CPU/16/BAUD-1)

#include <avr/io.h>
#include "uart.h"

int uart_tx(char c, FILE *stream)
{
	while(!(UCSR0A & 1<<UDRE0));	// loop until buffer empty
	UDR0 = c;						// push data into buffer
	return 0;
}

int uart_rx(FILE *stream)
{
	while(!(UCSR0A & 1<<RXC0));	// loop until data available
	return UDR0;				// retrieve data from buffer
}

static FILE uart_file = FDEV_SETUP_STREAM(uart_tx, uart_rx, _FDEV_SETUP_RW); // create UART stream

void uart_init(void)
{
	UBRR0H = UBR>>8;	//set baud rate
	UBRR0L = UBR;

	UCSR0C = 1<<UCSZ01 | 1<<UCSZ00; // set UART to be 8 bits
	UCSR0B = 1<<RXEN0 | 1<<TXEN0;   // enable TX and RX
	
	stdout = &uart_file;	// redirect stdout to UART
	stdin = &uart_file;	// redirect stdin to UART
	stderr = &uart_file;	// redirect stderr to UART
}
