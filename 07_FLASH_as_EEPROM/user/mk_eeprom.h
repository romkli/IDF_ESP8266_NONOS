/*
 * mk_eeprom.h
 *
 *  Created on: 20 sty 2022
 *      Author: Miros³aw Kardaœ
 */

#ifndef USER_MK_EEPROM_H_
#define USER_MK_EEPROM_H_


#define CFG_MARKER    	0x55AA5AA5

#define CFG_LOCATION    0x3f8



typedef struct {
	char 	ssid[33];
	char 	pass[65];
	struct info {
	    struct ip_addr ip;
	    struct ip_addr netmask;
	    struct ip_addr gw;
	} ip_info;
} TMKWIFI;


typedef struct {
	uint32	 	cfg_marker;
	char 		name[100];
	char 		str1[100];
	char 		str2[100];
	TMKWIFI 	mkwifi[5];
} TMYCONFIG;



extern TMYCONFIG myconfig;






extern void ICACHE_FLASH_ATTR flash_cfg_save( void );
extern void ICACHE_FLASH_ATTR flash_cfg_load( uint8 reset_to_defaults );



#endif /* USER_MK_EEPROM_H_ */
