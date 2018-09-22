/*
 * 2_1.c
 *
 * Created: 2018/9/7 15:17:46
 * Author : cjiawei
 * Pin Map: PD6(6)
 */ 

#include <avr/io.h>


int main(void)
{
    DDRD |= 1<<DDD6; // set PD6 as output
    OCR0A = 70; // compare output threshold, 71-1 since counting from 0
    TCCR0A |= 1<<COM0A0 | 1<<WGM01; // 01 - toggle OC0A on compare match, 010 - CTC mode
    TCCR0B |= 1<<CS02; // 100 - counter clock divided by 256
    
    while (1);
}

