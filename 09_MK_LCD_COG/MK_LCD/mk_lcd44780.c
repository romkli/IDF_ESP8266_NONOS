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


#include <stdlib.h>
#include <string.h>


#include "MK_LCD/mk_lcd44780.h"

#if USE_ST7032 == 1

static uint8_t contrast = 32;		// 63 max

#endif

static uint8_t display;


static int ay, ax;

#if USE_BIG_DIGITS > 0
	static uint8_t current_big_font = USE_BIG_DIGITS;
#endif


// makrodefinicje operacji na sygna≥ach sterujπcych RS,RW oraz E

#if USE_ST7032 == 0
	#if !USE_I2C
		#define SET_RS 	MKLCD_PORT(LCD_RSPORT) |= (1<<LCD_RS) 				// stan wysoki na linii RS
		#define CLR_RS 	MKLCD_PORT(LCD_RSPORT) &= ~(1<<LCD_RS) 				// stan niski na linii RS

		#define SET_RW 	MKLCD_PORT(LCD_RWPORT) |= (1<<LCD_RW) 				// stan wysoki na RW - odczyt z LCD
		#define CLR_RW 	MKLCD_PORT(LCD_RWPORT) &= ~(1<<LCD_RW) 				// stan niski na RW - zapis do LCD

		#define SET_E 	MKLCD_PORT(LCD_EPORT) |= (1<<LCD_E) 					// stan wysoki na linii E
		#define CLR_E 	MKLCD_PORT(LCD_EPORT) &= ~(1<<LCD_E) 				// stan niski na linii E
	#else
		#define SET_RS 	mpxLCD |= (1<<LCD_RS); 	SEND_I2C	// stan wysoki na linii RS
		#define CLR_RS 	mpxLCD &= ~(1<<LCD_RS); SEND_I2C	// stan niski na linii RS

		#define SET_RW 	mpxLCD |= (1<<LCD_RW); 	SEND_I2C	// stan wysoki na RW - odczyt z LCD
		#define CLR_RW 	mpxLCD &= ~(1<<LCD_RW); SEND_I2C	// stan niski na RW - zapis do LCD

		#define SET_E 	mpxLCD |= (1<<LCD_E); 	SEND_I2C	// stan wysoki na linii E
		#define CLR_E 	mpxLCD &= ~(1<<LCD_E); 	SEND_I2C	// stan niski na linii E
	#endif

#endif




#if USE_RW && USE_ST7032 == 0
uint8_t ICACHE_FLASH_ATTR check_BF(void);			// deklaracja funkcji wewnÍtrznej
#endif


#if USE_I2C == 1 && USE_ST7032 == 0
//Deklaracja zmiennej na potrzeby obs≥ugi multipleksera
static uint8_t	mpxLCD;
#endif

//********************* FUNKCJE WEWN TRZNE *********************

//----------------------------------------------------------------------------------------
//
//		 Ustawienie wszystkich 4 linii danych jako WYjúcia
//
//----------------------------------------------------------------------------------------
#if USE_ST7032 == 0
static inline void data_dir_out(void) {


	#if !USE_I2C
		MKLCD_DDR(LCD_D7PORT)	|= (1<<LCD_D7);
		MKLCD_DDR(LCD_D6PORT)	|= (1<<LCD_D6);
		MKLCD_DDR(LCD_D5PORT)	|= (1<<LCD_D5);
		MKLCD_DDR(LCD_D4PORT)	|= (1<<LCD_D4);
	#else
		//Zerowanie zmiennych danych (D4..D7)
		mpxLCD	&= ~(1<<LCD_D7);
		mpxLCD	&= ~(1<<LCD_D6);
		mpxLCD	&= ~(1<<LCD_D5);
		mpxLCD	&= ~(1<<LCD_D4);
		SEND_I2C;
	#endif

}
#endif

//----------------------------------------------------------------------------------------
//
//		 Ustawienie wszystkich 4 linii danych jako WEjúcia
//
//----------------------------------------------------------------------------------------
#if USE_RW && USE_ST7032 == 0
static inline void data_dir_in(void) {

	#if !USE_I2C
		MKLCD_DDR(LCD_D7PORT)	&= ~(1<<LCD_D7);
		MKLCD_DDR(LCD_D6PORT)	&= ~(1<<LCD_D6);
		MKLCD_DDR(LCD_D5PORT)	&= ~(1<<LCD_D5);
		MKLCD_DDR(LCD_D4PORT)	&= ~(1<<LCD_D4);
	#else
		// PCF8574 wymaga ustawienia stanu wysokiego dla wejúÊ
		mpxLCD |= (1<<LCD_D7);
		mpxLCD |= (1<<LCD_D6);
		mpxLCD |= (1<<LCD_D5);
		mpxLCD |= (1<<LCD_D4);
		SEND_I2C;
    #endif

}
#endif

//----------------------------------------------------------------------------------------
//
//		 Wys≥anie po≥Ûwki bajtu do LCD (D4..D7)
//
//----------------------------------------------------------------------------------------
#if USE_ST7032 == 0
static inline void lcd_sendHalf(uint8_t data) {


#if USE_ST7032 == 0
	#if !USE_I2C

	if (data&(1<<0)) MKLCD_PORT(LCD_D4PORT) |= (1<<LCD_D4); else MKLCD_PORT(LCD_D4PORT) &= ~(1<<LCD_D4);
	if (data&(1<<1)) MKLCD_PORT(LCD_D5PORT) |= (1<<LCD_D5); else MKLCD_PORT(LCD_D5PORT) &= ~(1<<LCD_D5);
	if (data&(1<<2)) MKLCD_PORT(LCD_D6PORT) |= (1<<LCD_D6); else MKLCD_PORT(LCD_D6PORT) &= ~(1<<LCD_D6);
	if (data&(1<<3)) MKLCD_PORT(LCD_D7PORT) |= (1<<LCD_D7); else MKLCD_PORT(LCD_D7PORT) &= ~(1<<LCD_D7);

	#else
		if (data&(1<<0)) mpxLCD |= (1<<LCD_D4); else mpxLCD &= ~(1<<LCD_D4);
		if (data&(1<<1)) mpxLCD |= (1<<LCD_D5); else mpxLCD &= ~(1<<LCD_D5);
		if (data&(1<<2)) mpxLCD |= (1<<LCD_D6); else mpxLCD &= ~(1<<LCD_D6);
		if (data&(1<<3)) mpxLCD |= (1<<LCD_D7); else mpxLCD &= ~(1<<LCD_D7);
		SEND_I2C;
	#endif
#endif
}
#endif

