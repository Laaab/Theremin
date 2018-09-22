Source code for Lab2 Theremin
======
The code is for 3.3&3.4, other code is in 'archive' folder

Function description:
------
1. Generate PWM signal for buzzer
2. Frequency depends on distance measured by ultrasound rangefinder
   * discrete when switch is on and continuous when switch is off
3. Amplitude depends on voltage from photo-resistor voltage divider
   * discrete amplitude control from PB2-4 to resistor network DAC
   * continuous amplitude control from PD3 to RC filter

PinMap:
------
* PD3(3) - timer2 PWM out (continuous amplitude)
* PD6(6) - timer0 PWM out (pitch frequency)
* PC0(A0) - analog input (photo-resistor measurement)
* PB0(8) - Echo for ultrasonic rangefinder
* PB1(9) - Trigger for ultrasonic rangefinder
* PB2-4(10-12) - DAC output (discrete amplitude)
* PD7(7) - switch	(switch between continuous and discrete pitch)

Module functionality:
------
1. Timer0(CTC mode): for PWM pitch control 
2. Timer1(normal mode): for ultrasonic rangefinder distance measurement
   * use output compare unit A for trigger signal, force set when overflow and clear when compare match.
   * use input capture unit for reflect pulse measurement.
   * use overflow interrupt for pulse measurement crossing count and watchdog.
3. Timer2(fast PWM mode): (compare unit B) for continuous amplitude control
4. ADC: for photo-resistor voltage divider measurement
