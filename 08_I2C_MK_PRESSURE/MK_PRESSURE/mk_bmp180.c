/*
 * MkBMP180.c
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

#include <math.h>


#include "MK_PRESSURE/mk_pressure_cfg.h"


#if USE_BMP_085_180 == 1

#include "MODULES/mk_i2c.h"

#include "MK_PRESSURE/mk_bmp180.h"





static TBMPCALIB bmp180;



// kdo �r�d�owy dla AVR do pobrania ze strony:
//		https://sklep.atnel.pl/pl/p/MK-PRESSURE-HUMIDITY-AVR-Biblioteka-C/282





#endif		// #if USE_BMP_085_180 == 1







