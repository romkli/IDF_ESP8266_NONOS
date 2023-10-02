

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "espconn.h"

#include "mem.h"


/*------------- WiFi START ------------------------------------------------*/
#define WIFI_SSID			"Atnel-2.4GHz"
//#define WIFI_SSID			"MirMUR"

#define STA_IP				"192.168.1.170"
#define STA_MASK			"255.255.255.0"
#define STA_GW				"192.168.1.1"

//#define STA_IP			"192.168.2.170"
//#define STA_MASK			"255.255.255.0"
//#define STA_GW			"192.168.2.2"
/*............. WiFi end ..................................................*/

/*-------------- UDP START ------------------------------------------------*/
#define UDP_REMOTE_IP		"192.168.1.188"
//#define UDP_REMOTE_IP		"192.168.2.116"

#define UDP_REMOTE_PORT		8888

struct espconn udp;
esp_udp udp_proto;
/*.............. UDP end ..................................................*/


/*-------------- TCP START ------------------------------------------------*/

#define TCP_MODE			0		// 0-TCP Server, 1-TCP Client


#if TCP_MODE == 1

#define TCP_REMOTE_IP		"192.168.1.188"
//#define TCP_REMOTE_IP		"192.168.2.116"

#define TCP_REMOTE_PORT		9999

// reconnect
os_timer_t tcp_auto_reconnect_sw_timer;
#endif


struct espconn tcp;
esp_tcp tcp_proto;

// gdybyœmy chcieli rozbudowaæ kod do SCENARIUSZA II-go
//struct espconn tcp1, tcp2;
//esp_tcp tcp1_proto, tcp2_proto;

#if TCP_MODE == 0

#define	MAX_TCP_CLIENTS		3

// na potrzeby TASK'a do zabijania nieproszonych klientów TCP
#define SIG_CLI_REJECT 		0
#define SIG_CLI_QUEUE_LEN 	4

typedef struct {
  struct espconn *conn; 	// The tracked connection.
  uint8 idx;        		// The index in connection pool.
  uint8 remote_ip[4];      	// The remote IP associated with the connection.
  int 	remote_port;        // The remote port associated with the connection.
} TCLI_CONN;

static TCLI_CONN * tcp_cli_pool[ MAX_TCP_CLIENTS ];

char tx[] = "No room for new tcp client!";

#endif



/*.............. TCP end ..................................................*/





/*-------------- SYS START ------------------------------------------------*/
static os_timer_t gpio_sw_timer;
/*.............. SYS end ..................................................*/





/*-------------- UDP START ------------------------------------------------*/
void ICACHE_FLASH_ATTR udp_receive_cb( void *arg, char *data, uint16_t len ) {

	char buf[128];

	struct espconn *conn = (struct espconn *)arg;

	remot_info * premot = NULL;
	if( espconn_get_connection_info( conn, &premot, 0 ) == ESPCONN_OK ) {
		os_sprintf( buf,"\r\n+UDP_RX:"IPSTR":%d\r\n", IP2STR(premot->remote_ip), premot->remote_port );
		os_printf( buf );
	}

	os_memset( buf, 0, sizeof(buf) );
	os_memcpy( buf, data, len );
	os_printf( "+UDP_DATA:" );
	os_printf( buf );
}

void ICACHE_FLASH_ATTR udp_init( char * remote_ip, uint16_t remote_port, uint16_t local_port ) {

	ip_addr_t ip;
	ip.addr = ipaddr_addr( remote_ip );

	espconn_delete( &udp );

	udp_proto.local_port = local_port;

	udp.type = ESPCONN_UDP;
	udp.state = ESPCONN_NONE;
	udp.proto.udp = &udp_proto;

	espconn_create( &udp );

	udp.proto.udp->remote_ip[0] = ip4_addr1( &ip );
	udp.proto.udp->remote_ip[1] = ip4_addr2( &ip );
	udp.proto.udp->remote_ip[2] = ip4_addr3( &ip );
	udp.proto.udp->remote_ip[3] = ip4_addr4( &ip );

	udp.proto.udp->remote_port = remote_port;

	if( local_port ) {
		espconn_regist_recvcb( &udp, udp_receive_cb );
	}
}
/*.............. UDP end ..................................................*/







