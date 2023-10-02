/*
 * 		Mirosław Kardaś
 * 		czysty szablon projektu dla NonOS SDK
 *
 * 		ESP8266 NONOS SDK - API
 *
 */
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"








void ICACHE_FLASH_ATTR on_user_init_completed( void ) {



	os_printf( "\r\nuser_init COMPLETED !!!!!!!! \r\n" );
}



void ICACHE_FLASH_ATTR user_init(void) {

	uart_div_modify( 0, UART_CLK_FREQ / 115200 );
	os_delay_us( 100 );

	system_init_done_cb( on_user_init_completed );


	wifi_set_opmode( NULL_MODE );	// wyłącz WiFi




	os_printf( "\r\nready\r\n" );
}

