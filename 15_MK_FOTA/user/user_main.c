/*
 * 	MK FOTA - HTTP / UDP / AT
 * 	by: mirekk36
 *
 */

#include "osapi.h"
#include "ets_sys.h"
#include "at_custom.h"
#include "user_interface.h"

#include "my_at_cmd.h"

#include "driver/mk_i2c.h"
#include "MK_LCD/mk_lcd44780.h"

#include "MODULES/mk_wifi.h"

#include "MODULES/mk_udp_ota.h"
#include "MODULES/mk_http_ota.h"
#include "MODULES/mk_debug.h"


void ICACHE_FLASH_ATTR on_user_init_completed( void ) {

	i2c_init( aBITRATE_260_600KHZ );

	lcd_init();
	lcd_cls(0);

	lcd_str( "ESP8266 U/H FOTA" );
	lcd_locate(1, 0);

	uint8 unit = system_upgrade_userbin_check();
    if (unit == UPGRADE_FW_BIN1) {
    	lcd_str( "user1 - 0x001000" );
    } else {
    	lcd_str( "user2 - 0x101000" );
    }


	at_port_print_irom_str( "\r\n+USER_INIT COMPLETED:\r\n" );

	WIFI_Connect( WIFI_SSID, WIFI_PASS );
}




void ICACHE_FLASH_ATTR user_init( void ) {

    char buf[128];

	uart_div_modify( 0, UART_CLK_FREQ / 115200 );
	os_delay_us( 100 );

    at_customLinkMax = 5;	// maksymalna liczba po³¹czeñ - trzeba podaæ przed at_init(); !!!

    system_init_done_cb( on_user_init_completed );

    at_init();

#ifdef ESP_AT_FW_VERSION
    if ((ESP_AT_FW_VERSION != NULL) && (os_strlen(ESP_AT_FW_VERSION) < 64)) {
        os_sprintf(buf,"compile time:"__DATE__" "__TIME__"\r\n"ESP_AT_FW_VERSION);
    } else {
        os_sprintf(buf,"compile time:"__DATE__" "__TIME__);
    }
#else
    os_sprintf( buf,"compile time:"__DATE__" "__TIME__ " by mirekk36" );
#endif

    at_set_custom_info(buf);	// na potrzeby standardowej komendy AT+GMR

    at_port_print_irom_str("\r\n+READY:\r\n");

    wifi_set_opmode( STATION_MODE );



    my_custom_at_cmd_init();

}
