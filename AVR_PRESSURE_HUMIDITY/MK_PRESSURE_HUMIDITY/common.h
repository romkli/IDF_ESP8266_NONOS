/*
 * common.h
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


#define LED				(1<<PC6)
#define LED_PORT		PORTC
#define LED_DIR			DDRC

#define LED_OFF			LED_PORT |= LED
#define LED_ON			LED_PORT &= ~LED
#define LED_TOG			LED_PORT ^= LED




extern volatile uint16_t Timer1, Timer2, Timer3;







extern void myDelay( uint16_t ms );