#if USE_RW == 1 && USE_ST7032 == 0
//----------------------------------------------------------------------------------------
//
//		 Odczyt po≥Ûwki bajtu z LCD (D4..D7)
//
//----------------------------------------------------------------------------------------

static inline uint8_t lcd_readHalf(void) {


	uint8_t result=0;



	#if USE_I2C

		uint8_t res=0;

		res = RECEIVE_I2C;
		// WAØNA ZMIANA - by mirekk36
		if(res&(1<<LCD_D4)) result |= (1<<0);
		if(res&(1<<LCD_D5)) result |= (1<<1);
		if(res&(1<<LCD_D6)) result |= (1<<2);
		if(res&(1<<LCD_D7)) result |= (1<<3);

	#else

		if(MKLCD_PIN(LCD_D4PORT)&(1<<LCD_D4)) result |= (1<<0);
		if(MKLCD_PIN(LCD_D5PORT)&(1<<LCD_D5)) result |= (1<<1);
		if(MKLCD_PIN(LCD_D6PORT)&(1<<LCD_D6)) result |= (1<<2);
		if(MKLCD_PIN(LCD_D7PORT)&(1<<LCD_D7)) result |= (1<<3);

	#endif


	return result;
}
#endif

//----------------------------------------------------------------------------------------
//
//		 Zapis bajtu do wyúwietlacza LCD
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR _lcd_write_byte(unsigned char _data) {

#if USE_ST7032 == 0
	// Ustawienie pinÛw portu LCD D4..D7 jako wyjúcia
	data_dir_out();

	#if USE_RW == 1
		CLR_RW;
	#endif

	SET_E;
	lcd_sendHalf(_data >> 4);			// wys≥anie starszej czÍúci bajtu danych D7..D4
	CLR_E;

	SET_E;
	lcd_sendHalf(_data);				// wys≥anie m≥odszej czÍúci bajtu danych D3..D0
	CLR_E;

	#if USE_RW == 1

		while( (check_BF() & (1<<7)) );

	#else
		_delay_us(120);
	#endif

#else



#endif

}

#if USE_RW == 1 && USE_ST7032 == 0
//----------------------------------------------------------------------------------------
//
//		 Odczyt bajtu z wyúwietlacza LCD
//
//----------------------------------------------------------------------------------------
uint8_t ICACHE_FLASH_ATTR _lcd_read_byte(void) {

	uint8_t result=0;

	data_dir_in();

	SET_RW;

	SET_E;

	result = (lcd_readHalf() << 4);		// odczyt starszej czÍúci bajtu z LCD D7..D4

	CLR_E;

	SET_E;
	result |= lcd_readHalf();			// odczyt m≥odszej czÍúci bajtu z LCD D3..D0
	CLR_E;

	return result;
}
#endif


#if USE_RW == 1 && USE_ST7032 == 0
//----------------------------------------------------------------------------------------
//
//		 Sprawdzenie stanu Busy Flag (ZajÍtoúci wyúwietlacza)
//
//----------------------------------------------------------------------------------------
uint8_t ICACHE_FLASH_ATTR check_BF(void) {

	CLR_RS;

	return _lcd_read_byte();
}
#endif


//----------------------------------------------------------------------------------------
//
//		 Zapis komendy do wyúwietlacza LCD
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_write_cmd(uint8_t cmd) {

#if USE_ST7032 == 0
	CLR_RS;

	_lcd_write_byte(cmd);

#endif


#if USE_I2C == 1 && USE_ST7032 == 1
	i2c_start();
	i2c_write(ST7032_ADDR);
	i2c_write(CTRL_BYTE);
	i2c_write(cmd);
	i2c_stop();
	os_delay_us(WRITE_DELAY_US);

#endif

}

//----------------------------------------------------------------------------------------
//
//		 Zapis danych do wyúwietlacza LCD
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_write_data(uint8_t data) {

#if USE_ST7032 == 0
	SET_RS;
	_lcd_write_byte(data);

#endif


#if USE_I2C == 1 && USE_ST7032 == 1

	i2c_start();
	i2c_write(ST7032_ADDR);
	i2c_write(CTRL_BYTE_RS);
	i2c_write(data);
	i2c_stop();
	os_delay_us(WRITE_DELAY_US);

#endif
}




//**************************  FUNKCJE PRZEZNACZONE TAKØE DLA INNYCH MODU£”W  ******************


//----------------------------------------------------------------------------------------
//
//		 Wys≥anie pojedynczego znaku do wyúwietlacza LCD w postaci argumentu
//
//		 8 w≥asnych znakÛw zdefiniowanych w CGRAM
//		 wysy≥amy za pomocπ kodÛw 0x80 do 0x87 zamiast 0x00 do 0x07
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_char(char c) {

	lcd_write_data( ( c>=0x80 && c<=0x87 ) ? (c & 0x07) : c);

	ax++;
}


//----------------------------------------------------------------------------------------
//
//		 Wys≥anie stringa do wyúwietlacza LCD z pamiÍci RAM
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_str( char * str ) {

	register char znak;
	while ( (znak=*(str++)) ) lcd_char( znak );

}

void ICACHE_FLASH_ATTR lcd_str_al( uint8_t y, uint8_t x, char * str, uint8_t align ) {

	if( _center == align ) x = x/2 - strlen(str)/2+1;
	else if( _right == align ) x = x - strlen(str)+1;

	lcd_locate( y, x );

	register char znak;
	while ( (znak=*(str++)) ) lcd_char( znak );

}








//----------------------------------------------------------------------------------------
//
//		 Wyúwietla liczbÍ dziesiÍtnπ na wyúwietlaczu LCD
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_int( int32_t val ) {

	char bufor[17];
//	ltoa(val, bufor, 10);
	os_sprintf( bufor, "%d", val );
	lcd_str( bufor );
}




#if USE_BIG_DIGITS > 0

