/*
 * mk_uart.c
 *
 *  Created on: 7 lut 2022
 *      Author: Miros³aw Kardaœ
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "osapi.h"
#include "at_custom.h"
#include "user_interface.h"

#include "espconn.h"

#include "driver/uart.h"

#include "MODULES/mk_uart.h"
#include "MODULES/mk_udp.h"






void ICACHE_FLASH_ATTR  mk_uart0_init( void ) {

	int baud, databits, stopbits, parity, flow;

	// <baudrate>,<databits>,<stopbits>,<parity>,<flow control>

    //	typedef enum {
    //	    FIVE_BITS  = 0x0,
    //	    SIX_BITS   = 0x1,
    //	    SEVEN_BITS = 0x2,
    //	    EIGHT_BITS = 0x3
    //	} UartBitsNum4Char;
    //
    //	typedef enum {
    //	    ONE_STOP_BIT       = 0x1,
    //	    ONE_HALF_STOP_BIT  = 0x2,
    //	    TWO_STOP_BIT       = 0x3
    //	} UartStopBitsNum;
    //
    //	typedef enum {
    //	    NONE_BITS = 0x2,
    //	    ODD_BITS   = 1,
    //	    EVEN_BITS = 0
    //	} UartParityMode;
	//
    //	typedef enum {
    //	    USART_HardwareFlowControl_None    = 0x0,
    //	    USART_HardwareFlowControl_RTS     = 0x1,
    //	    USART_HardwareFlowControl_CTS     = 0x2,
    //	    USART_HardwareFlowControl_CTS_RTS = 0x3
    //	} UART_HwFlowCtrl;

	baud 		= MK_UART_BAUD;
	databits	= MK_UART_DATABITS;
	stopbits	= MK_UART_STOPBITS;
	parity		= MK_UART_PARITY;
	flow		= MK_UART_FLOW;

    switch( databits ) {
    	case 8 : { databits = EIGHT_BITS; break; }	// EIGHT_BITS
    	case 7 : { databits = SEVEN_BITS; break; }	// SEVEN_BITS
    	case 6 : { databits = SIX_BITS; break; }	// SIX_BITS
    	case 5 : { databits = FIVE_BITS; break; }	// FIVE_BITS
    	default: {
    		databits = EIGHT_BITS;					// EIGHT_BITS
//    		at_port_print_irom_str("\r\nBad DataBits value, default value will be used - 8 bits bit\r\n");
    	}
    }

    switch( stopbits ) {
    	case 1 : { stopbits = ONE_STOP_BIT; break; }		// ONE_STOP_BIT
    	case 2 : { stopbits = ONE_HALF_STOP_BIT; break; }	// ONE_HALF_STOP_BIT
    	case 3 : { stopbits = TWO_STOP_BIT; break; }		// TWO_STOP_BIT
    	default: {
    		stopbits = ONE_STOP_BIT;					// EIGHT_BITS
//    		at_port_print_irom_str("\r\nBad StopBits value, default value will be used - 1 stop bits bit\r\n");
    	}
    }

    switch( parity ) {
    	case 0 : { parity = 2; break; }	// NONE
    	case 1 : { parity = 1; break; }	// ODD
    	case 2 : { parity = 0; break; }	// EVEN
    	default: {
//    		at_port_print_irom_str("\r\nBad Parity value, default value will be used - NONE bit\r\n");
    		parity = 2;			// NONE
    	}
    }

    switch( flow ) {
    	case 0 : { flow = 0; break; }	// NONE
    	case 1 : { flow = 1; break; }	// RTS
    	case 2 : { flow = 2; break; }	// CTS
    	case 3 : { flow = 3; break; }	// CTS & RTS
    	default: {
    		flow = 0;					// NONE
//    		at_port_print_irom_str("\r\nBad FlowControl value, default value will be used - NONE\r\n");
    	}
    }

	UART_SetBaudrate( 0, baud );
	UART_SetWordLength( 0, databits );
	UART_SetStopBits( 0, stopbits );
	UART_SetParity( 0, parity );
	UART_SetFlowCtrl( 0, flow, 96 );

}


static os_timer_t transparent_off_timer;
uint8 transparent_mode_enabled;





void ICACHE_FLASH_ATTR transparent_off_timer_cb( void *arg ) {

	os_timer_disarm( &transparent_off_timer );
	change_transparent_mode( 0 );
	at_port_print_irom_str("\r\n+TRANSPARENT_MODE_OFF:\r\n");

	transparent_mode_enabled = 0;

}


void ICACHE_FLASH_ATTR transparent_init( void ) {
	// timer potrzebny na wy³¹czenie trybu transparent
	os_timer_disarm( &transparent_off_timer );
	os_timer_setfn( &transparent_off_timer, transparent_off_timer_cb, NULL );
}



void ICACHE_FLASH_ATTR change_transparent_mode( uint8 onoff ) {

	if( onoff ) {
		at_register_uart_rx_intr( user_uart_rx_intr );
		transparent_mode_enabled = 1;
	}
	else at_register_uart_rx_intr( NULL );
}


static const char tm_break[] ICACHE_RODATA_ATTR  = {"+++"};

void user_uart_rx_intr( uint8 * data, int32 len ) {

	if( len == 3 ) {
		if( !os_strcmp( data, tm_break ) ) {
			os_timer_disarm( &transparent_off_timer );
			os_timer_arm( &transparent_off_timer, 150, 0 );
			return;
		}
	}

	espconn_send( &udp, data, len );

}


void ICACHE_FLASH_ATTR uart0_send_buf( char * data, uint16_t len ) {

	while( len-- ) {
		uart_tx_one_char( 0, *data++ );
	}
}
