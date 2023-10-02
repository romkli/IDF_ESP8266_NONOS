/*
    FOTA HTTP for the ESP8266
    by mirekk36

*/
#include <stdlib.h>

#include "osapi.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "upgrade.h"

#include "MODULES/mk_utils.h"
#include "MODULES/mk_http_ota.h"
#include "MODULES/mk_debug.h"


#define pHeadStatic "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: agient mirekk36 ;) z ESP8266 NonOS\r\n\
Accept: */*\r\n"

typedef struct {
  char *host;			// http://atnel.pl	or  atnel.pl  or  192.168.1.100  or http://192.168.1.100
  char *filepath;		// /user2.bin  or /devices/sensor7/user1.bin
  uint16_t port;
  struct espconn *conn;
  struct upgrade_server_info *up_server;
  void *reverse;
} TMKFOTA;


static os_timer_t fota_restart_timer;
static TMKFOTA mk_fota;
static uint8 mk_fota_restart_flg;

LOCAL void ICACHE_FLASH_ATTR fota_restart() {

  if (mk_fota_restart_flg == 0) {
    mk_fota_restart_flg = 1;
    os_timer_disarm(&fota_restart_timer);
    os_timer_setfn(&fota_restart_timer, (os_timer_func_t *)fota_restart, NULL);
    os_timer_arm(&fota_restart_timer, 2000, 0);
  } else {
    system_restart();
  }
}

LOCAL void ICACHE_FLASH_ATTR fota_start_esp_connect(struct espconn *conn,
		void *connect_cb, void *disconnect_cb, void *reconn_cb) {

  espconn_regist_connectcb(conn, connect_cb);
  espconn_regist_disconcb(conn, disconnect_cb);
  espconn_regist_reconcb(conn, reconn_cb);

  if (espconn_connect(conn) != 0) {
    INFO("MK FOTA: Connect fail\n");
    fota_restart();
  }
}

LOCAL void ICACHE_FLASH_ATTR fota_response(void *arg) {

  struct upgrade_server_info *server = arg;

  if(server->upgrade_flag == true) {
    INFO("MK FOTA: Firmware upgrade success! / RESTART\n");
    mk_fota_restart_flg = 1;
    system_upgrade_reboot();
  }
  else {
    INFO("MK FOTA: Firmware upgrade fail\n");
    fota_restart();
  }
}

LOCAL void ICACHE_FLASH_ATTR fota_recon_cb(void *arg, sint8 err) {

	if( mk_fota_restart_flg ) return;

	if( -11 == err ) INFO("MK FOTA: FOTA client can't connect to Host\n");
	else INFO("MK FOTA: FOTA upgrade reconnect callback, error code %d\n", err);


//  fota_restart();
}

LOCAL void ICACHE_FLASH_ATTR fota_discon_cb(void *arg) {

  INFO("MK FOTA: FOTA client disconnect / restart\n");
  fota_restart();
}

LOCAL void ICACHE_FLASH_ATTR fota_connect_cb(void *arg) {

  struct espconn *pespconn = (struct espconn *)arg;
  TMKFOTA *afota_client = (TMKFOTA *)pespconn->reverse;
  char temp[32] = {0};
  uint8_t i = 0;

  INFO("MK FOTA: FOTA client connected\n");
  system_upgrade_init();

  afota_client->up_server = (struct upgrade_server_info *)os_zalloc(sizeof(struct upgrade_server_info));
  afota_client->up_server->upgrade_version[5] = '\0';       // no, we dont use this
  afota_client->up_server->pespconn = pespconn;
  os_memcpy(afota_client->up_server->ip, pespconn->proto.tcp->remote_ip, 4);
  afota_client->up_server->port = afota_client->port;
  afota_client->up_server->check_cb = fota_response;

  afota_client->up_server->check_times = 20000;
  if(afota_client->up_server->url == NULL) {
    afota_client->up_server->url = (uint8 *) os_zalloc(1024);
  }

  os_sprintf(afota_client->up_server->url, "GET %s HTTP/1.1\r\nHost: %s\r\n"pHeadStatic"\r\n",
		  afota_client->filepath, afota_client->host);

  if(system_upgrade_start(afota_client->up_server) == false) {
    INFO("MK FOTA: Fail to start system upgrade\n");
    fota_restart();
  }
}

LOCAL void ICACHE_FLASH_ATTR fota_dns_found( const char *name, ip_addr_t *ipaddr, void *arg ) {

  struct espconn *pespconn = (struct espconn *)arg;
  TMKFOTA *ota_client = (TMKFOTA *)pespconn->reverse;

  if(ipaddr == NULL) {
    INFO("MK FOTA: DNS Found, but got no ip\n");
    //fota_restart();
    return;
  }

  INFO("MK FOTA: DNS found ip %d.%d.%d.%d\n",
      *((uint8 *) &ipaddr->addr),
      *((uint8 *) &ipaddr->addr + 1),
      *((uint8 *) &ipaddr->addr + 2),
      *((uint8 *) &ipaddr->addr + 3));

  if(ipaddr->addr != 0) {
    os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);
  }
  // TODO:
  fota_start_esp_connect(ota_client->conn, fota_connect_cb, fota_discon_cb, fota_recon_cb);
}



LOCAL void ICACHE_FLASH_ATTR start_fota( TMKFOTA *afota_client ) {


	afota_client->conn = (struct espconn*) os_zalloc(sizeof(struct espconn));
	afota_client->conn->reverse = (void*) afota_client;
	afota_client->conn->type = ESPCONN_TCP;
	afota_client->conn->state = ESPCONN_NONE;

	afota_client->conn->proto.tcp = (esp_tcp*) os_zalloc(sizeof(esp_tcp));
	afota_client->conn->proto.tcp->local_port = espconn_port();
	afota_client->conn->proto.tcp->remote_port = afota_client->port;

	// if ip address is provided, go ahead
	if( mk_str_to_ip( afota_client->host,
			&afota_client->conn->proto.tcp->remote_ip) ) {
		INFO("MK FOTA: FOTA client connecting to IP: %s:%d\r\n",
				afota_client->host, afota_client->port);

		fota_start_esp_connect( afota_client->conn, fota_connect_cb,
				fota_discon_cb, fota_recon_cb );
	}

	// else, use dns query to get ip address
	else {
		INFO("MK FOTA: FOTA client connecting to domain: %s:%d\r\n",
				afota_client->host, afota_client->port);

		espconn_gethostbyname( afota_client->conn, afota_client->host,
				(ip_addr_t*) (afota_client->conn->proto.tcp->remote_ip),
				fota_dns_found );
	}


}




/*
* For now, only http is supported, but a different port can be used.
* But we are working on https
*
* 	only user1.bin or user2.bin have to be downloaded
*
*	available names conventions for url's or direct IP
*
*   http://atnel.pl:80/data/user2.bin
*   http://atnel.pl/data/user1.bin
*   http://atnel.pl/user2.bin
*   http://test.atnel.pl/user1.bin
*   192.168.2.130/user2.bin
*   192.168.2.130/data/user2.bin
*   http://192.168.2.130:8000/user2.bin
*   test.atnel.pl/devices/user2.bin
*   myhttpsrv/user2.bin
*/
void ICACHE_FLASH_ATTR mk_start_http_fota( char * url_or_ip ) {

  int i;
  char *str, *str2, *p, *q;
  TMKFOTA *afota_client = &mk_fota;

  mk_fota_restart_flg = 0;

  os_memset( afota_client, 0, sizeof(TMKFOTA) );

  str = strtok_r(url_or_ip, "/", &p);     // http: or 192.168.2.130:80 or myhttpsrv
  str2 = strstr(str, ":");          // : or :80 or null

  if( str2 && os_strlen(str2) == 1) {
    p++;                            // skip http://
    str = strtok_r(NULL, "/", &p);  // myhttpsrv:80 or myhttpsrv
    str2 = strstr(str, ":");        // :80 or null
  }
  if (str2) {
    i = str2 - str;
    str[i] = 0;                     // myhttpsrv
    afota_client->host = (char*)os_zalloc(os_strlen(str)+1);
    os_strncpy(afota_client->host, str, os_strlen(str));
    str[i] = ':';                   // myhttpsrv:80
    str2++;
    afota_client->port = atoi(str2);
  } else {
    afota_client->host = (char*)os_zalloc(os_strlen(str)+1);
    os_strncpy(afota_client->host, str, os_strlen(str));
    afota_client->port = 80;
  }
  i = os_strlen(str);
  str[i] = '/';
  p--;
  str = strtok_r(NULL, ".", &p);    // PATH --> /data/user1
  i = os_strlen(str);
  str[i] = '.';                     // PATH --> /devices/user1.bin
  if(system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
    str[i-1] = '2';                 // user2.bin	-----> need to be downloaded
  if(system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
    str[i-1] = '1';                 // user1.bin	-----> need to be downloaded
  afota_client->filepath = (char*)os_zalloc(os_strlen(str)+1);
  os_strncpy(afota_client->filepath, str, os_strlen(str));

  INFO( "FOTA: File [%s] have to be downloaded\n", &str[1] );

  start_fota( afota_client );
}