//#if USE_BIG_DIGITS == 1
const uint8_t bf01_a0[] ICACHE_RODATA_ATTR = {31,31,32,32,32,32,32,32};	// segment 	A
const uint8_t bf01_a1[] ICACHE_RODATA_ATTR = {31,31,31,31,31,31,31,31};	// segment  B / C / E / F
const uint8_t bf01_a2[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,31,31};	// segment  D
const uint8_t bf01_a3[] ICACHE_RODATA_ATTR = {31,32,32,32,32,32,32,32};	// A ma≥y
const uint8_t bf01_a4[] ICACHE_RODATA_ATTR = {31,31,32,32,32,32,31,31};	// segment  A i G
const uint8_t bf01_a5[] ICACHE_RODATA_ATTR = {31,31,32,32,32,32,32,31};	// A i G ma≥y
const uint8_t bf01_a6[] ICACHE_RODATA_ATTR = {31,32,32,32,32,32,31,31};	// A ma≥y i G
const uint8_t bf01_a7[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,32,31};	// D ma≥y
//#endif

//#if USE_BIG_DIGITS == 2
const uint8_t bf02_a0[] ICACHE_RODATA_ATTR = {21,21,32,32,32,32,32,32};	// 0  segment 	A
const uint8_t bf02_a1[] ICACHE_RODATA_ATTR = {21,21,21,21,21,21,21,21};	// 1  segment  B / C / E / F
const uint8_t bf02_a2[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,21,21};	// 2  segment  D
const uint8_t bf02_a3[] ICACHE_RODATA_ATTR = {31,31,31,31,31,31,31,31};	// 3  A ma≥y				kas
const uint8_t bf02_a4[] ICACHE_RODATA_ATTR = {21,21,32,32,32,32,21,21};	// 4  segment  A i G
const uint8_t bf02_a5[] ICACHE_RODATA_ATTR = {31,31,31,31,31,31,31,31};	// 5  A i G ma≥y			kas
const uint8_t bf02_a6[] ICACHE_RODATA_ATTR = {31,31,31,31,31,31,31,31};	// 6  A ma≥y i G			kas
const uint8_t bf02_a7[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,32,21};	// 7  D ma≥y
//#endif

//#if USE_BIG_DIGITS == 3
const uint8_t bf03_a0[] ICACHE_RODATA_ATTR = {21,21,32,32,32,32,32,32};	// 0  segment 	A
const uint8_t bf03_a1[] ICACHE_RODATA_ATTR = {20,20,20,20,20,20,20,20};	// 1  segment  B / C
const uint8_t bf03_a2[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,21,21};	// 2  segment  D
const uint8_t bf03_a3[] ICACHE_RODATA_ATTR = {5,5,5,5,5,5,5,5};			// 3  segment  E / F
const uint8_t bf03_a4[] ICACHE_RODATA_ATTR = {21,21,32,32,32,32,21,21};	// 4  segment  A i G
const uint8_t bf03_a5[] ICACHE_RODATA_ATTR = {5,5,32,32,32,32,5,5};		// 5  A i G ma≥y			kas
const uint8_t bf03_a6[] ICACHE_RODATA_ATTR = {20,20,32,32,32,32,20,20};	// 6  A ma≥y i G			kas
const uint8_t bf03_a7[] ICACHE_RODATA_ATTR = {5,5,32,32,32,32,32,32};	// 7  D ma≥y
//#endif

//#if USE_BIG_DIGITS == 4
const uint8_t bf04_a0[] ICACHE_RODATA_ATTR = {31,31,32,32,32,32,32,32};	// 0  segment 	A
const uint8_t bf04_a1[] ICACHE_RODATA_ATTR = {24,24,24,24,24,24,24,24};	// 1  segment  B / C
const uint8_t bf04_a2[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,31,31};	// 2  segment  D
const uint8_t bf04_a3[] ICACHE_RODATA_ATTR = {3,3,3,3,3,3,3,3};			// 3  segment  E / F
const uint8_t bf04_a4[] ICACHE_RODATA_ATTR = {31,31,32,32,32,32,31,31};	// 4  segment  A i G
const uint8_t bf04_a5[] ICACHE_RODATA_ATTR = {24,24,32,32,32,32,32,32};	// 5  A i G ma≥y			kas
const uint8_t bf04_a6[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,3,3};	// 6  A ma≥y i G			kas
const uint8_t bf04_a7[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,24,24};	// 7  D ma≥y
//#endif

//#if USE_BIG_DIGITS == 5
const uint8_t bf05_a0[] ICACHE_RODATA_ATTR = {31,32,32,32,32,32,32,32};	// 0  segment 	A
const uint8_t bf05_a1[] ICACHE_RODATA_ATTR = {16,16,16,16,16,16,16,16};	// 1  segment  B / C
const uint8_t bf05_a2[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,32,31};	// 2  segment  D
const uint8_t bf05_a3[] ICACHE_RODATA_ATTR = {1,1,1,1,1,1,1,1};			// 3  segment  E / F
const uint8_t bf05_a4[] ICACHE_RODATA_ATTR = {31,32,32,32,32,32,32,31};	// 4  segment  A i G
const uint8_t bf05_a5[] ICACHE_RODATA_ATTR = {32,32,1,2,4,32,32,32};	// 5  A i G ma≥y			kas
const uint8_t bf05_a6[] ICACHE_RODATA_ATTR = {16,16,16,16,16,16,16,22};	// 6  A ma≥y i G			kas
const uint8_t bf05_a7[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,32,3};	// 7  D ma≥y
//#endif

//#if USE_BIG_DIGITS == 6
const uint8_t bf06_a0[] ICACHE_RODATA_ATTR = {32,21,32,32,32,32,32,32};	// 0  segment 	A
const uint8_t bf06_a1[] ICACHE_RODATA_ATTR = {32,16,32,16,32,16,32,16};	// 1  segment  B / C
const uint8_t bf06_a2[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,32,21};	// 2  segment  D
const uint8_t bf06_a3[] ICACHE_RODATA_ATTR = {32,1,32,1,32,1,32,1};		// 3  segment  E / F
const uint8_t bf06_a4[] ICACHE_RODATA_ATTR = {21,32,32,32,32,32,32,21};	// 4  segment  A i G
const uint8_t bf06_a5[] ICACHE_RODATA_ATTR = {1,32,32,32,32,32,32,1};	// 5  A i G ma≥y			kas
const uint8_t bf06_a6[] ICACHE_RODATA_ATTR = {16,32,32,32,32,32,32,16};	// 6  A ma≥y i G			kas
const uint8_t bf06_a7[] ICACHE_RODATA_ATTR = {32,21,32,32,32,32,32,21};	// 7  D ma≥y
//#endif

