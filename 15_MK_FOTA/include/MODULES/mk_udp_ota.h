/*
 * mk_udp_ota.h
 *
 *  Created on: 11 lut 2022
 *      Author: Miros³aw Kardaœ
 */

#ifndef USER_MK_UDP_OTA_H_
#define USER_MK_UDP_OTA_H_














extern void ICACHE_FLASH_ATTR mk_fota_udp_init( char * remote_ip, uint16_t remote_port, uint16_t local_port );

extern void ICACHE_FLASH_ATTR srv_udp_send( char * data, uint16 len );

#endif /* USER_MK_UDP_OTA_H_ */
