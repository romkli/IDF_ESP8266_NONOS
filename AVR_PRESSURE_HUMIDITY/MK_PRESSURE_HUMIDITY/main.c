/*
 * main.c
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
 *
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>


#include "common.h"

#include "MK_LCD/mk_lcd44780.h"
#include "MK_I2C/mk_i2c.h"

#include "MK_PRESSURE_HUMIDITY_LIB/mk_pressure_cfg.h"


const uint8_t stopien_ico[] PROGMEM 	= {12,18,18,12,32,32,32,32};
const uint8_t hum_ico[] PROGMEM 	= {4,4,10,10,17,17,17,14};
const uint8_t temp_ico[] PROGMEM 	= {4,10,10,10,17,31,31,14};
const uint8_t hpa_ico[] PROGMEM 	= {4,4,21,14,4,32,10,21};


char buf[17];



int main( void ) {

	LED_DIR |= LED;	// LED
	LED_OFF;

	lcd_init();
	lcd_defchar_P(0, stopien_ico);
	lcd_defchar_P(1, temp_ico);
	lcd_defchar_P(2, hum_ico);
	lcd_defchar_P(3, hpa_ico);
	lcd_cls();



#ifdef TCCR0B
	TCCR0A = (1<<WGM01);
	TCCR0B |= (1<<CS02)|(1<<CS00);
	TIMSK0 |= (1<<OCIE0A);
	OCR0A = F_CPU/1024ul/100ul;
#else
	TCCR0 = (1<<WGM01);
	TCCR0 |= (1<<CS02)|(1<<CS00);
	TIMSK |= (1<<OCIE0);
	OCR0 = F_CPU/1024ul/100ul;
#endif

	i2c_init( 400 );			// nie zapomnij o inicjalizacji i2c i prêdkoœci w kHz


	uint8_t res = mk_press_hum_init();


	strcat_P( buf, PSTR("Czujniki: "));


#if USE_BME_BMP_280 == 1
	if( (res & st_bmp280) || (res & st_bme280) ) {
		if( (res & st_bme280) ) strcat_P( buf, PSTR("BME280 "));
		else strcat_P( buf, PSTR("BMP280 "));
	}
#endif

#if USE_BMP_085_180 == 1
	if( res & st_bmp180 ) {
		if( strlen( buf ) > 12 ) {
			lcd_str( buf );
			lcd_locate(1,0);
			buf[0] = 0;
		}
		strcat_P( buf, PSTR("BMP180 "));
	}
#endif


#if USE_HIH6131 == 1
	if( res & st_hih6131 ) {
		if( strlen( buf ) > 10 ) {
			lcd_str( buf );
			lcd_locate(1,0);
			buf[0] = 0;
		}
		strcat_P( buf, PSTR("HIH6131"));
	}
#endif

	if( res == st_none ) {
		lcd_str_P( PSTR("Czujniki: Brak ") );
		while(1);	// gdy nie wykryto czujnika - STOP
	}

	lcd_str( buf );

	myDelay(1500);
	lcd_cls();



	sei();


	while(1) {

		/* TEMPERATURA w stopniach Celsiusza */
		int8_t t_int = 5;
		uint8_t t_fract;
		if( !mkp_read_temp( st_bme280, &t_int, &t_fract ) ) {
			lcd_locate(0,0);
			lcd_str( "\x81" );
			lcd_int( t_int );
			lcd_str( "." );
			if( t_fract < 10 ) lcd_str_P( PSTR("0") );
			lcd_int( t_fract );
			lcd_str( "\x80""C" );
		}


		/* CIŒNIENIE ATMOSFERYCZNE w hPa */
		uint16_t hp_int;
		uint8_t hp_fract;
		if( !mkp_read_pressure( st_bme280, &hp_int, &hp_fract ) ) {
			lcd_locate(1,2);
			lcd_str( "\x83" );
			lcd_long( hp_int );
			lcd_str( "." );
			if( hp_fract < 10 ) lcd_str_P( PSTR("0") );
			lcd_int( hp_fract );
			lcd_str( " hPa" );
		}


		/* WILGOTNOŒÆ POWIETRZA w % (0-100) */
		uint8_t hm_int;
		uint8_t hm_fract;
		if( !mkp_read_humidity( st_bme280, &hm_int, &hm_fract ) ) {
			lcd_locate(0,9);
			lcd_str( "\x82" );
			lcd_long( hm_int );
			lcd_str( "." );
			if( hm_fract < 10 && hm_int < 100 ) lcd_str_P( PSTR("0") );
			lcd_int( hm_fract );
			lcd_str( "% " );
		}

		_delay_ms( 500 );
	}
}