//#if USE_BIG_DIGITS == 7
const uint8_t bf07_a0[] ICACHE_RODATA_ATTR = {32,21,32,32,32,32,32,32};	// 0  segment 	A
const uint8_t bf07_a1[] ICACHE_RODATA_ATTR = {32,20,32,20,32,20,32,20};	// 1  segment  B / C
const uint8_t bf07_a2[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,32,21};	// 2  segment  D
const uint8_t bf07_a3[] ICACHE_RODATA_ATTR = {32,5,32,5,32,5,32,5};		// 3  segment  E / F
const uint8_t bf07_a4[] ICACHE_RODATA_ATTR = {21,32,32,32,32,32,32,21};	// 4  segment  A i G
const uint8_t bf07_a5[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,32,16};	// 5  A i G ma≥y			kas
const uint8_t bf07_a6[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,32,1};	// 6  A ma≥y i G			kas
const uint8_t bf07_a7[] ICACHE_RODATA_ATTR = {32,21,32,32,32,32,32,21};	// 7  D ma≥y
//#endif

//#if USE_BIG_DIGITS == 8
const uint8_t bf08_a0[] ICACHE_RODATA_ATTR = {21,10,32,32,32,32,32,32};	// 0  segment 	A
const uint8_t bf08_a1[] ICACHE_RODATA_ATTR = {8,20,8,20,8,20,8,20};		// 1  segment  B / C
const uint8_t bf08_a2[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,10,21};	// 2  segment  D
const uint8_t bf08_a3[] ICACHE_RODATA_ATTR = {2,5,2,5,2,5,2,5};			// 3  segment  E / F
const uint8_t bf08_a4[] ICACHE_RODATA_ATTR = {21,10,32,32,32,32,10,21};	// 4  segment  A i G
const uint8_t bf08_a5[] ICACHE_RODATA_ATTR = {1,32,32,32,32,32,32,1};	// 5  A i G ma≥y			kas
const uint8_t bf08_a6[] ICACHE_RODATA_ATTR = {16,32,32,32,32,32,32,16};	// 6  A ma≥y i G			kas
const uint8_t bf08_a7[] ICACHE_RODATA_ATTR = {21,10,32,32,32,32,32,21};	// 7  D ma≥y
//#endif

//#if USE_BIG_DIGITS == 9
const uint8_t bf09_a0[] ICACHE_RODATA_ATTR = {31,14,32,32,32,32,32,32};	// 0  segment 	A
const uint8_t bf09_a1[] ICACHE_RODATA_ATTR = {8,28,28,28,28,28,28,8};	// 1  segment  B / C
const uint8_t bf09_a2[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,14,31};	// 2  segment  D
const uint8_t bf09_a3[] ICACHE_RODATA_ATTR = {2,7,7,7,7,7,7,2};			// 3  segment  E / F
const uint8_t bf09_a4[] ICACHE_RODATA_ATTR = {31,14,32,32,32,32,14,31};	// 4  segment  A i G
const uint8_t bf09_a5[] ICACHE_RODATA_ATTR = {1,32,32,32,32,32,32,1};	// 5  A i G ma≥y			kas
const uint8_t bf09_a6[] ICACHE_RODATA_ATTR = {16,32,32,32,32,32,32,16};	// 6  A ma≥y i G			kas
const uint8_t bf09_a7[] ICACHE_RODATA_ATTR = {21,10,32,32,32,32,32,21};	// 7  D ma≥y
//#endif

//#if USE_BIG_DIGITS == 10
const uint8_t bf10_a0[] ICACHE_RODATA_ATTR = {31,32,32,32,32,32,32,32};	// 0  segment 	A
const uint8_t bf10_a1[] ICACHE_RODATA_ATTR = {32,16,16,16,16,16,16,32};	// 1  segment  B / C
const uint8_t bf10_a2[] ICACHE_RODATA_ATTR = {32,32,32,32,32,32,32,31};	// 2  segment  D
const uint8_t bf10_a3[] ICACHE_RODATA_ATTR = {32,1,1,1,1,1,1,32};		// 3  segment  E / F
const uint8_t bf10_a4[] ICACHE_RODATA_ATTR = {31,32,32,32,32,32,32,31};	// 4  segment  A i G
const uint8_t bf10_a5[] ICACHE_RODATA_ATTR = {1,32,32,32,32,32,32,1};	// 5  A i G ma≥y			kas
const uint8_t bf10_a6[] ICACHE_RODATA_ATTR = {16,32,32,32,32,32,32,16};	// 6  A ma≥y i G			kas
const uint8_t bf10_a7[] ICACHE_RODATA_ATTR = {1,32,32,32,32,32,32,32};	// 7  D ma≥y
//#endif

const uint8_t  * const (font_tab)[10][8] ICACHE_RODATA_ATTR = {					//xxx:
		{bf01_a0, bf01_a1, bf01_a2, bf01_a3, bf01_a4, bf01_a5, bf01_a6, bf01_a7},

//		bf06_a0, bf06_a1, bf06_a2, bf06_a3, bf06_a4, bf06_a5, bf06_a6, bf06_a7,

		{bf02_a0, bf02_a1, bf02_a2, bf02_a3, bf02_a4, bf02_a5, bf02_a6, bf02_a7},
		{bf03_a0, bf03_a1, bf03_a2, bf03_a3, bf03_a4, bf03_a5, bf03_a6, bf03_a7},
		{bf04_a0, bf04_a1, bf04_a2, bf04_a3, bf04_a4, bf04_a5, bf04_a6, bf04_a7},
		{bf05_a0, bf05_a1, bf05_a2, bf05_a3, bf05_a4, bf05_a5, bf05_a6, bf05_a7},
		{bf06_a0, bf06_a1, bf06_a2, bf06_a3, bf06_a4, bf06_a5, bf06_a6, bf06_a7},
		{bf07_a0, bf07_a1, bf07_a2, bf07_a3, bf07_a4, bf07_a5, bf07_a6, bf07_a7},
		{bf08_a0, bf08_a1, bf08_a2, bf08_a3, bf08_a4, bf08_a5, bf08_a6, bf08_a7},
		{bf09_a0, bf09_a1, bf09_a2, bf09_a3, bf09_a4, bf09_a5, bf09_a6, bf09_a7},
		{bf10_a0, bf10_a1, bf10_a2, bf10_a3, bf10_a4, bf10_a5, bf10_a6, bf10_a7},
};



