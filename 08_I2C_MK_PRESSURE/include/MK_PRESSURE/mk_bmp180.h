/*
 * MkBMP180.h
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

#ifndef BMP180_MKBMP180_H_
#define BMP180_MKBMP180_H_

#include "mk_pressure_cfg.h"

#if USE_BMP_085_180 == 1

#define BMP180_ADDR 	0xEE        // Adres I2C


// kdo �r�d�owy dla AVR do pobrania ze strony:
//		https://sklep.atnel.pl/pl/p/MK-PRESSURE-HUMIDITY-AVR-Biblioteka-C/282










extern uint8_t ICACHE_FLASH_ATTR bmp180_init(void);

extern uint8_t ICACHE_FLASH_ATTR bmp180_read_temp( int8_t * at_int, uint8_t * at_fract );
extern uint8_t ICACHE_FLASH_ATTR bmp180_read_pressure( uint16_t * hp_int, uint8_t * hp_fract );





#endif		// #if USE_BMP_085_180 == 1


#endif /* BMP180_MKBMP180_H_ */
