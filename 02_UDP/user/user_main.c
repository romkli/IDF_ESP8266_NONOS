

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "espconn.h"

#define STA_IP				"192.168.1.170"
#define STA_MASK			"255.255.255.0"
#define STA_GW				"192.168.1.1"


struct espconn udp;
esp_udp udp_proto;





static os_timer_t gpio_sw_timer;

void ICACHE_FLASH_ATTR gpio_sw_timer_cb( void *arg ) {

	static uint8_t licznik;
	char buf[128];
	os_sprintf( buf, "Leci liczba: %d\r\n", licznik++ );

	espconn_send( &udp, buf, os_strlen(buf) );

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





void ICACHE_FLASH_ATTR wifi_handle_event_cb( System_Event_t * evt ) {

	char buf[128] = {0};

	switch (evt->event) {

	case EVENT_STAMODE_CONNECTED:

		os_sprintf( buf, "\r\n+STA_CONNECTED_SSID: %s, channel %d\r\n",
		evt->event_info.connected.ssid,
		evt->event_info.connected.channel);
		os_printf( buf );

		// tu mo¿na w³¹czyæ diodê LED NIEBIESK¥

		break;

	case EVENT_STAMODE_DISCONNECTED:

		os_sprintf( buf,"\r\n+STA_DISCONNECTED_FROM_SSID:%s, reason %d\n",
		evt->event_info.disconnected.ssid,
		evt->event_info.disconnected.reason);
		os_printf(buf);

		// tu mo¿na WYL¥CZYÆ diodê LED NIEBIESK¥
		// tu mo¿na WYL¥CZYÆ diodê LED ZIELON¥

		break;

	case EVENT_STAMODE_AUTHMODE_CHANGE:

		os_printf("mode:	%d	->	%d\n",
		evt->event_info.auth_change.old_mode,
		evt->event_info.auth_change.new_mode);
		break;

	case EVENT_STAMODE_GOT_IP:

		os_sprintf(buf, "\r\n+STA_IP:" IPSTR "\r\n+STA_MASK:" IPSTR "\r\n+STA_GW:" IPSTR "\r\n",
		IP2STR(&evt->event_info.got_ip.ip),
		IP2STR(&evt->event_info.got_ip.mask),
		IP2STR(&evt->event_info.got_ip.gw));
		os_printf( buf );

		// tuaj wa¿ny moment

		// tu mo¿na w³¹czyæ diodê LED ZIELON¥

		os_timer_disarm( &gpio_sw_timer );
		os_timer_arm( &gpio_sw_timer, 1000, 1 );

		break;

	case EVENT_SOFTAPMODE_STACONNECTED:

		os_sprintf(buf, "\r\n+AP_MAC_CONNECTED:" MACSTR ", AID = %d\r\n",
		MAC2STR(evt->event_info.sta_connected.mac),
		evt->event_info.sta_connected.aid);
		os_printf( buf );

		//

		break;

	case EVENT_SOFTAPMODE_STADISCONNECTED:

		os_sprintf(buf, "\r\n+AP_MAC_DISCONNECTED:" MACSTR ", AID = %d\n",
		MAC2STR(evt->event_info.sta_disconnected.mac),
		evt->event_info.sta_disconnected.aid);
		os_printf(buf);



		break;

	default:

		break;

	}
}




void ICACHE_FLASH_ATTR WIFI_Connect( uint8_t * ssid, uint8_t * pass ) {

	wifi_station_disconnect();

	wifi_set_event_handler_cb( wifi_handle_event_cb );

	struct station_config stationConf;
	os_memset( &stationConf, 0, sizeof( struct station_config ) );
	os_sprintf( stationConf.ssid, "%s", ssid );
	os_sprintf( stationConf.password, "%s", pass );
	wifi_station_set_config( &stationConf );

	// ustawienie statycznego adresu IP dla naszego ESP
	// odkomentuj poni¿sze linie i ustaw w³asny adres statyczny w #define
	wifi_station_dhcpc_stop();
	struct ip_info info;
	info.ip.addr = ipaddr_addr( STA_IP );
	info.gw.addr = ipaddr_addr( STA_GW );
	info.netmask.addr = ipaddr_addr( STA_MASK );
	wifi_set_ip_info( STATION_IF, &info );


	wifi_station_connect();
}




void ICACHE_FLASH_ATTR scan_done_cb( void *arg, STATUS status ) {

	int ret;
	uint8 ssid[128];
	char temp[128];
	uint8 res = 0;

	os_printf("\r\n+WIFI SCAN PROGRESS: wait...\r\n");

	if (status == OK) {

		struct bss_info *bss_link = (struct bss_info*) arg;

		while ( bss_link != NULL ) {

			os_memset( ssid, 0, sizeof(ssid) );
			os_memcpy( ssid, bss_link->ssid, os_strlen(bss_link->ssid) );

			os_sprintf( temp, "(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n",
					bss_link->authmode, ssid, bss_link->rssi,
					MAC2STR(bss_link->bssid), bss_link->channel);

			os_printf( temp );

			bss_link = bss_link->next.stqe_next;
		}

		os_printf("\r\n+WIFI SCAN END: OK\r\n");

		WIFI_Connect( "Atnel-2.4GHz", "55AA55AA55" );

	} else {
		os_printf("\r\n+SCAN FAIL: ERROR\r\n");
	}
}





void ICACHE_FLASH_ATTR on_user_init_completed( void ) {

	os_printf("\r\n+WIFI SCAN START:\r\n");
	wifi_station_scan( NULL, scan_done_cb );



	os_printf( "\r\nready - user_init COMPLETED !!!!!!!! \r\n" );
}







void ICACHE_FLASH_ATTR user_init( void ) {

	uart_div_modify( 0, UART_CLK_FREQ / 115200 );
	os_delay_us( 100 );

	system_init_done_cb( on_user_init_completed );


	wifi_set_opmode( STATIONAP_MODE );


	os_timer_disarm( &gpio_sw_timer );
	os_timer_setfn( &gpio_sw_timer, gpio_sw_timer_cb, NULL );
//	os_timer_arm( &gpio_sw_timer, 1000, 1 );

	udp_init( "192.168.1.188", 8888, 4444 );

	os_printf( "\r\nready - user_init end\r\n" );

}

