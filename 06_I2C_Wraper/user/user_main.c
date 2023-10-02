
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "driver/i2c_master.h"

#include "mk_i2c.h"


#define DS1337_ADDR 		0xD0
#define PCF8583_ADDR 		0xA2
#define EEPROM_24C04_ADDR 	0xA8


static os_timer_t rtc_sw_timer;

void ICACHE_FLASH_ATTR rtc_sw_timer_cb( void *arg ) {

	char buf[128];
	uint8_t ack;
	uint8 ss=0,mm=0,hh=0;
	uint8 czas[3];

//	i2c_master_start();
//	i2c_master_writeByte( PCF8583_ADDR );
//	ack = i2c_master_checkAck();
//
//	if( ack ) {
//		i2c_master_writeByte( 2 );
//		i2c_master_checkAck();
//		i2c_master_start();
//		i2c_master_writeByte( PCF8583_ADDR + 1 );
//		i2c_master_checkAck();
//
//		ss = i2c_master_readByte();
//		ss = ((ss>>4)*10) + (ss & 0x0F);
//		i2c_master_send_ack();
//
//		mm = i2c_master_readByte();
//		mm = ((mm>>4)*10) + (mm & 0x0F);
//		i2c_master_send_ack();
//
//		hh = i2c_master_readByte();
//		hh = ((hh>>4)*10) + (hh & 0x0F);
//		i2c_master_send_nack();
//
//		i2c_master_stop();
//
//		os_sprintf( buf, "godzina: %02d:%02d:%02d\r\n", hh, mm, ss );
//		os_printf( buf );
//		return;
//	} else {
//		os_printf( "\r\n***** PCF8583 not visible - timer STOP *****\r\n" );
//		os_timer_disarm( &rtc_sw_timer );
//	}


	if( i2c_check_slave( PCF8583_ADDR ) ) {
		i2c_read_buf( PCF8583_ADDR, 2, 3, czas );

		ss = czas[0];
		ss = ((ss>>4)*10) + (ss & 0x0F);
		mm = czas[1];
		mm = ((mm>>4)*10) + (mm & 0x0F);
		hh = czas[2];
		hh = ((hh>>4)*10) + (hh & 0x0F);

		os_sprintf( buf, "godzina: %02d:%02d:%02d\r\n", hh, mm, ss );
		os_printf( buf );

	} else {
		os_printf( "\r\n***** PCF8583 not visible - timer STOP *****\r\n" );
		os_timer_disarm( &rtc_sw_timer );
	}


}


void ICACHE_FLASH_ATTR on_user_init_completed( void ) {

	os_printf( "\r\n***** I2C INIT *****\r\n" );

	//	if( system_update_cpu_freq( 160 ) ) {
	//		os_printf( "\r\n***** F_CPU = 160 MHz *****\r\n" );
	//	}

	i2c_init( BITRATE_100KHZ );


	os_delay_us( 1000 );

	os_printf( "\r\nChecking RTC: PCF8583\r\n" );
	i2c_start();
	if( i2c_check_slave( PCF8583_ADDR ) ) os_printf( "ACK OK\r\n" );
	else os_printf( "ACK missing!\r\n" );
	i2c_stop();


	os_printf( "\r\nChecking RTC: RTC_DS1337\r\n" );
	if( i2c_check_slave( DS1337_ADDR ) ) os_printf( "ACK OK\r\n" );
	else os_printf( "ACK missing!\r\n" );


	os_printf( "\r\nChecking EEPROM: 24c04\r\n" );
	if( i2c_check_slave( EEPROM_24C04_ADDR ) ) os_printf( "ACK OK\r\n" );
	else os_printf( "ACK missing!\r\n" );





	/* ustawienie godziny */
//	i2c_master_start();
//	i2c_master_writeByte( PCF8583_ADDR );
//	uint8 ack = i2c_master_checkAck();
//
//	if( ack ) {
//		i2c_master_writeByte( 2 );
//		i2c_master_checkAck();
//
//		/********** ustawiamy godzinê na 23:59:49 ************/
//		i2c_master_writeByte( (4<<4) + 9 );		// 49
//		i2c_master_checkAck();
//		i2c_master_writeByte( (5<<4) + 9 );		// 59
//		i2c_master_checkAck();
//		i2c_master_writeByte( (2<<4) + 3 );		// 23
//		i2c_master_checkAck();
//	}
//	i2c_master_stop();



	if( i2c_check_slave( PCF8583_ADDR ) ) {
		/********** ustawiamy godzinê na 23:59:49 w BCD ************/
		uint8 buf[3];
		buf[0] = (4<<4) + 9;	// ss
		buf[1] = (5<<4) + 9;	// mm
		buf[2] = (2<<4) + 3;	// hh
		i2c_write_buf( PCF8583_ADDR , 2, 3, buf );
	}





	os_timer_disarm( &rtc_sw_timer );
	os_timer_setfn( &rtc_sw_timer, rtc_sw_timer_cb, NULL );
	os_timer_arm( &rtc_sw_timer, 1000, 1 );


}


void ICACHE_FLASH_ATTR user_init(void) {

	uart_div_modify( 0, UART_CLK_FREQ / 115200 );
	os_delay_us( 100 );

	system_init_done_cb( on_user_init_completed );

	wifi_set_opmode( NULL_MODE );



	os_printf( "\r\nSys Init:\r\n" );
}

