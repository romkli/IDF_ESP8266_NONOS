/*
 * 	czysty szablon z obs³ug¹ komend AT
 * 	by: mirekk36
 *
 */

#include "osapi.h"
#include "ets_sys.h"
#include "at_custom.h"
#include "user_interface.h"

#include "driver/uart.h"

#include "my_at_cmd.h"

#include "driver/mk_i2c.h"
#include "MK_LCD/mk_lcd44780.h"

#include "MODULES/mk_wifi.h"
#include "MODULES/mk_udp.h"
#include "MODULES/mk_uart.h"




void ICACHE_FLASH_ATTR on_user_init_completed( void ) {

	i2c_init( aBITRATE_100KHZ );

	lcd_init();
	lcd_cls(0);

	lcd_str( "Custom  AT CMD's" );
	lcd_locate(1, 0);
	lcd_str( "Kurs  ESP i RTOS" );

	at_port_print_irom_str( "\r\n+USER INIT COMPLETED:\r\n" );

	WIFI_Connect( WIFI_SSID, WIFI_PASS );

	transparent_init();

	change_transparent_mode( 1 );




}



void ICACHE_FLASH_ATTR user_init( void ) {




    char buf[128];

    at_customLinkMax = 5;	// maksymalna liczba po³¹czeñ - trzeba podaæ przed at_init(); !!!

    system_init_done_cb( on_user_init_completed );

    at_init();

    /***** jeœli mamy zmieniæ baudrate to koniecznie po at_init() *******/
    //UART_SetBaudrate( 0, 115200 );


#ifdef ESP_AT_FW_VERSION
    if ((ESP_AT_FW_VERSION != NULL) && (os_strlen(ESP_AT_FW_VERSION) < 64)) {
        os_sprintf(buf,"compile time:"__DATE__" "__TIME__"\r\n"ESP_AT_FW_VERSION);
    } else {
        os_sprintf(buf,"compile time:"__DATE__" "__TIME__);
    }
#else
    os_sprintf( buf,"+COMPILE TIME:"__DATE__" "__TIME__ " by mirekk36" );
#endif

    at_set_custom_info(buf);	// na potrzeby standardowej komendy AT+GMR

    at_port_print_irom_str("\r\n+READY:\r\n");

    wifi_set_opmode( STATION_MODE );

    udp_init( "192.168.1.100", 8888, 4444 );

    my_custom_at_cmd_init();

}