void ICACHE_FLASH_ATTR set_current_big_font( uint8_t nr ) {


	if( nr>10 ) nr = 10;

	current_big_font = nr;

	nr -= 1;
	lcd_defchar( 0, font_tab[nr][0] );
	lcd_defchar( 1, font_tab[nr][1] );
	lcd_defchar( 2, font_tab[nr][2] );
	lcd_defchar( 3, font_tab[nr][3] );
	lcd_defchar( 4, font_tab[nr][4] );
	lcd_defchar( 5, font_tab[nr][5] );
	lcd_defchar( 6, font_tab[nr][6] );
	lcd_defchar( 7, font_tab[nr][7] );

}







static void ICACHE_FLASH_ATTR display_big_digit( int8_t dig ) {

	if( dig<0 ) {
		lcd_char(2);	// minus
		ax--;
#if USE_BIG_DIGITS < 3
		lcd_char(32);
#endif
		ax++;
		dig = dig * -1;
	}

	int sx = ax;

if( current_big_font == 1 ) {
	if( !dig ) {
		lcd_char(1); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(1); lcd_char(2); lcd_char(1);
	}
	else if( 1==dig ) {
		lcd_char(0); lcd_char(1); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(2); lcd_char(1); lcd_char(2);
	}
	else if( 2==dig ) {
		lcd_char(0); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(1); lcd_char(4); lcd_char(4);
	}
	else if( 3==dig ) {
		lcd_char(0); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(4); lcd_char(4); lcd_char(1);
	}
	else if( 4==dig ) {
		lcd_char(1); lcd_char(32); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(0); lcd_char(0); lcd_char(1);
	}
	else if( 5==dig ) {
		lcd_char(1); lcd_char(4); lcd_char(4);
		lcd_locate(ay+1,sx);
		lcd_char(2); lcd_char(2); lcd_char(1);
	}
	else if( 6==dig ) {
		lcd_char(1); lcd_char(4); lcd_char(4);
		lcd_locate(ay+1,sx);
		lcd_char(1); lcd_char(2); lcd_char(1);
	}
	else if( 7==dig ) {
		lcd_char(1); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(32); lcd_char(1);
	}
	else if( 8==dig ) {
		lcd_char(1); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(1); lcd_char(4); lcd_char(1);
	}
	else if( 9==dig ) {
		lcd_char(1); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(4); lcd_char(4); lcd_char(1);
	}
}

else if( current_big_font == 2 ) {
	if( !dig ) {
		lcd_char(1); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(1); lcd_char(2); lcd_char(1);
	}
	else if( 1==dig ) {
		lcd_char(0); lcd_char(1); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(2); lcd_char(1); lcd_char(2);
	}
	else if( 2==dig ) {
		lcd_char(4); lcd_char(4); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(1); lcd_char(4); lcd_char(4);
	}
	else if( 3==dig ) {
		lcd_char(4); lcd_char(4); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(4); lcd_char(4); lcd_char(1);
	}
	else if( 4==dig ) {
		lcd_char(1); lcd_char(2); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(0); lcd_char(0); lcd_char(1);
	}
	else if( 5==dig ) {
		lcd_char(1); lcd_char(4); lcd_char(4);
		lcd_locate(ay+1,sx);
		lcd_char(4); lcd_char(4); lcd_char(1);
	}
	else if( 6==dig ) {
		lcd_char(1); lcd_char(4); lcd_char(4);
		lcd_locate(ay+1,sx);
		lcd_char(1); lcd_char(4); lcd_char(1);
	}
	else if( 7==dig ) {
		lcd_char(1); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(32); lcd_char(1);
	}
	else if( 8==dig ) {
		lcd_char(1); lcd_char(4); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(1); lcd_char(4); lcd_char(1);
	}
	else if( 9==dig ) {
		lcd_char(1); lcd_char(4); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(4); lcd_char(4); lcd_char(1);
	}
}


else if( current_big_font == 3 ) {
	if( !dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(2); lcd_char(1);
	}
	else if( 1==dig ) {
		lcd_char(7); lcd_char(1); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(1); lcd_char(32);
	}
	else if( 2==dig ) {
		lcd_char(5); lcd_char(4); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(6);
	}
	else if( 3==dig ) {
		lcd_char(5); lcd_char(4); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(5); lcd_char(4); lcd_char(1);
	}
	else if( 4==dig ) {
		lcd_char(3); lcd_char(2); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(7); lcd_char(0); lcd_char(1);
	}
	else if( 5==dig ) {
		lcd_char(3); lcd_char(4); lcd_char(6);
		lcd_locate(ay+1,sx);
		lcd_char(5); lcd_char(4); lcd_char(1);
	}
	else if( 6==dig ) {
		lcd_char(3); lcd_char(4); lcd_char(6);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 7==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(32); lcd_char(1);
	}
	else if( 8==dig ) {
		lcd_char(3); lcd_char(4); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 9==dig ) {
		lcd_char(3); lcd_char(4); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(5); lcd_char(4); lcd_char(1);
	}
}

else if( current_big_font == 4 ) {
	if( !dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(2); lcd_char(1);
	}
	else if( 1==dig ) {
		lcd_char(32); lcd_char(1); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(1); lcd_char(32);
	}
	else if( 2==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(32);
	}
	else if( 3==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(4); lcd_char(1);
	}
	else if( 4==dig ) {
		lcd_char(3); lcd_char(32); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(0); lcd_char(1);
	}
	else if( 5==dig ) {
		lcd_char(3); lcd_char(4); lcd_char(5);
		lcd_locate(ay+1,sx);
		lcd_char(6); lcd_char(2); lcd_char(1);
	}
	else if( 6==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 7==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(32); lcd_char(1);
	}
	else if( 8==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 9==dig ) {
		lcd_char(3); lcd_char(4); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(2); lcd_char(1);
	}
}

else if( current_big_font == 5 ) {
	if( !dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(2); lcd_char(1);
	}
	else if( 1==dig ) {
		lcd_char(5); lcd_char(1); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(7); lcd_char(6); lcd_char(32);
	}
	else if( 2==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(32);
	}
	else if( 3==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(4); lcd_char(1);
	}
	else if( 4==dig ) {
		lcd_char(3); lcd_char(32); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(0); lcd_char(1);
	}
	else if( 5==dig ) {
		lcd_char(3); lcd_char(4); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(2); lcd_char(1);
	}
	else if( 6==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 7==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(32); lcd_char(1);
	}
	else if( 8==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 9==dig ) {
		lcd_char(3); lcd_char(4); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(2); lcd_char(1);
	}
}

else if( current_big_font == 6 ) {
	if( !dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(2); lcd_char(1);
	}
	else if( 1==dig ) {
		lcd_char(32); lcd_char(1); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(1); lcd_char(32);
	}
	else if( 2==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(32);
	}
	else if( 3==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(4); lcd_char(1);
	}
	else if( 4==dig ) {
		lcd_char(3); lcd_char(2); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(32); lcd_char(1);
	}
	else if( 5==dig ) {
		lcd_char(3); lcd_char(7); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(2); lcd_char(1);
	}
	else if( 6==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 7==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(32); lcd_char(1);
	}
	else if( 8==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 9==dig ) {
		lcd_char(3); lcd_char(7); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(2); lcd_char(1);
	}
}

else if( current_big_font == 7 ) {
	if( !dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(2); lcd_char(1);
	}
	else if( 1==dig ) {
		lcd_char(32); lcd_char(1); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(1); lcd_char(32);
	}
	else if( 2==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(5);
	}
	else if( 3==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(6); lcd_char(4); lcd_char(1);
	}
	else if( 4==dig ) {
		lcd_char(3); lcd_char(32); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(0); lcd_char(1);
	}
	else if( 5==dig ) {
		lcd_char(3); lcd_char(7); lcd_char(5);
		lcd_locate(ay+1,sx);
		lcd_char(6); lcd_char(2); lcd_char(1);
	}
	else if( 6==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 7==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(32); lcd_char(1);
	}
	else if( 8==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 9==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(6); lcd_char(4); lcd_char(1);
	}
}

else if( current_big_font == 8 ) {
	if( !dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(2); lcd_char(1);
	}
	else if( 1==dig ) {
		lcd_char(32); lcd_char(1); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(1); lcd_char(32);
	}
	else if( 2==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(32);
	}
	else if( 3==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(4); lcd_char(1);
	}
	else if( 4==dig ) {
		lcd_char(3); lcd_char(32); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(0); lcd_char(1);
	}
	else if( 5==dig ) {
		lcd_char(3); lcd_char(4); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(2); lcd_char(1);
	}
	else if( 6==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 7==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(32); lcd_char(1);
	}
	else if( 8==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 9==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(4); lcd_char(1);
	}
}

else if( current_big_font == 9 ) {
	if( !dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(2); lcd_char(1);
	}
	else if( 1==dig ) {
		lcd_char(32); lcd_char(1); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(1); lcd_char(32);
	}
	else if( 2==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(32);
	}
	else if( 3==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(4); lcd_char(1);
	}
	else if( 4==dig ) {
		lcd_char(3); lcd_char(32); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(0); lcd_char(1);
	}
	else if( 5==dig ) {
		lcd_char(3); lcd_char(4); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(2); lcd_char(1);
	}
	else if( 6==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 7==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(32); lcd_char(1);
	}
	else if( 8==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 9==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(4); lcd_char(1);
	}
}


else if( current_big_font == 10 ) {
	if( !dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(2); lcd_char(1);
	}
	else if( 1==dig ) {
		lcd_char(32); lcd_char(1); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(1); lcd_char(32);
	}
	else if( 2==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(32);
	}
	else if( 3==dig ) {
		lcd_char(32); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(4); lcd_char(1);
	}
	else if( 4==dig ) {
		lcd_char(3); lcd_char(32); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(0); lcd_char(1);
	}
	else if( 5==dig ) {
		lcd_char(3); lcd_char(4); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(2); lcd_char(1);
	}
	else if( 6==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(32);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 7==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(32); lcd_char(1);
	}
	else if( 8==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(3); lcd_char(4); lcd_char(1);
	}
	else if( 9==dig ) {
		lcd_char(3); lcd_char(0); lcd_char(1);
		lcd_locate(ay+1,sx);
		lcd_char(32); lcd_char(4); lcd_char(1);
	}
}


	ay--;
	ax = sx;

}

