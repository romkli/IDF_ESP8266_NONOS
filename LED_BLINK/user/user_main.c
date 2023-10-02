/*
 * user_main.c
 *
 *  Created on: 26 lip 2021
 *      Author: Mirosław Kardaś
 *
 * 		FLASH 4 MBytes
 * 		COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=DIO SPI_SIZE_MAP=6
 *
 * 		FLASH 2 MBytes
 * 		COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=DIO SPI_SIZE_MAP=5
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"


#define LED_IO_MUX     PERIPHS_IO_MUX_GPIO2_U
#define LED_IO_NUM     2
#define LEDIO_FUNC     FUNC_GPIO2


static os_timer_t gpio_sw_timer;

void ICACHE_FLASH_ATTR gpio_sw_timer_cb( void *arg ) {

	static uint8 sw;

	GPIO_OUTPUT_SET( LED_IO_NUM, sw );

	sw ^= 1;

}



void ICACHE_FLASH_ATTR user_init(void) {


	PIN_FUNC_SELECT( LED_IO_MUX, LEDIO_FUNC );
	gpio_output_set( 0, LED_IO_NUM, LED_IO_NUM, 0 );


	os_timer_disarm( &gpio_sw_timer );
	os_timer_setfn( &gpio_sw_timer, gpio_sw_timer_cb, NULL );
	os_timer_arm( &gpio_sw_timer, 100, 1 );


}

