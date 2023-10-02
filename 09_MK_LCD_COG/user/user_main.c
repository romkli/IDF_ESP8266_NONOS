/*-----------------------------------------------------------------------------------------------------------
// *** Obs≥uga wyúwietlaczy alfanumerycznych zgodnych z HD44780 ***
 *
 * ..............................................................................
 * . UWAGA! wersja dla ESP nie obs≥uguje w ogÛle wyúwietlaczy w trybie 4-bit	.
 * .        TYLKO I WY£•CZNIE po magistrali I2C !!!								.
 * .	BIBLIOTEKA przeznaczona tylko dla ESP8266 w trybie NonOS SDK			.
 *  .............................................................................
 *
// - Sterowanie: tryb 4-bitowy
// - Dowolne przypisanie kaødego sygna≥u sterujπcego do dowolnego pinu mikrokontrolera
// - Praca z pinem RW pod≥πczonym do GND lub do mikrokontrolera (sprawdzanie BusyFLAG - szybkie operacje LCD)
//
//	Biblioteka MK_LCD ver: 3.0	(with COG Chip On Glass ST7032 I2C)
//	nowoúci:
//	- nowa szybka funkcja: lcd_cls(X) z argumentami X=0 - kasuje ca≥y wyúwietlacz, X=n - kasuje liniÍ nr n wyúwietlacza
//    n - linie numerowane sπ od 1 do 4 (a nie od 0 do 3). Po skasowaniu kursor ustawia siÍ na poczπtku linii n.
//  - poprawione makra aby nie k≥Ûci≥y siÍ z innymi kodami z Atnela
// 	- przyk≥ady dla obs≥ugi arduinowego konwertera I2C - LCD z PCF8584(A)
//  - Biblioteka dzia≥a przy taktowaniu od 1MHz do 20MHz
//  - Biblioteka dzia≥a na DOWOLNYCH mikrokontrolerach AVR
//  - 10 duøych fontÛw dla cyfr i znaku minus, do wyúwietlania wiÍkszych liczb
//  - nowe funkcje: lcd_big_dig(), lcd_big_dig1(), lcd_int_big(), lcd_load_big_fonts()
//
//  - Do pracy z konwerterem (arduino) I2C-LCD wymagana jest dodatkowo biblioteka MK_I2C
//    ktÛrπ moøna zakupiÊ w sklepie: https://sklep.atnel.pl/pl/searchquery/i2c/1/default/5?url=i2c
//
//  - Do pracy z konwerterem (arduino) I2C-LCD wymagana jest dodatkowo biblioteka MK_I2C
//    ktÛrπ moøna zakupiÊ w sklepie: https://sklep.atnel.pl/pl/searchquery/i2c/1/default/5?url=i2c
//
// Pliki 			: mk_lcd44780.c , mk_lcd44780.h
// Mikrokontrolery 	: ESP8266 NonOS SDK
// Kompilator 		: xtensa-gcc
// èrÛd≥o 			: http://www.atnel.pl
// Data 			: 2021-05-29
// Autor 			: Miros≥aw Kardaú
//-----------------------------------------------------------------------------------------------------------
 */
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "driver/mk_i2c.h"
#include "MK_LCD/mk_lcd44780.h"



static os_timer_t rtc_sw_timer;

void ICACHE_FLASH_ATTR rtc_sw_timer_cb( void *arg ) {


	static uint8 bigf=1;

	uint8_t x0 = 0;
	uint8_t x1 = 4;
	uint8_t x2 = 8;
	uint8_t x3 = 12;
	uint8_t xd = 7;

	if( bigf < 3 ) {
		x0 = 0;
		x1 = 4;
		x2 = 8;
		x3 = 12;
		xd = 7;
	} else {
		x0 = 0;
		x1 = 3;
		x2 = 7;
		x3 = 10;
		xd = 6;
	}

	set_current_big_font(bigf);

	lcd_cls(0);



	lcd_big_dig(bigf,0,x0,0);
	lcd_big_dig(bigf,0,x1,1);

	lcd_locate(0,xd); lcd_char('.');
	lcd_locate(0+1,xd); lcd_char('.');

	lcd_big_dig(bigf,0,x2,2);
	lcd_big_dig(bigf,0,x3,3);

	if( bigf > 2 ) {
		lcd_locate(0,13); lcd_str(":12");
	}

	bigf++;
	if(bigf>10) bigf=1;




}


const uint8_t kr[] ICACHE_RODATA_ATTR = {32,32,4,32,32,4,32,32};

// char txt1[]  = "COG Display  OK!";

void ICACHE_FLASH_ATTR on_user_init_completed( void ) {

	i2c_init( aBITRATE_100KHZ );

	lcd_init();
	lcd_cls(0);

	lcd_defchar( 0, kr );

	lcd_cls(0);

	lcd_str( "MK LCD 3.0 " "\x80" " ESP" );
	lcd_locate(1, 0);
	lcd_str( "COG Display  OK!" );




	os_timer_disarm( &rtc_sw_timer );
	os_timer_setfn( &rtc_sw_timer, rtc_sw_timer_cb, NULL );
	os_timer_arm( &rtc_sw_timer, 2000, 1 );

}



void ICACHE_FLASH_ATTR user_init(void) {

	uart_div_modify( 0, UART_CLK_FREQ / 115200 );
	os_delay_us( 100 );

	system_init_done_cb( on_user_init_completed );

	wifi_set_opmode( NULL_MODE );



	os_printf( "\r\nSys Init:\r\n" );

}

