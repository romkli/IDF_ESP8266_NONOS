/*
*	mirekk36
*/
#ifndef _MK_UTILS_H_
#define	_MK_UTILS_H_

#include "c_types.h"


uint8_t ICACHE_FLASH_ATTR mk_str_to_ip( const char * str, void *ip );
uint8_t ICACHE_FLASH_ATTR mk_isIPv4 ( const char *str );

#endif	// end _MK_UTILS_H_
