/*
 * 		Miros³aw Kardaœ
 * 		czysty szablon projektu dla NonOS SDK
 *
 * 		ESP8266 NONOS SDK - API
 *
 */
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "espnow.h"


#define LED_IO_MUX     PERIPHS_IO_MUX_GPIO2_U
#define LED_IO_NUM     2
#define LEDIO_FUNC     FUNC_GPIO2


uint8 slave1_mac[] = {0x5C,0xCF,0x7F,0x76,0x60,0xA5};	// ESP12E w zestawie ATB
//uint8 slave2_mac[] = {};



os_timer_t rtc_sw_timer;

void ICACHE_FLASH_ATTR rtc_sw_timer_cb( void *arg ) {

	static uint8 licznik;

	char buf[128];
	os_memset( buf, 0, sizeof(buf) );
	os_sprintf( buf, "Licznik: %d", licznik++ );


	esp_now_send( 0, buf, os_strlen(buf) );


}







void ICACHE_FLASH_ATTR OnEspNowDataSent( uint8_t *mac_addr, uint8_t sendStatus ) {


  if( sendStatus == 0 ) os_printf( "Pakiet odebrany\r\n" );
  else os_printf( "Pakiet nie odebrany\r\n" );

}


void ICACHE_FLASH_ATTR EspNowOnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {

	if( len == 1 && incomingData[0] == '0' ) {
		GPIO_OUTPUT_SET( LED_IO_NUM, 0 );
	}

	if( len == 1 && incomingData[0] == '1' ) {
		GPIO_OUTPUT_SET( LED_IO_NUM, 1 );
	}

}




void ICACHE_FLASH_ATTR on_user_init_completed( void ) {

	uint8 mac[6];
	wifi_get_macaddr( 0, mac );
	os_printf( "+MAC_ ADDR: 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X",
			mac[0],mac[1],mac[2],mac[3],mac[4],mac[5] );


	if( !esp_now_init() ) {
		os_printf( "\r\nESP NOW - Started OK!\r\n" );

//		esp_now_set_self_role( ESP_NOW_ROLE_CONTROLLER );
		esp_now_set_self_role( ESP_NOW_ROLE_COMBO );
		esp_now_add_peer( slave1_mac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0 );
//		esp_now_add_peer( slave2_mac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0 );
		esp_now_register_send_cb( OnEspNowDataSent );
		esp_now_register_recv_cb( EspNowOnDataRecv );

		os_timer_disarm( &rtc_sw_timer );
		os_timer_setfn( &rtc_sw_timer, rtc_sw_timer_cb, NULL );
		os_timer_arm( &rtc_sw_timer, 2000, 1 );
	}


	os_printf( "\r\nuser_init COMPLETED !!!!!!!! \r\n" );
}



void ICACHE_FLASH_ATTR user_init(void) {

	uart_div_modify( 0, UART_CLK_FREQ / 115200 );
	os_delay_us( 100 );

	system_init_done_cb( on_user_init_completed );

//	wifi_set_opmode( NULL_MODE );
	wifi_set_opmode( STATION_MODE );



	os_printf( "\r\nready\r\n" );
}

