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
#include "espconn.h"

#include "driver/mk_i2c.h"
#include "MK_LCD/mk_lcd44780.h"



uint8 slave1_mac[] = {0xE8,0xDB,0x84,0xDD,0x42,0x99};	// Node MCU


struct espconn udp;
esp_udp udp_proto;



void ICACHE_FLASH_ATTR EspNowOnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {

	char buf[251];
	static char bf[251];

	os_memset( buf, 0, sizeof(buf) );
	os_memcpy( buf, incomingData, len );

	os_printf( buf );
	os_printf( "\r\n" );

	lcd_cls(1);
	lcd_str( buf );
	lcd_cls(2);
	lcd_str(bf);

	os_memset( bf, 0, sizeof(bf) );
	os_memcpy( bf, incomingData, len );

	espconn_send( &udp, buf, os_strlen(buf) );
}




void ICACHE_FLASH_ATTR on_user_init_completed( void ) {

	i2c_init( aBITRATE_100KHZ );

	lcd_init();
	lcd_cls(0);

	lcd_str( "ESP-NOW JEDZIEMY" );
	lcd_locate(1, 0);
	lcd_str( "Kurs  ESP i RTOS" );

	uint8 mac[6];
	wifi_get_macaddr( 0, mac );

	os_printf( "MAC ADDR: 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X",
			mac[0],mac[1],mac[2],mac[3],mac[4],mac[5] );


	if( !esp_now_init() ) {
		os_printf( "\r\nESP NOW - Started OK!\r\n" );

//		esp_now_set_self_role( ESP_NOW_ROLE_SLAVE );
		esp_now_set_self_role( ESP_NOW_ROLE_COMBO );
		esp_now_add_peer( slave1_mac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0 );
		esp_now_register_recv_cb( EspNowOnDataRecv );

	} else {
		os_printf( "\r\nESP NOW - not started ERROR!\r\n" );
	}


	os_printf( "\r\nuser_init COMPLETED !!!!!!!! \r\n" );
}


void ICACHE_FLASH_ATTR udp_receive_cb( void *arg, char *data, uint16_t len ) {

	char buf[128];

	struct espconn *conn = (struct espconn *)arg;

	remot_info * premot = NULL;
	if( espconn_get_connection_info( conn, &premot, 0 ) == ESPCONN_OK ) {
		os_sprintf( buf,"\r\n+UDP_RX:"IPSTR":%d\r\n", IP2STR(premot->remote_ip), premot->remote_port );
		os_printf( buf );
	}

	os_memset( buf, 0, sizeof(buf) );
	os_memcpy( buf, data, len );
	os_printf( "+UDP_DATA:" );
	os_printf( buf );

	lcd_cls(1);
	lcd_str( buf );

	if( len == 1 && (data[0] == '0' || data[0] == '1') ) {
		esp_now_send( slave1_mac, data, 1 );
	}
}




void ICACHE_FLASH_ATTR udp_init( char * remote_ip, uint16_t remote_port, uint16_t local_port ) {

	ip_addr_t ip;
	ip.addr = ipaddr_addr( remote_ip );

	espconn_delete( &udp );

	udp_proto.local_port = local_port;

	udp.type = ESPCONN_UDP;
	udp.state = ESPCONN_NONE;
	udp.proto.udp = &udp_proto;

	espconn_create( &udp );

	udp.proto.udp->remote_ip[0] = ip4_addr1( &ip );
	udp.proto.udp->remote_ip[1] = ip4_addr2( &ip );
	udp.proto.udp->remote_ip[2] = ip4_addr3( &ip );
	udp.proto.udp->remote_ip[3] = ip4_addr4( &ip );

	udp.proto.udp->remote_port = remote_port;

	if( local_port ) {
		espconn_regist_recvcb( &udp, udp_receive_cb );
	}
}





void ICACHE_FLASH_ATTR user_init(void) {

	uart_div_modify( 0, UART_CLK_FREQ / 115200 );
	os_delay_us( 100 );

	system_init_done_cb( on_user_init_completed );


//	wifi_set_opmode( NULL_MODE );
//	wifi_set_opmode( STATION_MODE );
	wifi_set_opmode( STATIONAP_MODE );


	udp_init( "192.168.4.2", 8888, 4444 );

	os_printf( "\r\nready\r\n" );
}

