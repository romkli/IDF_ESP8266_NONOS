/*
 * 	czysty szablon z obs³ug¹ komend AT
 * 	by: mirekk36
 *
 */

#include "osapi.h"
#include "ets_sys.h"
#include "at_custom.h"
#include "user_interface.h"

#include "my_at_cmd.h"




void ICACHE_FLASH_ATTR on_user_init_completed( void ) {



	at_port_print_irom_str( "\r\nuser_init COMPLETED !!!!!!!! \r\n" );
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

    at_port_print_irom_str("\r\nready\r\n");

//    wifi_set_opmode( NULL_MODE );	// wy³¹cz WiFi

    my_custom_at_cmd_init();

}
