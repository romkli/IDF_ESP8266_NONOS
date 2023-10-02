/*
 * mk_udp.c
 *
 *  Created on: 7 lut 2022
 *      Author: Miros³aw Kardaœ
 */
#include <stdlib.h>

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"


#include "espconn.h"

#include "MODULES/mk_udp.h"
#include "MODULES/mk_uart.h"




struct espconn udp;
esp_udp udp_proto;




void ICACHE_FLASH_ATTR udp_receive_cb( void *arg, char *data, uint16_t len ) {

	if( !transparent_mode_enabled ) return;


//	struct espconn *conn = (struct espconn *)arg;
//
//	remot_info * premot = NULL;
//	if( espconn_get_connection_info( conn, &premot, 0 ) == ESPCONN_OK ) {
////		os_sprintf( buf,"\r\n+UDP_RX:"IPSTR":%d\r\n", IP2STR(premot->remote_ip), premot->remote_port );
////		os_printf( buf );
//	}

//	if( premot->remote_ip == )

	uart0_send_buf( data, len );



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












