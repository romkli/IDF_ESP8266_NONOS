/*
 * mk_i2c.h
 *
 *  Created on: 15 sty 2022
 *      Author: Miros≥aw Kardaú
 */

#ifndef USER_MK_I2C_H_
#define USER_MK_I2C_H_


#define ACK 	1
#define NACK 	0


#define BITRATE_100KHZ		0
#define BITRATE_200_400KHZ	1	// dla F_CPU=80MHz bitrate = 200 kHz, dla FCPU=160MHz bitrate = 400kHz



/* Funkcja MUSI BY∆ wywo≥ywana osobno, wykasowano jej wywo≥ywanie z i2c_master_gpio_init()
 *
 * Argumenty
 * #define BITRATE_100KHZ		0
 * #define BITRATE_200_400KHZ	1	// dla F_CPU=80MHz bitrate = 200 kHz, dla FCPU=160MHz bitrate = 400kHz
 *
 */
extern int ICACHE_FLASH_ATTR i2c_init( uint8_t bitrate_mode );

extern void ICACHE_FLASH_ATTR i2c_start( void );
extern void ICACHE_FLASH_ATTR i2c_stop( void );

extern uint8_t ICACHE_FLASH_ATTR i2c_write( uint8 byte );
extern uint8_t ICACHE_FLASH_ATTR i2c_read( uint8 ack );


// sprawdü czy na magistrali i2c jest pod≥πczony scalak SLAVE o adresie SLA
extern uint8 ICACHE_FLASH_ATTR i2c_check_slave( uint8 SLA );


extern void ICACHE_FLASH_ATTR i2c_write_buf( uint8 SLA, uint8 adr, uint16 len, uint8 *buf );
// sequential write into SLAVE
extern void ICACHE_FLASH_ATTR i2c_write_buf1( uint8 SLA, uint16 len, uint8 *buf );

extern void ICACHE_FLASH_ATTR i2c_read_buf( uint8 SLA, uint8 adr, uint16 len, uint8 *buf );
// sequential read from SLAVE
extern void ICACHE_FLASH_ATTR i2c_read_buf1( uint8 SLA, uint16 len, uint8 *buf );



extern void ICACHE_FLASH_ATTR i2c_send_byte( uint8 SLA, uint8 byte );
extern uint8 ICACHE_FLASH_ATTR i2c_read_byte( uint8 SLA );


extern void ICACHE_FLASH_ATTR i2c_send_word( uint8 SLA, uint16 data );
extern uint16 ICACHE_FLASH_ATTR i2c_read_word( uint8 SLA );


#endif /* USER_MK_I2C_H_ */
