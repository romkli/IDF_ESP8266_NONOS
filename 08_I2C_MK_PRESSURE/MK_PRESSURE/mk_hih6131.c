/*
 * mk_hih6131.c
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


#include "MK_PRESSURE/mk_pressure_cfg.h"


#if USE_HIH6131 == 1

#include "MODULES/mk_i2c.h"

#include "MK_PRESSURE/mk_hih6131.h"




static int8_t hih_temp1;
static uint8_t hih_temp2;

static uint8_t hih_hm1;
static uint8_t hih_hm2;

// kdo Ÿród³owy dla AVR do pobrania ze strony:
//		https://sklep.atnel.pl/pl/p/MK-PRESSURE-HUMIDITY-AVR-Biblioteka-C/282





#endif	// #if USE_HIH6131 == 1
