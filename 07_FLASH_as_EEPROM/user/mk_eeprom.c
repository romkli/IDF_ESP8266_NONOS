/*
 * mk_eeprom.c
 *
 *  Created on: 20 sty 2022
 *      Author: Miros³aw Kardaœ
 */
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "mk_eeprom.h"

typedef struct {
    uint32 flag;
} TMKSAVEFLAG;



TMYCONFIG myconfig;

LOCAL TMKSAVEFLAG mksaveflag;


/*

	0xf38 - dane (backup)
	0x3f9 - dane (backup)
	0x3fA - FLAGA

*/


void ICACHE_FLASH_ATTR flash_cfg_save( void ) {

	uint8 res;

	 spi_flash_read( (CFG_LOCATION + 2) * SPI_FLASH_SEC_SIZE,
	                   (uint32 *)&mksaveflag, sizeof(TMKSAVEFLAG) );

	 if (mksaveflag.flag == 0) {

			spi_flash_erase_sector(CFG_LOCATION + 1);
			res = spi_flash_write( (CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
							(uint32 *)&myconfig, sizeof(TMYCONFIG) );

			if( SPI_FLASH_RESULT_OK == res) {
				mksaveflag.flag = 1;
				spi_flash_erase_sector(CFG_LOCATION + 2);
				spi_flash_write( (CFG_LOCATION + 2) * SPI_FLASH_SEC_SIZE,
								(uint32 *)&mksaveflag, sizeof(TMKSAVEFLAG) );
			}

	 } else {

			spi_flash_erase_sector(CFG_LOCATION + 0);
			res = spi_flash_write( (CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
							(uint32 *)&myconfig, sizeof(TMYCONFIG ));

			if( SPI_FLASH_RESULT_OK == res) {
				mksaveflag.flag = 0;
				spi_flash_erase_sector(CFG_LOCATION + 2);
				spi_flash_write( (CFG_LOCATION + 2) * SPI_FLASH_SEC_SIZE,
								(uint32 *)&mksaveflag, sizeof(TMKSAVEFLAG) );
			}
	 }
}




void ICACHE_FLASH_ATTR flash_cfg_load( uint8 reset_to_defaults ) {

	 spi_flash_read( (CFG_LOCATION + 2) * SPI_FLASH_SEC_SIZE,
	                   (uint32 *)&mksaveflag, sizeof(TMKSAVEFLAG) );

		if (mksaveflag.flag == 0) {
			spi_flash_read( (CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
						   (uint32 *)&myconfig, sizeof(TMYCONFIG) );
		} else {
			spi_flash_read( (CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
						   (uint32 *)&myconfig, sizeof(TMYCONFIG) );
		}


		if( myconfig.cfg_marker != CFG_MARKER || reset_to_defaults ) {

			os_memset( &myconfig, 0, sizeof(TMYCONFIG) );
			myconfig.cfg_marker = CFG_MARKER;


//			myconfig.name[0] = 0;
			os_strcat( myconfig.name, "mirekk36 z Atnela" );
			os_strcat( myconfig.str1, "NazwaESP" );


			os_strcat( myconfig.mkwifi[0].ssid, "MirMur" );
			os_strcat( myconfig.mkwifi[0].pass, "55AA55AA55" );

			myconfig.mkwifi[0].ip_info.ip.addr 		= ipaddr_addr( "192.168.2.170" );
			myconfig.mkwifi[0].ip_info.gw.addr 		= ipaddr_addr( "192.168.2.1" );
			myconfig.mkwifi[0].ip_info.netmask.addr = ipaddr_addr( "255.255.255.0" );



			os_strcat( myconfig.mkwifi[1].ssid, "Atnel-2.4GHz" );
			os_strcat( myconfig.mkwifi[1].pass, "55AA55AA55" );

			myconfig.mkwifi[1].ip_info.ip.addr 		= ipaddr_addr( "192.168.1.170" );
			myconfig.mkwifi[1].ip_info.gw.addr 		= ipaddr_addr( "192.168.1.1" );
			myconfig.mkwifi[1].ip_info.netmask.addr = ipaddr_addr( "255.255.255.0" );


			os_printf( "\r\nDEFAULTS LOADED NOW!\r\n" );

			flash_cfg_save();					// save defaults
		}

}

