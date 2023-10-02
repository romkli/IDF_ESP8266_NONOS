/*
 * mk_sntp.h
 *
 *  Created on: 29 sty 2022
 *      Author: Miros³aw Kardaœ
 *
 *      ver: 1.0
 *
 *      ESP8266 NonOS SDK
 */

#ifndef USER_MK_SNTP_H_
#define USER_MK_SNTP_H_


enum { czas_zimowy=1, czas_letni=2 };


typedef struct {
	uint32	unix_time;
	uint8 	hh;
	uint8	mm;
	uint8	ss;
	uint8	YY;
	uint16	YYYY;
	uint8	MM;
	char	MM_NAME[12];
	uint8	DD;
	char	DD_NAME[13];
	uint8	day_nr;
	uint8	time_zone;
	uint8	short_names;
	char	time_str[32];
	char	date_str[32];
	char	datetime_str[64];
} TMKDATETIME;


typedef void (* TGET_MK_DATETIME_CB)( TMKDATETIME * mkdt );


extern TMKDATETIME mk_datetime;



//------------------------------

extern void ICACHE_FLASH_ATTR register_mk_datetime_cb( TGET_MK_DATETIME_CB cb );


extern void ICACHE_FLASH_ATTR mk_sntp_init( void );
extern void ICACHE_FLASH_ATTR mk_sntp_get_time( void );
extern void ICACHE_FLASH_ATTR mk_set_sntp_servers( char * ntp1, char * ntp2, char * ntp3 );
extern void ICACHE_FLASH_ATTR mk_sntp_get_ntp_servers( void );

extern void ICACHE_FLASH_ATTR mk_sntp_set_tzone( int8_t tzone );
extern void ICACHE_FLASH_ATTR mk_sntp_get_tzone( void );

#endif /* USER_MK_SNTP_H_ */
