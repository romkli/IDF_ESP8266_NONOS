/*
 * mk_sntp.c
 *
 *  Created on: 29 sty 2022
 *      Author: Miros³aw Kardaœ
 *
 *      ver: 1.0
 *
 *      ESP8266 NonOS SDK
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "osapi.h"
#include "at_custom.h"
#include "user_interface.h"
#include "sntp.h"
#include "mem.h"
#include "ip_addr.h"
#include "time.h"


#include "espconn.h"

#include "MODULES/mk_sntp.h"


TMKDATETIME	mk_datetime;


static const char day_name[7][13] ICACHE_RODATA_ATTR = {
	"Niedziela",
	"Poniedzia³ek",
	"Wtorek",
	"Œroda",
	"Czwartek",
	"Pi¹tek",
	"Sobota"
};

static const char mon_name[12][12] ICACHE_RODATA_ATTR = {
	"Styczeñ",
	"Luty",
	"Marzec",
	"Kwiecieñ",
	"Maj",
	"Czerwiec",
	"Lipiec",
	"Sierpieñ",
	"Wrzesieñ",
	"PaŸdziernik",
	"Listopad",
	"Grudzieñ"
};




LOCAL TGET_MK_DATETIME_CB get_mk_datetime_cb;


void ICACHE_FLASH_ATTR register_mk_datetime_cb( TGET_MK_DATETIME_CB cb ) {
	get_mk_datetime_cb = cb;
}


LOCAL os_timer_t sntp_timer;

char * ICACHE_FLASH_ATTR sntp_asctime1_r(struct tm *tim_p ,char *result) {

	mk_datetime.YYYY 	= 1900 + tim_p->tm_year;
	mk_datetime.YY		= tim_p->tm_year-100;
	mk_datetime.MM		= tim_p->tm_mon+1;
	mk_datetime.DD		= tim_p->tm_mday;
	mk_datetime.day_nr	= tim_p->tm_wday;

	os_memset( mk_datetime.MM_NAME, 0, sizeof(mk_datetime.MM_NAME) );
	os_memset( mk_datetime.DD_NAME, 0, sizeof(mk_datetime.DD_NAME) );
	os_memset( mk_datetime.time_str, 0, sizeof(mk_datetime.time_str) );
	os_memset( mk_datetime.date_str, 0, sizeof(mk_datetime.date_str) );
	os_memset( mk_datetime.datetime_str, 0, sizeof(mk_datetime.datetime_str) );

	if( !mk_datetime.short_names ) {
		os_strcat( mk_datetime.MM_NAME, mon_name[tim_p->tm_mon] );
		os_strcat( mk_datetime.DD_NAME, day_name[tim_p->tm_wday] );
	} else {
		os_memcpy( mk_datetime.MM_NAME, mon_name[tim_p->tm_mon], 3 );
		os_memcpy( mk_datetime.DD_NAME, day_name[tim_p->tm_wday], 3 );
	}

	mk_datetime.hh		= tim_p->tm_hour;
	mk_datetime.mm		= tim_p->tm_min;
	mk_datetime.ss		= tim_p->tm_sec;


  os_sprintf (result, "%04d-%02d-%02d %02d:%02d:%02d,%s %d %s %02d",
		  1900 + tim_p->tm_year,
		  tim_p->tm_mon+1,
		  tim_p->tm_mday,

		  tim_p->tm_hour,
		  tim_p->tm_min,
		  tim_p->tm_sec,

		  day_name[tim_p->tm_wday],
		  tim_p->tm_mday,
		  mon_name[tim_p->tm_mon],
		  1900 + tim_p->tm_year

	   );


  os_strcat( mk_datetime.datetime_str, result );
  os_sprintf( mk_datetime.time_str, "%02d:%02d:%02d", mk_datetime.hh, mk_datetime.mm, mk_datetime.ss );
  os_sprintf( mk_datetime.date_str, "%04d-%02d-%02d", mk_datetime.YYYY, mk_datetime.MM, mk_datetime.DD );

  if( get_mk_datetime_cb ) {
	  get_mk_datetime_cb( &mk_datetime );
  }

  return result;
}


char *ICACHE_FLASH_ATTR sntp_asctime1(struct tm *tim_p) {
	char reult[100];
    return sntp_asctime1_r (tim_p, reult);
}

char* sntp_get_real_time1(time_t t) {
	return sntp_asctime1( localtime (&t) );
}

void ICACHE_FLASH_ATTR user_check_sntp_stamp_callback(void *arg) {

	os_timer_disarm(&sntp_timer);
	os_timer_arm(&sntp_timer, 100, 1);

	if( wifi_station_get_connect_status() != STATION_GOT_IP ) return;

	uint32 current_stamp;
	char buf[128] = {0};

	current_stamp = sntp_get_current_timestamp();
	if (current_stamp == 0) {
		static uint16_t tmo;
		if( tmo < 250 ) {					// sprawdzaj czas 150 razy co 200 ms
			os_timer_disarm(&sntp_timer);
			os_timer_arm(&sntp_timer, 100, 0);
		} else {
			os_timer_disarm(&sntp_timer);
			os_sprintf(buf,"\r\n+SNTP_TIME:ERROR TIMEOUT\r\n");
			at_port_print(buf);
			tmo=0;
		}
		tmo++;
	} else {
		os_timer_disarm(&sntp_timer);

		os_sprintf(buf,"\r\n+SNTP_TIME:%d,%s\r\n", current_stamp,
		sntp_get_real_time1(current_stamp));

		if( !get_mk_datetime_cb ) {
			at_port_print(buf);
		}
	}
}


void ICACHE_FLASH_ATTR mk_sntp_init( void ) {

	/* https://pl.wikipedia.org/wiki/Serwer_czasu */
	sntp_setservername(0, "time.nist.gov"); // set server 0 by domain name - mo¿e byæ przekazany przez IP

	sntp_setservername(1, "ntp.itl.waw.pl"); // set server 1 by domain name - mo¿e byæ przekazany przez IP

	/* tu sposób na przekazywanie serwera ntp tylko przez adres IP w postaci stringa */
	ip_addr_t *addr = (ip_addr_t *)os_zalloc(sizeof(ip_addr_t));
	addr->addr = ipaddr_addr("80.50.231.226");	// 80.50.231.226 - atomowy zegar cezowy 5071A TP S.A.Orange Polska S.A. wg https://pl.wikipedia.org/wiki/Serwer_czasu
	sntp_setserver(2, addr); // set server 2 by IP address

	sntp_stop();
	sntp_set_timezone( mk_datetime.time_zone );
	sntp_init();
	os_free(addr);



	os_timer_disarm(&sntp_timer);
	os_timer_setfn(&sntp_timer, (os_timer_func_t *)user_check_sntp_stamp_callback, NULL);
	os_timer_arm(&sntp_timer, 2500, 1);

}


