/*
 * common.h
 *
 * 	sample code for CPU's: m16/32 and m644(P)/m1284(P)
 * 	F_CPU: 1 - 20 MHz
 *
 *  Created on: 22 kwiecieñ 2021
 *      Author: Miros³aw Kardaœ
 *
 *		www.atnel.pl           www.sklep.atnel.pl
 *      Pressure, Temperature & humidity LIB
 *      ver: 1.0
 *
 *      for:
 *      BMP085/BMP180	- ciœnienie, temperatura,     -
 *      BMP280			- ciœnienie, temperatura,     -
 *      BME280			- ciœnienie, temperatura, wilgotnoœæ
 *      HIH6131 		-     -      temperatura, wilgotnoœæ 				(najlepszy czujnik wilgotnoœci !)
 */


#define LED				(1<<PC6)
#define LED_PORT		PORTC
#define LED_DIR			DDRC

#define LED_OFF			LED_PORT |= LED
#define LED_ON			LED_PORT &= ~LED
#define LED_TOG			LED_PORT ^= LED




extern volatile uint16_t Timer1, Timer2, Timer3;







extern void myDelay( uint16_t ms );


