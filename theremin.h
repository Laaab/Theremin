/*
 * theremin.h
 *
 * Created: 2018/9/11 17:13:30
 *  Author: cjiawei
 */ 


#ifndef THEREMIN_H_
#define THEREMIN_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#define FREQ_TOP 20000
#define FREQ_BOTTOM 5000
#define AMP_TOP 800
#define AMP_BOTTOM 400

#define MEASURE_GAP 2

//#define FREQ_CONTINUOUS

#define MAP(x, top, bottom, catagory) ((x)-bottom)*catagory/(top-bottom)

#define TRIM(x, top, bottom) (x)>=top?top-1:((x)<bottom?bottom:(x))

#endif /* THEREMIN_H_ */