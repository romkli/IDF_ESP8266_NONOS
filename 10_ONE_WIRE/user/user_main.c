/*
 * MK ONE WIRE
 *
 *  Created on: 2022-01-26
 *      by: Miros³aw Kardaœ
 */
#include <stdlib.h>

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"


#include "MK_PRESSURE/mk_pressure_cfg.h"
#include "MK_LCD/mk_lcd44780.h"
#include "driver/mk_i2c.h"

#include "MK_ONE_WIRE/ds18x20.h"

uint8_t czujniki_cnt;
int8 subzero, cel, cel_fract_bits;


const uint8_t kr[] ICACHE_RODATA_ATTR = {32,32,4,32,32,4,32,32};
const uint8_t stC[] ICACHE_RODATA_ATTR = {12,18,18,12,32,32,32,32};


static os_timer_t rtc_sw_timer;

void ICACHE_FLASH_ATTR rtc_sw_timer_cb( void *arg ) {

	char buf[128];
	static uint8 sw;

	if( !sw ) {
		czujniki_cnt = search_sensors();
//		os_printf( "iloœæ czujników na magistrali: %d \r\n", czujniki_cnt );
		DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL );
	}


	if( czujniki_cnt > 0 && sw==1 && DS18X20_OK == DS18X20_read_meas( gSensorIDs[0], &subzero, &cel, &cel_fract_bits ) ) {
		if( subzero ) cel = cel * -1;
		os_sprintf( buf, "(1)Temperatura DS18B20: %d,%d C\r\n", cel, cel_fract_bits );
		os_printf( buf );
		lcd_cls(1);
		os_sprintf( buf, "[1]  IN: %d,%d" "\x81" "C", cel, cel_fract_bits );
		lcd_str( buf );
	}

	if( czujniki_cnt > 1 && sw==1 && DS18X20_OK == DS18X20_read_meas( gSensorIDs[1], &subzero, &cel, &cel_fract_bits ) ) {
		if( subzero ) cel = cel * -1;
		os_sprintf( buf, "(2)Temperatura DS18B20: %d,%d C\r\n", cel, cel_fract_bits );
		os_printf( buf );
		lcd_cls(2);
		os_sprintf( buf, "[2] OUT: %d,%d" "\x81" "C", cel, cel_fract_bits );
		lcd_str( buf );
	}


	if( sw==2 ) {
		/* CIŒNIENIE ATMOSFERYCZNE w hPa */
		uint16_t hp_int;
		uint8_t hp_fract;
		if( !mkp_read_pressure( st_bmp280, &hp_int, &hp_fract ) ) {
			os_sprintf( buf, "BMP280 - Ciœnienie: %d,%02d hPa\r\n", hp_int, hp_fract );
			os_printf( buf );
			lcd_cls(0);
			os_sprintf( buf, "BMP280  Pressure" );
			lcd_str( buf );
			lcd_cls(2);
			os_sprintf( buf, "\x80""  %d,%02d hPa ""\x80", hp_int, hp_fract );
			lcd_str( buf );
		}
	}


	sw += 1;
	if( sw>2 ) sw = 0;
}





void ICACHE_FLASH_ATTR on_user_init_completed( void ) {

	if( system_update_cpu_freq( 160 ) ) {
		os_printf( "\r\n***** F_CPU = 160 MHz *****\r\n" );
	}

	i2c_init( aBITRATE_100KHZ );

	mk_press_hum_init();

	DS18X20_init();

	lcd_init();
	lcd_cls(0);

	lcd_defchar( 0, kr );
	lcd_defchar( 1, stC );

	lcd_cls(0);

	lcd_str( "MK LCD 3.0 " "\x80" " ESP" );
	lcd_locate(1, 0);
	lcd_str( "COG Display  OK!" );


	os_timer_disarm( &rtc_sw_timer );
	os_timer_setfn( &rtc_sw_timer, rtc_sw_timer_cb, NULL );
	os_timer_arm( &rtc_sw_timer, 1000, 1 );

}


void ICACHE_FLASH_ATTR user_init(void) {

	uart_div_modify( 0, UART_CLK_FREQ / 115200 );
	os_delay_us( 100 );

	system_init_done_cb( on_user_init_completed );

	wifi_set_opmode( NULL_MODE );




	os_printf( "\r\nSys Init:\r\n" );


}