/*-------------- TCP START ------------------------------------------------*/


/*------------ OBS£UGA TASK'a -----------------------------*/
#if TCP_MODE == 0

os_event_t *CLI_REJECT_Queue;

void cli_reject_task( os_event_t *e ) {

	switch ( e->sig ) {
		case SIG_CLI_REJECT:
			os_printf( "\r\nTASK ubi³ klienta jak psa\r\n" );
			espconn_disconnect( (struct espconn *)e->par );
			break;
		default:
			break;
	}
}


void task_init(void) {
	CLI_REJECT_Queue = (os_event_t*) os_malloc( sizeof(os_event_t) * SIG_CLI_QUEUE_LEN );

	system_os_task( cli_reject_task, USER_TASK_PRIO_0, CLI_REJECT_Queue, SIG_CLI_QUEUE_LEN );
}


void cli_reject_task_post( struct espconn * cli ) {

	system_os_post( USER_TASK_PRIO_0, SIG_CLI_REJECT, (os_param_t)cli );
}

#endif
/*------------ OBS£UGA TASK'a -----------------------------*/


//----------------------------- SEKCJA RECONNECT start ------------------------------------------
#if TCP_MODE == 1

int8 ICACHE_FLASH_ATTR tcp_connect_as_client( char * remote_ip, uint16_t remote_port ) {

	ip_addr_t ip;
	ip.addr = ipaddr_addr( remote_ip );

	tcp.proto.tcp->remote_ip[0] = ip4_addr1( &ip );
	tcp.proto.tcp->remote_ip[1] = ip4_addr2( &ip );
	tcp.proto.tcp->remote_ip[2] = ip4_addr3( &ip );
	tcp.proto.tcp->remote_ip[3] = ip4_addr4( &ip );

	tcp.proto.tcp->remote_port = remote_port;

	return espconn_connect( &tcp );
}


void ICACHE_FLASH_ATTR tcp_reconnect_as_client( uint32 ms ) {

	os_timer_disarm( &tcp_auto_reconnect_sw_timer );
	os_timer_arm( &tcp_auto_reconnect_sw_timer, ms, 1 );
}


void ICACHE_FLASH_ATTR tcp_auto_reconnect_sw_timer_cb( void *arg ) {

	os_timer_disarm( &tcp_auto_reconnect_sw_timer );

	int8_t res = tcp_connect_as_client( TCP_REMOTE_IP, TCP_REMOTE_PORT );

	if( res == 0 ) {
		os_timer_disarm( &tcp_auto_reconnect_sw_timer );
		return;
	}

    switch ( res ) {

        case ESPCONN_MEM:
            os_printf("Unable to connect to server - out of memory.\r\n");
            break;
        case ESPCONN_TIMEOUT:
            os_printf("Unable to connect to server - timeout.\r\n");
            break;
        case ESPCONN_ISCONN:
            os_printf("Unable to connect to server - already connected.\r\n");
            break;
        case ESPCONN_ARG:
            os_printf("Unable to connect to server - illegal argument.\r\n");
            break;
        default:
            os_printf("Unable to connect to server - unknown error - %d.\r\n", res);
            break;
    }

    tcp_reconnect_as_client( 3000 );
}

void ICACHE_FLASH_ATTR tcp_reconnect_cb( void *arg, int8_t err ) {

	struct espconn *conn = (struct espconn *)arg;

	char buf[128];
	os_sprintf( buf,"\r\n+TCP_RECONNECT:"IPSTR":%d\r\n", IP2STR(conn->proto.tcp->remote_ip), conn->proto.tcp->remote_port );
	os_printf( buf );

	tcp_reconnect_as_client( 5000 );	// reconnect
}
#endif
//----------------------------- SEKCJA RECONNECT end ------------------------------------------


#if TCP_MODE == 0
TCLI_CONN * ICACHE_FLASH_ATTR find_tcp_cli_connection( uint8 *remote_ip, int remote_port ) {

	  for ( uint8_t i = 0; i < MAX_TCP_CLIENTS; i++) {
	    if ( tcp_cli_pool[i] != NULL
	        && tcp_cli_pool[i]->remote_port == remote_port
	        && os_memcmp( tcp_cli_pool[i]->remote_ip, remote_ip, 4 ) == 0 ) {

	      return tcp_cli_pool[i];
	    }
	  }

	  return NULL;
}
#endif



