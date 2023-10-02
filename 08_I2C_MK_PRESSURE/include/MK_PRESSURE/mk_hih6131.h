/*
 * mk_sht3x.h
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

#ifndef MK_PRESSURE_LIB_MK_HIH6131_H_
#define MK_PRESSURE_LIB_MK_HIH6131_H_


#include "mk_pressure_cfg.h"

#if USE_HIH6131 == 1

#define HIH6131_ADDR 	0x4E        // Adres I2C




// kdo �r�d�owy dla AVR do pobrania ze strony:
//		https://sklep.atnel.pl/pl/p/MK-PRESSURE-HUMIDITY-AVR-Biblioteka-C/282





extern uint8_t ICACHE_FLASH_ATTR hih6131_init( void );

extern uint8_t ICACHE_FLASH_ATTR hih6131_read_temp( int8_t * at_int, uint8_t * at_fract );
extern uint8_t ICACHE_FLASH_ATTR hih6131_read_humidity( uint8_t * hm_int, uint8_t * hm_fract );




#endif	// #if USE_HIH6131 == 1

#endif /* MK_PRESSURE_LIB_MK_HIH6131_H_ */
