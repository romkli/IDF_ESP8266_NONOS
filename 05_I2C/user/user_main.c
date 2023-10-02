
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "driver/i2c_master.h"

#include "driver/gpio16.h"

#define DS1337_ADDR 		0xD0
#define PCF8583_ADDR 		0xA2
#define EEPROM_24C04_ADDR 	0xA8


static os_timer_t rtc_sw_timer;

void ICACHE_FLASH_ATTR rtc_sw_timer_cb( void *arg ) {

	char buf[128];
	uint8_t ack;
	uint8 ss=0,mm=0,hh=0;

	i2c_master_start();
	i2c_master_writeByte( PCF8583_ADDR );
	ack = i2c_master_checkAck();

	if( ack ) {
		i2c_master_writeByte( 2 );
		i2c_master_checkAck();
		i2c_master_start();
		i2c_master_writeByte( PCF8583_ADDR + 1 );
		i2c_master_checkAck();

		ss = i2c_master_readByte();
		ss = ((ss>>4)*10) + (ss & 0x0F);
		i2c_master_send_ack();

		mm = i2c_master_readByte();
		mm = ((mm>>4)*10) + (mm & 0x0F);
		i2c_master_send_ack();

		hh = i2c_master_readByte();
		hh = ((hh>>4)*10) + (hh & 0x0F);
		i2c_master_send_nack();

		i2c_master_stop();

		os_sprintf( buf, "godzina: %02d:%02d:%02d\r\n", hh, mm, ss );
		os_printf( buf );
		return;
	} else {
		os_printf( "\r\n***** PCF8583 not visible - timer STOP *****\r\n" );
		os_timer_disarm( &rtc_sw_timer );
	}


}


void ICACHE_FLASH_ATTR on_user_init_completed( void ) {

	os_printf( "\r\n***** I2C INIT *****\r\n" );
	i2c_master_gpio_init();

//	if( system_update_cpu_freq( 160 ) ) {
//		os_printf( "\r\n***** F_CPU = 160 MHz *****\r\n" );
//	}
	i2c_master_init( BITRATE_200_400KHZ );

	os_delay_us( 1000 );

	os_printf( "\r\nChecking RTC: PCF8583\r\n" );
	i2c_master_start();
	i2c_master_writeByte( PCF8583_ADDR );
	if( i2c_master_checkAck() ) os_printf( "ACK OK\r\n" );
	else os_printf( "ACK missing!\r\n" );
	i2c_master_stop();

	os_printf( "\r\nChecking RTC: RTC_DS1337\r\n" );
	i2c_master_start();
	i2c_master_writeByte( DS1337_ADDR );
	if( i2c_master_checkAck() ) os_printf( "ACK OK\r\n" );
	else os_printf( "ACK missing!\r\n" );
	i2c_master_stop();

	os_printf( "\r\nChecking EEPROM: 24c04\r\n" );
	i2c_master_start();
	i2c_master_writeByte( EEPROM_24C04_ADDR );
	if( i2c_master_checkAck() ) os_printf( "ACK OK\r\n" );
	else os_printf( "ACK missing!\r\n" );
	i2c_master_stop();





	/* ustawienie godziny */
	i2c_master_start();
	i2c_master_writeByte( PCF8583_ADDR );
	uint8 ack = i2c_master_checkAck();

	if( ack ) {
		i2c_master_writeByte( 2 );
		i2c_master_checkAck();

		/********** ustawiamy godzinê na 23:59:49 ************/
		i2c_master_writeByte( (4<<4) + 9 );		// 49
		i2c_master_checkAck();
		i2c_master_writeByte( (5<<4) + 9 );		// 59
		i2c_master_checkAck();
		i2c_master_writeByte( (2<<4) + 3 );		// 23
		i2c_master_checkAck();
	}
	i2c_master_stop();



	os_timer_disarm( &rtc_sw_timer );
	os_timer_setfn( &rtc_sw_timer, rtc_sw_timer_cb, NULL );
	os_timer_arm( &rtc_sw_timer, 1000, 1 );



	/* obiecane zabawy GPIO16 */
	for( int i=0; i<10; i++ ) {
//		system_soft_wdt_feed();
		gpio16_output_set(1);
		os_delay_us(60000);
		os_delay_us(30000);
		gpio16_output_set(0);
		os_delay_us(60000);
		os_delay_us(30000);
	}

}


void ICACHE_FLASH_ATTR user_init(void) {

	gpio16_output_conf();	// GPIO16 as output
	gpio16_output_set( 1 );	// LED OFF

	uart_div_modify( 0, UART_CLK_FREQ / 115200 );
	os_delay_us( 100 );

	system_init_done_cb( on_user_init_completed );

	wifi_set_opmode( NULL_MODE );



	os_printf( "\r\nSys Init:\r\n" );
}

