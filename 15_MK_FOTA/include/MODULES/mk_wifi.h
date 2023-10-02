/*
 * mk_wifi.h
 *
 *  Created on: 7 lut 2022
 *      Author: Miros³aw Kardaœ
 */

#ifndef INCLUDE_MODULES_MK_WIFI_H_
#define INCLUDE_MODULES_MK_WIFI_H_


#define WIFI_SSID			"Atnel-2.4GHz"
//#define WIFI_SSID			"MirMUR"
#define WIFI_PASS			"55AA55AA55"


#define UDP_FOTA_SERVER		"192.168.1.100"
//#define UDP_FOTA_SERVER		"192.168.2.130"




extern void ICACHE_FLASH_ATTR WIFI_Connect( uint8_t * ssid, uint8_t * pass );

#endif /* INCLUDE_MODULES_MK_WIFI_H_ */