void ICACHE_FLASH_ATTR mk_set_sntp_servers( char * ntp1, char * ntp2, char * ntp3 ) {

	if( ntp1 == NULL ) sntp_setservername(0, "time.nist.gov"); // set server 0 by domain name - mo¿e byæ przekazany przez IP
	else sntp_setservername(0, ntp1 );

	if( ntp2 == NULL )sntp_setservername(1, "ntp.itl.waw.pl"); // set server 1 by domain name - mo¿e byæ przekazany przez IP
	else sntp_setservername(0, ntp2 );

	if( ntp3 == NULL )sntp_setservername(1, "ntp1.tp.pl"); // set server 1 by domain name - mo¿e byæ przekazany przez IP
	else sntp_setservername(0, ntp3 );

}


void ICACHE_FLASH_ATTR mk_sntp_get_time( void ) {
	os_timer_disarm(&sntp_timer);
	os_timer_arm(&sntp_timer, 100, 1);
}

void ICACHE_FLASH_ATTR mk_sntp_get_ntp_servers( void ) {

	char buf[128] = {0};
	char * sntpptr;
	ip_addr_t ip_sntp;

	sntpptr = sntp_getservername(0);
	if( *sntpptr ) os_sprintf(buf,"\r\n+SNTP_SRV1:%s (main server)\r\n", sntpptr );
	else {
		if( sntpptr ) {
			ip_sntp = sntp_getserver(0);
			os_sprintf(buf, "\r\n+SNTP_SRV1:" IPSTR " (main server)\r\n", IP2STR(&ip_sntp.addr) );
		}
		else os_sprintf(buf,"\r\n+SNTP_SRV1:none (main server)\r\n");
	}
	at_port_print(buf);

	sntpptr = sntp_getservername(1);
	if( *sntpptr ) os_sprintf(buf,"+SNTP_SRV2:%s (backup server)\r\n", sntpptr );
	else {
		if( sntpptr ) {
			ip_sntp = sntp_getserver(1);
			os_sprintf(buf, "+SNTP_SRV2:" IPSTR " (backup server)\r\n", IP2STR(&ip_sntp.addr) );
		}
		else os_sprintf(buf,"+SNTP_SRV2:none (backup server)\r\n");
	}
	at_port_print(buf);

	sntpptr = sntp_getservername(2);
	if( *sntpptr ) os_sprintf(buf,"+SNTP_SRV3:%s (backup server)\r\n", sntpptr );
	else {
		if( sntpptr ) {
			ip_sntp = sntp_getserver(2);
			os_sprintf(buf, "+SNTP_SRV3:" IPSTR " (backup server)\r\n", IP2STR(&ip_sntp.addr) );
		}
		else os_sprintf(buf,"+SNTP_SRV3:none (backup server)\r\n");
	}
	at_port_print(buf);

	os_sprintf(buf,"+SNTP_TZONE:%d\r\n", sntp_get_timezone() );
	at_port_print(buf);
}


void ICACHE_FLASH_ATTR mk_sntp_set_tzone( int8_t tzone ) {
	
	sntp_stop();

	mk_datetime.time_zone = tzone;
	
	if( sntp_set_timezone( tzone ) ) {
		sntp_init();
	}
}

void ICACHE_FLASH_ATTR mk_sntp_get_tzone( void ) {

	char buf[128] = {0};

	os_sprintf(buf,"+SNTP_TZONE:%d\r\n", sntp_get_timezone() );
	at_port_print(buf);

}
