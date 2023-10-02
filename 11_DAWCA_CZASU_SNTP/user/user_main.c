/*
 * mk_sntp library
 *
 *  Created on: 29 sty 2022
 *      Author: Miros³aw Kardaœ
 *
 *      ver: 1.0
 *
 *      ESP8266 NonOS SDK
 */
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "MODULES/mk_sntp.h"


#define WIFI_SSID			"Atnel-2.4GHz"
#define WIFI_PASS			"55AA55AA55"




void ICACHE_FLASH_ATTR get_mk_datetime_cb( TMKDATETIME * mkdt ) {

//	os_printf( "\r\n ************ Date & Time ***************\r\n" );
//
//	os_printf( "czas:\t\t\t%s\r\n", mk_datetime.time_str );
//	os_printf( "data:\t\t\t%s\r\n", mk_datetime.date_str );
//	os_printf( "data i czas:\t%s\r\n", mk_datetime.datetime_str );
//
//	os_printf( "miesi¹c:\t\t%s\r\n", mk_datetime.MM_NAME );
//	os_printf( "dzieñ:\t\t%s\r\n", mk_datetime.DD_NAME );
//
//	os_printf( "dzieñ nr:\t\t%d\r\n", mk_datetime.day_nr );

	char buf[128];
	os_sprintf( buf, "\r\n+MY_TIME:%02d.%02d.%02d %02d:%02d\r\n",
			mk_datetime.YY, mk_datetime.MM, mk_datetime.DD,
			mk_datetime.hh, mk_datetime.mm
			);
	os_printf( buf );
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

		register_mk_datetime_cb( get_mk_datetime_cb );
		mk_datetime.time_zone = czas_zimowy;
//		mk_datetime.short_names = 1;
		mk_sntp_init();

		mk_sntp_get_tzone();
		mk_sntp_get_ntp_servers();


//		mk_sntp_get_time();


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
//	wifi_station_dhcpc_stop();
//	struct ip_info info;
//	info.ip.addr = ipaddr_addr( STA_IP );
//	info.gw.addr = ipaddr_addr( STA_GW );
//	info.netmask.addr = ipaddr_addr( STA_MASK );
//	wifi_set_ip_info( STATION_IF, &info );

	wifi_station_connect();
}




void ICACHE_FLASH_ATTR on_user_init_completed( void ) {

	WIFI_Connect( WIFI_SSID, WIFI_PASS );


	os_printf( "\r\nready - user_init COMPLETED !!!!!!!! \r\n" );
}




void ICACHE_FLASH_ATTR user_init(void) {


	uart_div_modify( 0, UART_CLK_FREQ / 115200 );
	os_delay_us( 100 );

	system_init_done_cb( on_user_init_completed );


	wifi_set_opmode( STATION_MODE );


}