void ICACHE_FLASH_ATTR lcd_big_dig( uint8_t font_nr, uint8_t y, uint8_t x, int8_t dig ) {

	lcd_locate(y,x);

	ay = y; ax = x;

	display_big_digit(dig);

	ay=y;
	if( font_nr < 3 ) ax+=4;
	else ax+=3;

//	lcd_locate(ay,ax);

}

void ICACHE_FLASH_ATTR lcd_big_dig1( uint8_t font_nr, int8_t dig ) {

	lcd_locate(ay,ax);

	display_big_digit(dig);

	if( font_nr < 3 ) ax+=4;
	else ax+=3;

}

void ICACHE_FLASH_ATTR lcd_int_big( uint8_t font_nr, int32_t val ) {

	char bufor[17];
//	ltoa(val, bufor, 10);
	os_sprintf( bufor, "%d", val );
	int8_t w = 0;

	char * c = bufor;
	uint8_t minus = val < 0;
	if( minus ) {
		c++;
		w = *c-'0';
		w *= -1;
		lcd_big_dig1( font_nr, w );
		c++;
		if( !*c ) return;
	}
	do {
		w = *c-'0';
		lcd_big_dig1( font_nr, w );
		c++;
	} while( *c );



}
#endif

void ICACHE_FLASH_ATTR lcd_int_al( uint8_t y, uint8_t x, int32_t val, uint8_t align ) {

	char bufor[17];
	os_sprintf( bufor, "%d", val );
	lcd_str_al( y, x, bufor, align );
}

void ICACHE_FLASH_ATTR lcd_long( uint32_t val ) {

	char bufor[17];
	os_sprintf( bufor, "%d", val );
	lcd_str( bufor );
}

