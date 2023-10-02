/*
 * mk_udp.h
 *
 *  Created on: 7 lut 2022
 *      Author: admin
 */

#ifndef INCLUDE_MODULES_MK_UDP_H_
#define INCLUDE_MODULES_MK_UDP_H_





extern struct espconn udp;





extern void ICACHE_FLASH_ATTR udp_init( char * remote_ip, uint16_t remote_port, uint16_t local_port );






#endif /* INCLUDE_MODULES_MK_UDP_H_ */
