
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"


#include "mk_eeprom.h"


void ICACHE_FLASH_ATTR on_user_init_completed( void ) {

	flash_cfg_load( 0 );


	os_printf( "\r\n----- EEPROM INFO -----\r\n\r\n" );

	os_printf( "Name: %s\r\n", myconfig.name );
	os_printf( "str1: %s\r\n", myconfig.str1 );
	os_printf( "str2: %s\r\n\r\n", myconfig.str2 );

	os_printf( "\r\n...... Acces Point 1 ........\r\n" );
	os_printf( "SSID: %s\r\n", myconfig.mkwifi[0].ssid );
	os_printf( "PASS: %s\r\n", myconfig.mkwifi[0].pass );

	os_printf( "IP:\t" IPSTR "\r\n", IP2STR( &myconfig.mkwifi[0].ip_info.ip.addr ) );
	os_printf( "MASK:\t" IPSTR "\r\n", IP2STR( &myconfig.mkwifi[0].ip_info.netmask.addr ) );
	os_printf( "GW:\t" IPSTR "\r\n", IP2STR( &myconfig.mkwifi[0].ip_info.gw.addr ) );


	os_printf( "\r\n\r\n...... Acces Point 2 ........\r\n" );
	os_printf( "SSID: %s\r\n", myconfig.mkwifi[1].ssid );
	os_printf( "PASS: %s\r\n", myconfig.mkwifi[1].pass );

	os_printf( "IP:\t" IPSTR "\r\n", IP2STR( &myconfig.mkwifi[1].ip_info.ip.addr ) );
	os_printf( "MASK:\t" IPSTR "\r\n", IP2STR( &myconfig.mkwifi[1].ip_info.netmask.addr ) );
	os_printf( "GW:\t" IPSTR "\r\n", IP2STR( &myconfig.mkwifi[1].ip_info.gw.addr ) );



}



void ICACHE_FLASH_ATTR user_init(void) {

	// gpioX - wejœcie + podci¹gniêcie VCC
	// if( PIN = GND ) flash_cfg_load( 1 );


	uart_div_modify( 0, UART_CLK_FREQ / 115200 );
	os_delay_us( 100 );

	system_init_done_cb( on_user_init_completed );

	wifi_set_opmode( NULL_MODE );



	os_printf( "\r\nSys Init:\r\n" );







}