void ICACHE_FLASH_ATTR tcp_disconnect_cb( void *arg ) {

	struct espconn *conn = (struct espconn *)arg;

	char buf[128];
	os_sprintf( buf,"\r\n+TCP_DISCONNECT:"IPSTR":%d\r\n", IP2STR(conn->proto.tcp->remote_ip), conn->proto.tcp->remote_port );
	os_printf( buf );

#if TCP_MODE == 1
	tcp_reconnect_as_client( 5000 );	// reconnect
#endif


#if TCP_MODE == 0

	TCLI_CONN * cp = find_tcp_cli_connection(
	                         conn->proto.tcp->remote_ip,
	                         conn->proto.tcp->remote_port);


	if (cp == NULL) {
		// to siê nie powinno zdarzyæ
		return;
	}

	int8_t i = cp->idx;
	os_free( tcp_cli_pool[i] );		// zwalniamy pamiêæ na stercie po kliencie
	tcp_cli_pool[i] = NULL;


#endif
}

void ICACHE_FLASH_ATTR tcp_receive_cb( void *arg, char *data, uint16_t len ) {

	struct espconn *conn = (struct espconn *)arg;

	char buf[128];
	os_sprintf( buf,"\r\n+TCP_FROM:"IPSTR":%d\r\n", IP2STR(conn->proto.tcp->remote_ip), conn->proto.tcp->remote_port );
	os_printf( buf );

	os_memset( buf, 0, sizeof(buf) );
	os_memcpy( buf, data, len );
	os_printf( "+TCP_DATA:" );
	os_printf( buf );
}


void ICACHE_FLASH_ATTR tcp_connect_cb( void *arg ) {

	struct espconn *conn = (struct espconn *)arg;

	char buf[128];
	os_sprintf( buf,"\r\n+TCP_CONNECT:"IPSTR":%d\r\n", IP2STR(conn->proto.tcp->remote_ip), conn->proto.tcp->remote_port );
	os_printf( buf );

#if TCP_MODE == 1
	espconn_regist_recvcb( conn, tcp_receive_cb );
#endif

#if TCP_MODE == 0

	uint8_t i;

	for( i = 0; i < MAX_TCP_CLIENTS; i++) {
		if( tcp_cli_pool[i] == NULL ) break;
	}

	if( i == MAX_TCP_CLIENTS ) {

		espconn_send( conn, tx, sizeof(tx) );
		os_printf("\r\n Wywo³ujê TASKA killera clientów\r\n");

		//espconn_disconnect( conn );	// tego nie mo¿na nigdy wywo³ywaæ w callbacku

		cli_reject_task_post( conn );

	}


	tcp_cli_pool[i] = os_zalloc( sizeof(TCLI_CONN) );	// rezerwujemy pamiêæ dla nowego klienta

	tcp_cli_pool[i]->idx = i;
	tcp_cli_pool[i]->conn = conn;
	tcp_cli_pool[i]->remote_port = conn->proto.tcp->remote_port;
	os_memcpy( tcp_cli_pool[i]->remote_ip, conn->proto.tcp->remote_ip, 4 );

	espconn_regist_recvcb( conn, tcp_receive_cb );
#endif
}


