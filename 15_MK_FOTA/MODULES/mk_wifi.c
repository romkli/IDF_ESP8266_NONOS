/*
 * mk_wifi.c
 *
 *  Created on: 7 lut 2022
 *      Author: Miros³aw Kardaœ
 */
#include "ets_sys.h"
#include "osapi.h"
#include "at_custom.h"
#include "user_interface.h"

#include "MODULES/mk_wifi.h"
#include "MODULES/mk_udp_ota.h"

void ICACHE_FLASH_ATTR wifi_handle_event_cb( System_Event_t * evt ) {

	char buf[128] = {0};

	switch (evt->event) {

	case EVENT_STAMODE_CONNECTED:

		os_sprintf( buf, "\r\n+STA_CONNECTED_SSID: %s, channel %d\r\n",
		evt->event_info.connected.ssid,
		evt->event_info.connected.channel);
		at_port_print( buf );

		// tu mo¿na w³¹czyæ diodê LED NIEBIESK¥

		break;

	case EVENT_STAMODE_DISCONNECTED:

		os_sprintf( buf,"\r\n+STA_DISCONNECTED_FROM_SSID:%s, reason %d\n",
		evt->event_info.disconnected.ssid,
		evt->event_info.disconnected.reason);
		at_port_print(buf);

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
		at_port_print( buf );

		// tuaj wa¿ny moment

		// tu mo¿na w³¹czyæ diodê LED ZIELON¥

//		mk_sntp_get_time();

		mk_fota_udp_init( UDP_FOTA_SERVER, 8888, 4444 );


		break;

	case EVENT_SOFTAPMODE_STACONNECTED:

		os_sprintf(buf, "\r\n+AP_MAC_CONNECTED:" MACSTR ", AID = %d\r\n",
		MAC2STR(evt->event_info.sta_connected.mac),
		evt->event_info.sta_connected.aid);
		at_port_print( buf );

		//

		break;

	case EVENT_SOFTAPMODE_STADISCONNECTED:

		os_sprintf(buf, "\r\n+AP_MAC_DISCONNECTED:" MACSTR ", AID = %d\n",
		MAC2STR(evt->event_info.sta_disconnected.mac),
		evt->event_info.sta_disconnected.aid);
		at_port_print(buf);



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