void ICACHE_FLASH_ATTR lcd_long_al( uint8_t y, uint8_t x, uint32_t val, uint8_t align ) {

	char bufor[17];
	os_sprintf( bufor, "%d", val );
	lcd_str_al( y, x, bufor, align );
}



// konwersja do postaci binarnej liczb max 32-bitowych
// ARG:
// val - liczba do konwersji
// len - iloúÊ znakÛw postaci binarnej z zerami nieznaczπcymi
void ICACHE_FLASH_ATTR lcd_bin_al( uint8_t y, uint8_t x, uint32_t val, uint8_t len, uint8_t align ) {
	char str[len+1];
	memset( str, 0, len+1 );
	for( int8_t i=0, k=len-1; i<len; i++ ) {
		uint32_t a = val >> k;
		if( a & 0x0001 ) str[k]='1'; else str[k]='0';
		k--;
	}
//	strrev( str );							UWAGA bÍdzie babol !!! bo w ESP nie ma strrev
	lcd_str_al( y, x, str, align );
}

void ICACHE_FLASH_ATTR lcd_bin( uint32_t val, uint8_t len ) {
	char str[len+1];
	memset( str, 0, len+1 );
	for( int8_t i=0, k=len-1; i<len; i++ ) {
		uint32_t a = val >> k;
		if( a & 0x0001 ) str[k]='1'; else str[k]='0';
		k--;
	}
//	strrev( str );						UWAGA bÍdzie babol bo w ESP nie ma tej funkcji
	lcd_str( str );
}


//----------------------------------------------------------------------------------------
//
//		 Wyúwietla liczbÍ szestnastkowπ HEX na wyúwietlaczu LCD
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_hex( int32_t val ) {
	char bufor[17];
	os_sprintf( bufor, "%X", val );
	lcd_str( bufor );
}

void ICACHE_FLASH_ATTR lcd_hex_al( uint8_t y, uint8_t x, int32_t val, uint8_t align ) {
	char bufor[17];
	os_sprintf( bufor, "%X", val );
	lcd_str_al( y, x, bufor, align );
}



//----------------------------------------------------------------------------------------
//
//		Definicja w≥asnego znaku na LCD z pamiÍci FLASH w ESP	xxx:
//
//		argumenty:
//		nr: 		- kod znaku w pamiÍci CGRAM od 0x80 do 0x87
//		*def_znak:	- wskaünik do tablicy 7 bajtÛw definiujπcych znak
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_defchar( uint8_t nr, const uint8_t *def_znak ) {

	register uint8_t i,c;

#if USE_ST7032 == 0
	lcd_write_cmd( 64+((nr&0x07)*8) );
	for(i=0;i<8;i++) {
		c = *(def_znak++);
		lcd_write_data(c);
	}

#else

	// change instruction set
	lcd_write_cmd(FUNCTION_SET
		| FUNCTION_SET_DL
		| FUNCTION_SET_N);

	lcd_write_cmd(SET_CGRAM_ADDRESS | ((nr&0x07)*8) );
	for (i = 0; i < 8; i++) {
		c = *(def_znak++);
		lcd_write_data(c);
	}

	// change back to previous instruction set
	lcd_write_cmd(FUNCTION_SET
		| FUNCTION_SET_DL
		| FUNCTION_SET_N
		| FUNCTION_SET_IS);

#endif
}





//----------------------------------------------------------------------------------------
//
//		Ustawienie kursora w pozycji Y-wiersz, X-kolumna
//
// 		Y = od 0 do 3
// 		X = od 0 do n
//
//		funkcja dostosowuje automatycznie adresy DDRAM
//		w zaleønoúci od rodzaju wyúwietlacza (ile posiada wierszy)
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_locate(int y, int x) {

	ay = y; ax = x;

	if( x<0 || x>LCD_COLS-1 || y<0 || y>LCD_ROWS-1 ) return;

	switch(y) {
		case 0: y = LCD_LINE1; break;

#if (LCD_ROWS>1)
	    case 1: y = LCD_LINE2; break; // adres 1 znaku 2 wiersza
#endif
#if (LCD_ROWS>2)
    	case 2: y = LCD_LINE3; break; // adres 1 znaku 3 wiersza
#endif
#if (LCD_ROWS>3)
    	case 3: y = LCD_LINE4; break; // adres 1 znaku 4 wiersza
#endif
	}

	lcd_write_cmd( (0x80 + y + x) );
}



//----------------------------------------------------------------------------------------
//
//		Kasowanie ekranu wyúwietlacza
//
//		Argumenty:
//					0 - kasuje ca≥y wyúwietlacz i ustawia kursor w lewym gÛrnym rogu
//					n - kasuje liniÍ nr n i ustawia kursor na poczπtku tej linii
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_cls( uint8_t line ) {

	void lcd_clear_line( uint8_t ln ) {		// nested function -------------
		lcd_locate( ln, 0 );
		for( uint8_t i=0; i<LCD_COLS; i++ ) lcd_char(' ');
	}										// -----------------------------

	if( !line ) {
#if USE_ST7032 == 0
		for( uint8_t i=0; i<LCD_ROWS; i++ ) lcd_clear_line( i );

#else
		lcd_write_cmd(CLEAR_DISPLAY);
		os_delay_us(HOME_CLEAR_DELAY_US);
#endif
		lcd_locate( 0, 0 );
	} else {
		lcd_clear_line( line-1 );
		lcd_locate( line-1, 0 );
	}
}






//----------------------------------------------------------------------------------------
//
//		W≥πczenie kursora na LCD
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_cursor_on(void) {

	display |= LCDC_CURSORON;
	lcd_write_cmd( LCDC_ONOFF | display );
}

//----------------------------------------------------------------------------------------
//
//		Wy≥πczenie kursora na LCD
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_cursor_off(void) {

	display &= ~LCDC_CURSORON;
	lcd_write_cmd( LCDC_ONOFF|LCDC_DISPLAYON);
}


//----------------------------------------------------------------------------------------
//
//		W£πcza miganie kursora na LCD
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_blink_on(void) {

	display |= LCDC_BLINKON;
	lcd_write_cmd( LCDC_ONOFF | display );
}