void ICACHE_FLASH_ATTR tcp_init( char * remote_ip, uint16_t remote_port, uint16_t local_port ) {

	ip_addr_t ip;
	ip.addr = ipaddr_addr( remote_ip );

	espconn_delete( &tcp );

	tcp.type = ESPCONN_TCP;
	tcp.state = ESPCONN_NONE;
	tcp.proto.tcp = &tcp_proto;

	espconn_regist_connectcb( &tcp, tcp_connect_cb );
    espconn_regist_disconcb( &tcp, tcp_disconnect_cb );

#if TCP_MODE == 1
	tcp.proto.tcp->remote_ip[0] = ip4_addr1( &ip );
	tcp.proto.tcp->remote_ip[1] = ip4_addr2( &ip );
	tcp.proto.tcp->remote_ip[2] = ip4_addr3( &ip );
	tcp.proto.tcp->remote_ip[3] = ip4_addr4( &ip );

	tcp.proto.tcp->remote_port = remote_port;

	tcp_proto.local_port = 0;		// bêdzie problem gdy dla TCP CLIENT bêdzie inna wartoœæ ni¿ 0

    espconn_regist_reconcb( &tcp, tcp_reconnect_cb );

	os_timer_disarm( &tcp_auto_reconnect_sw_timer );
	os_timer_setfn( &tcp_auto_reconnect_sw_timer, tcp_auto_reconnect_sw_timer_cb, NULL );
//	os_timer_arm( &tcp_auto_reconnect_sw_timer, 1000, 1 );

#endif

#if TCP_MODE == 0
	if( !local_port ) local_port = 5555;
	tcp_proto.local_port = local_port;

	espconn_accept( &tcp );
	espconn_regist_time( &tcp, 0, 0 );	// wy³¹czenie domyœlnego roz³¹czania klienta
	espconn_tcp_set_max_con( 100 );	// ograniczenie po³¹czeñ do serwera TCP za pomoc¹ API

#endif

}


/*.............. TCP end ..................................................*/



/*------------- WiFi START ------------------------------------------------*/
void ICACHE_FLASH_ATTR wifi_handle_event_cb( System_Event_t * evt ) {

	char buf[128] = {0};

	switch (evt->event) {

	case EVENT_STAMODE_CONNECTED:

		os_sprintf( buf, "\r\n+STA_CONNECTED_SSID: %s, channel %d\r\n",
		evt->event_info.connected.ssid,
		evt->event_info.connected.channel);
		os_printf( buf );

		// tu mo¿na w³¹czyæ diodê LED NIEBIESK¥

		break;

	case EVENT_STAMODE_DISCONNECTED:

		os_sprintf( buf,"\r\n+STA_DISCONNECTED_FROM_SSID:%s, reason %d\n",
		evt->event_info.disconnected.ssid,
		evt->event_info.disconnected.reason);
		os_printf(buf);

		// tu mo¿na WYL¥CZYÆ diodê LED NIEBIESK¥
		// tu mo¿na WYL¥CZYÆ diodê LED ZIELON¥

		break;

	case EVENT_STAMODE_AUTHMODE_CHANGE:

		os_printf("mode:	%d	->	%d\n",
		evt->event_info.auth_change.old_mode,
		evt->event_info.auth_change.new_mode);
		break;

	case EVENT_STAMODE_GOT_IP:

		os_sprintf(buf, "\r\n+STA_IP:" IPSTR "\r\n+STA_MASK:" IPSTR "\r\n+STA_GW:" IPSTR "\r\n",
		IP2STR(&evt->event_info.got_ip.ip),
		IP2STR(&evt->event_info.got_ip.mask),
		IP2STR(&evt->event_info.got_ip.gw));
		os_printf( buf );

		// tuaj wa¿ny moment

		// tu mo¿na w³¹czyæ diodê LED ZIELON¥

		os_timer_disarm( &gpio_sw_timer );
		os_timer_arm( &gpio_sw_timer, 1000, 1 );

//		espconn_connect( &tcp );

		break;

	case EVENT_SOFTAPMODE_STACONNECTED:

		os_sprintf(buf, "\r\n+AP_MAC_CONNECTED:" MACSTR ", AID = %d\r\n",
		MAC2STR(evt->event_info.sta_connected.mac),
		evt->event_info.sta_connected.aid);
		os_printf( buf );

		//

		break;

	case EVENT_SOFTAPMODE_STADISCONNECTED:

		os_sprintf(buf, "\r\n+AP_MAC_DISCONNECTED:" MACSTR ", AID = %d\n",
		MAC2STR(evt->event_info.sta_disconnected.mac),
		evt->event_info.sta_disconnected.aid);
		os_printf(buf);



		break;

	default:

		break;

	}
}

