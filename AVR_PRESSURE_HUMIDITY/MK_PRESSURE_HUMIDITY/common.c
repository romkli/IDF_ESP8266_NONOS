/*
 * common.c
 *
 * 	sample code for CPU's: m16/32 and m644(P)/m1284(P)
 * 	F_CPU: 1 - 20 MHz
 *
 *  Created on: 22 kwiecie� 2021
 *      Author: Miros�aw Karda�
 *
 *		www.atnel.pl           www.sklep.atnel.pl
 *      Pressure, Temperature & humidity LIB
 *      ver: 1.0
 *
 *      for:
 *      BMP085/BMP180	- ci�nienie, temperatura,     -
 *      BMP280			- ci�nienie, temperatura,     -
 *      BME280			- ci�nienie, temperatura, wilgotno��
 *      HIH6131 		-     -      temperatura, wilgotno�� 				(najlepszy czujnik wilgotno�ci !)
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

#include "common.h"

#include "MK_LCD/mk_lcd44780.h"




volatile uint16_t Timer1, Timer2, Timer3;


void myDelay( uint16_t ms ) {
	while( ms-- ) _delay_ms(1);
}














#ifdef TIMER0_COMPA_vect
	ISR( TIMER0_COMPA_vect ) {
#else
	ISR( TIMER0_COMP_vect ) {
#endif

	uint16_t n;

	n = Timer1;
	if( n ) Timer1 = --n;

	n = Timer2;
	if( n ) Timer2 = --n;

	n = Timer3;
	if( n ) Timer3 = --n;
}
