/*
 * mk_uart.h
 *
 *  Created on: 7 lut 2022
 *      Author: Miros³aw Kardaœ
 */

#ifndef INCLUDE_MODULES_MK_UART_H_
#define INCLUDE_MODULES_MK_UART_H_

/*------ moja konfiguracja UART'a ----------------------------*/
#define MK_UART_BAUD			 115200
#define MK_UART_DATABITS		8
#define MK_UART_STOPBITS		1
#define MK_UART_PARITY			0
#define MK_UART_FLOW			0
/*-------------------------------------------------------------*/


extern uint8 transparent_mode_enabled;








extern void ICACHE_FLASH_ATTR  mk_uart0_init( void );

extern void ICACHE_FLASH_ATTR change_transparent_mode( uint8 onoff );
extern void ICACHE_FLASH_ATTR transparent_init( void );

extern void user_uart_rx_intr( uint8 * data, int32 len );

extern void ICACHE_FLASH_ATTR uart0_send_buf( char * data, uint16_t len );




#endif /* INCLUDE_MODULES_MK_UART_H_ */
