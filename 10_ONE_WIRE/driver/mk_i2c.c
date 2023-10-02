/*
 * mk_i2c.c
 *
 *  Created on: 15 sty 2022
 *      Author: Miros³aw Kardaœ
 */



#include "driver/i2c_master.h"

#include "driver/mk_i2c.h"


/* Funkcja MUSI BYÆ wywo³ywana osobno, wykasowano jej wywo³ywanie z i2c_master_gpio_init()
 *
 * Argumenty
 * #define BITRATE_100KHZ		0
 * #define BITRATE_200_400KHZ	1	// dla F_CPU=80MHz bitrate = 200 kHz, dla FCPU=160MHz bitrate = 400kHz
 * #define BITRATE_260_600KHZ	2	// dla F_CPU=80MHz bitrate = 260 kHz, dla FCPU=160MHz bitrate = 580kHz
 *
 */
int ICACHE_FLASH_ATTR i2c_init( uint8_t bitrate_mode ) {

	i2c_master_gpio_init();
	i2c_master_init( bitrate_mode );

	return -1;
}



void ICACHE_FLASH_ATTR i2c_start( void ) {
	i2c_master_start();
}

void ICACHE_FLASH_ATTR i2c_stop( void ) {
	i2c_master_stop();
}


uint8 ICACHE_FLASH_ATTR i2c_write( uint8 byte ) {
	i2c_master_writeByte( byte );
	return i2c_master_checkAck();
}


// sprawdŸ czy na magistrali i2c jest pod³¹czony scalak SLAVE o adresie SLA
uint8 ICACHE_FLASH_ATTR i2c_check_slave( uint8 SLA ) {
	i2c_start();
	uint8_t ack = i2c_write( SLA );
	i2c_stop();
	return ack;
}



uint8 ICACHE_FLASH_ATTR i2c_read( uint8 ack ) {

	uint8 byte = i2c_master_readByte();

	if( ack ) i2c_master_send_ack();
	else i2c_master_send_nack();

	return byte;
}


void ICACHE_FLASH_ATTR i2c_write_buf( uint8 SLA, uint8 adr, uint16 len, uint8 *buf ) {

	i2c_start();
	i2c_write(SLA);
	i2c_write(adr);
	while (len--) {
		if( !i2c_write(*buf++) ) break;
	}
	i2c_stop();
}


// sequential write into SLAVE
void ICACHE_FLASH_ATTR i2c_write_buf1( uint8 SLA, uint16 len, uint8 *buf ) {

	i2c_start();
	i2c_write(SLA);
	while (len--) {
		if( !i2c_write(*buf++) ) break;;
	}
	i2c_stop();
}



void ICACHE_FLASH_ATTR i2c_read_buf( uint8 SLA, uint8 adr, uint16 len, uint8 *buf ) {

	i2c_start();
	i2c_write(SLA);
	i2c_write(adr);
	i2c_start();
	i2c_write(SLA + 1);
	while (len--) {
		*buf++ = i2c_read( len ? ACK : NACK );
	}
	i2c_stop();
}


// sequential read from SLAVE
void ICACHE_FLASH_ATTR i2c_read_buf1( uint8 SLA, uint16 len, uint8 *buf ) {

	i2c_start();
	i2c_write(SLA);
	i2c_start();
	i2c_write(SLA + 1);
	while (len--) {
		*buf++ = i2c_read( len ? ACK : NACK );
	}
	i2c_stop();
}



void ICACHE_FLASH_ATTR i2c_send_byte( uint8 SLA, uint8 byte ) {

	i2c_start();
	i2c_write(SLA);
	i2c_write( byte );
	i2c_stop();

}

uint8 ICACHE_FLASH_ATTR i2c_read_byte( uint8 SLA ) {

	uint8_t res = 0;
	i2c_start();
	i2c_write(SLA+1);
	res = i2c_read( NACK );
	i2c_stop();

	return res;
}


void ICACHE_FLASH_ATTR i2c_send_word( uint8 SLA, uint16 data ) {

	i2c_start();
	i2c_write(SLA);
	i2c_write( data >> 8 );
	i2c_write( data >> 0 );
	i2c_stop();
}

uint16 ICACHE_FLASH_ATTR i2c_read_word( uint8 SLA ) {

	uint16_t res = 0;
	uint8_t * buf = (uint8_t*)&res;
	uint8_t len = 2;

	i2c_start();
	i2c_write(SLA+1);
	while (len--) *buf++ = i2c_read( len ? ACK : NACK );
	i2c_stop();

	return res;
}