//----------------------------------------------------------------------------------------
//
//		WY≥πcza miganie kursora na LCD
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_blink_off(void) {

	display &= ~LCDC_BLINKON;
	lcd_write_cmd( LCDC_ONOFF | display );
}


#if USE_ST7032 == 1



void ICACHE_FLASH_ATTR st7032_set_contrast(uint8_t value) {

	if (value > ST7032_CONTRAST_MAX)
		value = ST7032_CONTRAST_MIN;
	else if (value < ST7032_CONTRAST_MIN)
		value = ST7032_CONTRAST_MAX;
	lcd_write_cmd(CONTRAST_SET | (value & 0x0f));
	lcd_write_cmd((value >> 4) | POWER_ICON_BOST_CONTR | POWER_ICON_BOST_CONTR_Bon);
	contrast = value;
}
#endif


//----------------------------------------------------------------------------------------
//
//		 ******* INICJALIZACJA WYåWIETLACZA LCD ********
//
//----------------------------------------------------------------------------------------
void ICACHE_FLASH_ATTR lcd_init( void ) {




	#if USE_I2C == 1 && USE_ST7032 == 1

		os_delay_us(40000); // guesstimated value, seems to work fine

		lcd_write_cmd(FUNCTION_SET
			| FUNCTION_SET_DL
			| FUNCTION_SET_N
			| FUNCTION_SET_IS);

		lcd_write_cmd(INTERNAL_OSC_FREQ
			| INTERNAL_OSC_FREQ_BS
			| INTERNAL_OSC_FREQ_F2);

		lcd_write_cmd(POWER_ICON_BOST_CONTR
			| POWER_ICON_BOST_CONTR_Ion);

		st7032_set_contrast(contrast);

		lcd_write_cmd(FOLLOWER_CONTROL
			| FOLLOWER_CONTROL_Fon
			| FOLLOWER_CONTROL_Rab2);

		display = DISPLAY_ON_OFF_D;
		lcd_write_cmd(DISPLAY_ON_OFF | display);

		os_delay_us(300);

		lcd_cls(0);

	#if USE_BIG_DIGITS > 0
		set_current_big_font(USE_BIG_DIGITS);
	#endif

		return;
	#endif


#if USE_ST7032 == 0

#if USE_I2C
		// zerowanie bufora danych
		mpxLCD = 0;

		// ustawienie prÍdkoci i2c
//		i2cSetBitrate( I2C_KHZ );

#if USE_BACKLIGHT == 1
		// w≥πczenie podwietlenia
		lcd_LED(1);
#endif

		os_delay_us(15000);
		mpxLCD &= ~(1<<LCD_E);
		mpxLCD &= ~(1<<LCD_RS);
		#if USE_RW == 1
			mpxLCD &= ~(1<<LCD_RW);
		#endif
		SEND_I2C;

	#else
		// inicjowanie pinÛw portÛw ustalonych do pod≥πczenia z wyúwietlaczem LCD
		// ustawienie wszystkich jako wyjúcia

#if USE_BACKLIGHT == 1
		MKLCD_DDR(LCD_LED_PORT) |= (1<<LCD_LED);
		lcd_LED(0);
#endif

		data_dir_out();
		MKLCD_DDR(LCD_RSPORT) |= (1<<LCD_RS);
		MKLCD_DDR(LCD_EPORT) |= (1<<LCD_E);
		#if USE_RW == 1
			MKLCD_DDR(LCD_RWPORT) |= (1<<LCD_RW);
		#endif


		MKLCD_PORT(LCD_RSPORT) |= (1<<LCD_RS);
		MKLCD_PORT(LCD_EPORT) |= (1<<LCD_E);
		#if USE_RW == 1
			MKLCD_PORT(LCD_RWPORT) |= (1<<LCD_RW);
		#endif

		_delay_ms(15);
		MKLCD_PORT(LCD_EPORT) &= ~(1<<LCD_E);
		MKLCD_PORT(LCD_RSPORT) &= ~(1<<LCD_RS);
		MKLCD_PORT(LCD_RWPORT) &= ~(1<<LCD_RW);
	#endif



	// jeszcze nie moøna uøywaÊ Busy Flag
	SET_E;
	lcd_sendHalf(0x03);	// tryb 8-bitowy
	CLR_E;
	os_delay_us(41000);



	SET_E;
	lcd_sendHalf(0x03);	// tryb 8-bitowy
	CLR_E;
	os_delay_us(100);



	SET_E;
	lcd_sendHalf(0x03);	// tryb 8-bitowy
	CLR_E;
	os_delay_us(100);

	SET_E;
	lcd_sendHalf(0x02);// tryb 4-bitowy
	CLR_E;
	os_delay_us(100);



	// juø moøna uøywaÊ Busy Flag
	// tryb 4-bitowy, 2 wiersze, znak 5x7
	lcd_write_cmd( LCDC_FUNC|LCDC_FUNC4B|LCDC_FUNC2L|LCDC_FUNC5x7 );

	// wy≥πczenie kursora
	lcd_write_cmd( LCDC_ONOFF|LCDC_CURSOROFF );
	// w≥πczenie wyúwietlacza
	lcd_write_cmd( LCDC_ONOFF|LCDC_DISPLAYON );
	// przesuwanie kursora w prawo bez przesuwania zawartoúci ekranu
	lcd_write_cmd( LCDC_ENTRY|LCDC_ENTRYR );

	display = LCDC_DISPLAYON;

#endif

#if USE_BIG_DIGITS > 0
	set_current_big_font(USE_BIG_DIGITS);
#endif

	// kasowanie ekranu
	lcd_cls(0);

}




void ICACHE_FLASH_ATTR lcd_LED( uint8_t enable ) {

#if USE_BACKLIGHT == 1
	#if USE_I2C && USE_ST7032 == 0
			if( enable ) mpxLCD |= (1<<LCD_LED);
			else mpxLCD &= ~(1<<LCD_LED);

			SEND_I2C;
	#else
#if USE_ST7032 == 0		// xxx: podúwietlenie tylko gdy PCF8574
			if( enable ) MKLCD_PORT(LCD_LED_PORT) |= (1<<LCD_LED);
			else MKLCD_PORT(LCD_LED_PORT) &= ~(1<<LCD_LED);
#endif
	#endif
#endif
}