void ICACHE_FLASH_ATTR WIFI_Connect( uint8_t * ssid, uint8_t * pass ) {

	wifi_station_disconnect();

	wifi_set_event_handler_cb( wifi_handle_event_cb );

	struct station_config stationConf;
	os_memset( &stationConf, 0, sizeof( struct station_config ) );
	os_sprintf( stationConf.ssid, "%s", ssid );
	os_sprintf( stationConf.password, "%s", pass );
	wifi_station_set_config( &stationConf );

	// ustawienie statycznego adresu IP dla naszego ESP
	// odkomentuj poni¿sze linie i ustaw w³asny adres statyczny w #define
	wifi_station_dhcpc_stop();
	struct ip_info info;
	info.ip.addr = ipaddr_addr( STA_IP );
	info.gw.addr = ipaddr_addr( STA_GW );
	info.netmask.addr = ipaddr_addr( STA_MASK );
	wifi_set_ip_info( STATION_IF, &info );


	wifi_station_connect();
}

void ICACHE_FLASH_ATTR scan_done_cb( void *arg, STATUS status ) {

	int ret;
	uint8 ssid[128];
	char temp[128];
	uint8 res = 0;

	os_printf("\r\n+WIFI SCAN PROGRESS: wait...\r\n");

	if (status == OK) {

		struct bss_info *bss_link = (struct bss_info*) arg;

		while ( bss_link != NULL ) {

			os_memset( ssid, 0, sizeof(ssid) );
			os_memcpy( ssid, bss_link->ssid, os_strlen(bss_link->ssid) );

			os_sprintf( temp, "(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n",
					bss_link->authmode, ssid, bss_link->rssi,
					MAC2STR(bss_link->bssid), bss_link->channel);

			os_printf( temp );

			bss_link = bss_link->next.stqe_next;
		}

		os_printf("\r\n+WIFI SCAN END: OK\r\n");

		WIFI_Connect( "Atnel-2.4GHz", "55AA55AA55" );

	} else {
		os_printf("\r\n+SCAN FAIL: ERROR\r\n");
	}
}
/*............. WiFi end ..................................................*/




/*-------------- SYS START ------------------------------------------------*/

void ICACHE_FLASH_ATTR gpio_sw_timer_cb( void *arg ) {

	static uint8_t licznik;
	char buf[128];
	os_sprintf( buf, "Leci liczba: %d\r\n", licznik++ );

	espconn_send( &udp, buf, os_strlen(buf) );

#if TCP_MODE == 1
	/* tu dzia³amy jako klient TCP */
	static uint8_t flag;
	if( !flag ) {
//		flag = 1;
//		espconn_connect( &tcp );
		if( !tcp_connect_as_client( TCP_REMOTE_IP, TCP_REMOTE_PORT ) ) flag = 1;
	} else {
		espconn_send( &tcp, buf, os_strlen(buf) );
	}
#endif

#if TCP_MODE == 0
	// tutaj wysy³amy dane do klientów pod³¹czonych do serwera TCP
	for( uint8_t i=0; i<MAX_TCP_CLIENTS; i++ ) {
		if( tcp_cli_pool[i] != NULL )
			espconn_send( tcp_cli_pool[i]->conn, buf, os_strlen(buf) );
	}
#endif

}



void ICACHE_FLASH_ATTR on_user_init_completed( void ) {

	os_printf("\r\n+WIFI SCAN START:\r\n");
	wifi_station_scan( NULL, scan_done_cb );



	os_printf( "\r\nready - user_init COMPLETED !!!!!!!! \r\n" );
}



void ICACHE_FLASH_ATTR user_init( void ) {

	uart_div_modify( 0, UART_CLK_FREQ / 115200 );
	os_delay_us( 100 );

	system_init_done_cb( on_user_init_completed );


	wifi_set_opmode( STATIONAP_MODE );


	os_timer_disarm( &gpio_sw_timer );
	os_timer_setfn( &gpio_sw_timer, gpio_sw_timer_cb, NULL );
//	os_timer_arm( &gpio_sw_timer, 1000, 1 );

	udp_init( UDP_REMOTE_IP, UDP_REMOTE_PORT, 4444 );

#if TCP_MODE == 1
	tcp_init( TCP_REMOTE_IP, TCP_REMOTE_PORT, 0 );
#endif

#if TCP_MODE == 0
	tcp_init( 0, 0, 5555 );
#endif


#if TCP_MODE == 0
	task_init();
#endif

	os_printf( "\r\nready - user_init end\r\n" );

}
/*.............. SYS end ..................................................*/
