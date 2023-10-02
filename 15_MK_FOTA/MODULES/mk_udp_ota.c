/*
 * mk_udp_ota.c
 *
 *  Created on: 11 lut 2022
 *      Author: Miros³aw Kardaœ
 */
#include <stdlib.h>

#include "osapi.h"
#include "ets_sys.h"
#include "at_custom.h"
#include "user_interface.h"
#include "mem.h"
#include "upgrade.h"

#include "espconn.h"

#include "MODULES/mk_udp_ota.h"

#include "MK_LCD/mk_lcd44780.h"

enum { _idle, _rcv_data };

uint8 mk_udp_fota_state = _idle;



struct espconn ota_udp;
esp_udp ota_udp_proto;

uint32 ota_base_addr = 0x1000;  // 0X101000
uint32 ota_address;
uint8 * flash_sector_buf = NULL;


uint16 sct_cnt;
uint16 fota_bin_sector_count;

LOCAL os_timer_t ota_reboot_timer, ota_timeout_timer;


void ICACHE_FLASH_ATTR ota_timeout_timer_cb( void *arg ) {
	at_port_print("+FOTA TIMEOUT - RESTART:\r\n");
	system_restart();
}


uint8 mk_fota_write_flash( char *data, uint16_t len ) {

	char buf[128];

	static uint8 sect_idx;

	if( len == 1024 ) {
		if( !sect_idx ) {
			if( flash_sector_buf == NULL ) {
				flash_sector_buf = (uint8 *)os_malloc(SPI_FLASH_SEC_SIZE);
			}
			os_bzero( flash_sector_buf, 4096 );
		}
		os_memmove( flash_sector_buf+(sect_idx*1024), data, 1024 );
		sect_idx++;

		if( sect_idx > 3 ) {	// wypal sektor w pamiêci FLASH
			sect_idx = 0;

			spi_flash_erase_sector( ota_address / SPI_FLASH_SEC_SIZE );

			SpiFlashOpResult res = spi_flash_write( ota_address, (uint32 *)flash_sector_buf, SPI_FLASH_SEC_SIZE );

			ota_address += 4096;

        	sct_cnt++;

        	os_sprintf( buf, "@sct=%d", sct_cnt );
        	espconn_send( &ota_udp, buf, os_strlen( buf ) );

        	os_sprintf( buf, "+Sector: ( %d / %d )\r\n", sct_cnt, fota_bin_sector_count );
        	at_port_print( buf );

        	if( sct_cnt == fota_bin_sector_count ) {
        		// KONIEC
        		return 1;
        	}
		}
	} else {

    	os_sprintf( buf, "ERROR sect: 0x%X", ota_base_addr );
    	espconn_send( &ota_udp, buf, os_strlen( buf ) );
	}

	return 0;
}

















void ICACHE_FLASH_ATTR udp_receive_cb( void *arg, char *data, uint16_t len ) {

	char buf[128];

	struct espconn *conn = (struct espconn *)arg;

	remot_info * premot = NULL;
	if( espconn_get_connection_info( conn, &premot, 0 ) == ESPCONN_OK ) {
//		os_sprintf( buf,"\r\n+UDP_RX:"IPSTR":%d\r\n", IP2STR(premot->remote_ip), premot->remote_port );
//		os_printf( buf );
	}


	if( len == 11 && !os_strcmp( data, "FotaInitUDP" ) ) {
    	os_sprintf( buf, "+FOTA UDP READY:" );
    	espconn_send(conn, buf, os_strlen( buf ) );

        os_timer_disarm(&ota_timeout_timer);
        os_timer_setfn(&ota_timeout_timer, (os_timer_func_t *)ota_timeout_timer_cb, NULL);
        os_timer_arm(&ota_timeout_timer, 4000, 0);

        lcd_cls(0);
        lcd_str( "FOTA - download!" );
        lcd_cls(2);
        lcd_str( "progress: 0%" );
	}


// tu nie mo¿e byæ else

	if( mk_udp_fota_state == _idle && !os_strcmp( data, "@GetBinNr" ) ) {

		uint8 unit = system_upgrade_userbin_check();
        if (unit == UPGRADE_FW_BIN1) {
        	ota_base_addr = 0x101000;
        	os_sprintf( buf, "+SET:user2.bin" );
        	espconn_send(conn, buf, os_strlen( buf ) );

        } else {
        	ota_base_addr = 0x1000;
        	os_sprintf( buf, "+SET:user1.bin" );
            espconn_send(conn, buf, os_strlen( buf ) );
        }
        ota_address = ota_base_addr;

        char bf[50];
        os_sprintf( bf, "%s\r\n", buf );
        at_port_print(bf);
	}

	else if( mk_udp_fota_state == _idle && !os_strncmp( data, "BinLen=", 7 ) ) {
		char * p;
		char * wsk = strtok_r( data, "=", &p );
		if( !p[0] ) return;

		fota_bin_sector_count = atoll( p );
		sct_cnt = 0;

        mk_udp_fota_state = _rcv_data;
        at_port_print("+RCV BIN DATA:\r\n");
	}

	else if( mk_udp_fota_state == _rcv_data ) {
		uint8 res = mk_fota_write_flash( data, len );

        os_sprintf( buf,"%d%", (sct_cnt*100) / fota_bin_sector_count );
        lcd_locate(1, 10);
        lcd_str( buf );

		if( res ) {
			lcd_cls(1);
			lcd_str( "FOTA OK! RESTART" );
			system_upgrade_flag_set( UPGRADE_FLAG_FINISH );
			at_port_print("+Scheduling RESTART: 2 sec.\r\n");
	        os_timer_disarm(&ota_reboot_timer);
	        os_timer_setfn(&ota_reboot_timer, (os_timer_func_t *)system_upgrade_reboot, NULL);
	        os_timer_arm(&ota_reboot_timer, 2000, 0);
	        return;
		}
	}

    os_timer_disarm(&ota_timeout_timer);
    os_timer_arm(&ota_timeout_timer, 5000, 0);
}




void ICACHE_FLASH_ATTR mk_fota_udp_init( char * remote_ip, uint16_t remote_port, uint16_t local_port ) {

	ip_addr_t ip;
	ip.addr = ipaddr_addr( remote_ip );

	espconn_delete( &ota_udp );

	ota_udp_proto.local_port = local_port;

	ota_udp.type = ESPCONN_UDP;
	ota_udp.state = ESPCONN_NONE;
	ota_udp.proto.udp = &ota_udp_proto;

	espconn_create( &ota_udp );

	ota_udp.proto.udp->remote_ip[0] = ip4_addr1( &ip );
	ota_udp.proto.udp->remote_ip[1] = ip4_addr2( &ip );
	ota_udp.proto.udp->remote_ip[2] = ip4_addr3( &ip );
	ota_udp.proto.udp->remote_ip[3] = ip4_addr4( &ip );

	ota_udp.proto.udp->remote_port = remote_port;

	if( local_port ) {
		espconn_regist_recvcb( &ota_udp, udp_receive_cb );
	}

	char buf[128];

	uint8 unit = system_upgrade_userbin_check();
	uint8 nr;
	if (unit == UPGRADE_FW_BIN1) nr = 1;
	else nr = 2;

	os_sprintf( buf, "MK FOTA UDP STARTED:%d", nr );
	espconn_send( &ota_udp, buf, os_strlen( buf ) );

}


void ICACHE_FLASH_ATTR srv_udp_send( char * data, uint16 len ) {
	espconn_send( &ota_udp, data, os_strlen( data ) );
}
























