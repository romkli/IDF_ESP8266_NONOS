/*
 * MkBMP180.h
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

#ifndef BMP180_MKBMP180_H_
#define BMP180_MKBMP180_H_

#include "mk_pressure_cfg.h"

#if USE_BMP_085_180 == 1

#define BMP180_ADDR 	0xEE        // Adres I2C


// kdo Ÿród³owy dla AVR do pobrania ze strony:
//		https://sklep.atnel.pl/pl/p/MK-PRESSURE-HUMIDITY-AVR-Biblioteka-C/282










extern uint8_t ICACHE_FLASH_ATTR bmp180_init(void);

extern uint8_t ICACHE_FLASH_ATTR bmp180_read_temp( int8_t * at_int, uint8_t * at_fract );
extern uint8_t ICACHE_FLASH_ATTR bmp180_read_pressure( uint16_t * hp_int, uint8_t * hp_fract );





#endif		// #if USE_BMP_085_180 == 1


#endif /* BMP180_MKBMP180_H_ */
