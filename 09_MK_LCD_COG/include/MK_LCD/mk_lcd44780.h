/*-----------------------------------------------------------------------------------------------------------
// *** Obs³uga wyœwietlaczy alfanumerycznych zgodnych z HD44780 ***
 *
 * ..............................................................................
 * . UWAGA! wersja dla ESP nie obs³uguje w ogóle wyœwietlaczy w trybie 4-bit	.
 * .        TYLKO I WY£¥CZNIE po magistrali I2C !!!								.
 * .	BIBLIOTEKA przeznaczona tylko dla ESP8266 w trybie NonOS SDK			.
 *  .............................................................................
 *
// - Sterowanie: tryb 4-bitowy
// - Dowolne przypisanie ka¿dego sygna³u steruj¹cego do dowolnego pinu mikrokontrolera
// - Praca z pinem RW pod³¹czonym do GND lub do mikrokontrolera (sprawdzanie BusyFLAG - szybkie operacje LCD)
//
//	Biblioteka MK_LCD ver: 3.0	(with COG Chip On Glass ST7032 I2C)
//	nowoœci:
//	- nowa szybka funkcja: lcd_cls(X) z argumentami X=0 - kasuje ca³y wyœwietlacz, X=n - kasuje liniê nr n wyœwietlacza
//    n - linie numerowane s¹ od 1 do 4 (a nie od 0 do 3). Po skasowaniu kursor ustawia siê na pocz¹tku linii n.
//  - poprawione makra aby nie k³óci³y siê z innymi kodami z Atnela
// 	- przyk³ady dla obs³ugi arduinowego konwertera I2C - LCD z PCF8584(A)
//  - Biblioteka dzia³a przy taktowaniu od 1MHz do 20MHz
//  - Biblioteka dzia³a na DOWOLNYCH mikrokontrolerach AVR
//  - 10 du¿ych fontów dla cyfr i znaku minus, do wyœwietlania wiêkszych liczb
//  - nowe funkcje: lcd_big_dig(), lcd_big_dig1(), lcd_int_big(), lcd_load_big_fonts()
//
//  - Do pracy z konwerterem (arduino) I2C-LCD wymagana jest dodatkowo biblioteka MK_I2C
//    któr¹ mo¿na zakupiæ w sklepie: https://sklep.atnel.pl/pl/searchquery/i2c/1/default/5?url=i2c
//
//  - Do pracy z konwerterem (arduino) I2C-LCD wymagana jest dodatkowo biblioteka MK_I2C
//    któr¹ mo¿na zakupiæ w sklepie: https://sklep.atnel.pl/pl/searchquery/i2c/1/default/5?url=i2c
//
// Pliki 			: mk_lcd44780.c , mk_lcd44780.h
// Mikrokontrolery 	: ESP8266 NonOS SDK
// Kompilator 		: xtensa-gcc
// ród³o 			: http://www.atnel.pl
// Data 			: 2021-05-29
// Autor 			: Miros³aw Kardaœ
//-----------------------------------------------------------------------------------------------------------
 */
#ifndef LCD_H_
#define LCD_H_

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

//----------------------------------------------------------------------------------------
//
//		Parametry pracy sterownika
//
//----------------------------------------------------------------------------------------
// rozdzielczoœæ wyœwietlacza LCD (wiersze/kolumny)
#define LCD_ROWS 	2		// iloœæ wierszy wyœwietlacza LCD
#define LCD_COLS 	16		// iloœæ kolumn wyœwietlacza LCD
#define LCD_TYPE	1		// typ 1 lub typ 2 (tylko w przypadku wyœwietlaczy 16x1 wystêpuj¹ 2 ró¿ne typy adresów pamiêci DDRAM)

/*.......... N O W O Œ Æ ...........................*/
/*..................................................*/
/* w³¹czanie BIG Fontów - do wyboru a¿ dziesiêæ 1-10 ró¿nych krojów czcionek dla cyfr ;) */
#define USE_BIG_DIGITS		1	// 0-wy³¹czone, 1-font nr 1, 2-font nr 2 ... itd    (10-ostatni)
/*..................................................*/

// tu ustalamy za pomoc¹ zera lub jedynki czy sterujemy pinem RW
//	0 - pin RW pod³¹czony na sta³e do GND
//	1 - pin RW pod³¹czony do mikrokontrolera
#define USE_RW 				1

/* jeœli korzystamy z pinu do sterowania podœwietleniem to 1, jeœli nie to 0 */
#define USE_BACKLIGHT		1		// podœwietlenie dzia³a tylko dla LCD z expanderem PCF8574

//----------------------------------------------------------------------------------------
//
//		Wybór trybu pracy I2C / Standard
//
//----------------------------------------------------------------------------------------
// w³¹czenie obs³ugi magistrali I2C (1 - w³¹czone, 0 - wy³¹czone)
#define USE_I2C				1	// dla ESP mo¿liwa TYLKO opcja = 1

// mo¿liwe wartoœci: aBITRATE_100KHZ lub aBITRATE_200_400KHZ - dla F_CPU=80MHz bitrate = 200 kHz, dla FCPU=160MHz bitrate = 400kHz



#define USE_ST7032			1	// 0-u¿yj LCD z PCF8574, 1-u¿yj COG ST7031



// ekspandery PCF8574(A) obs³uguj¹ standardowo 100 kHz mo¿na jednak zwiêkszaæ prêdkoœæ spokojnie do 250-300 kHz
// natomiast jeœli przewody I2C s¹ bardzo d³ugie mo¿na zmniejszyæ prêdkoœæ do 50 kHz

//----------------------------------------------------------------------------------------
//
//	Ustawienia sprzêtowe obs³ugi komunikacji I2C dla Ekspanderów PCF8574 oraz PCF8574A
//
//----------------------------------------------------------------------------------------
// Adres EXPANDERA
//#define PCF8574_LCD_ADDR 0x70	// PCF8574A gdy A0, A1 i A2 --> GND
//#define PCF8574_LCD_ADDR 0x7E	// PCF8574A gdy A0, A1 i A2 --> VCC		// najczêœciej w Arduino domyœlne ustawienie dla PCF8574A

//#define PCF8574_LCD_ADDR 0x40	// PCF8574  gdy A0, A1 i A2 --> GND
#define PCF8574_LCD_ADDR 0x4E	// PCF8574  gdy A0, A1 i A2 --> VCC		// najczêœciej w Arduino domyœlne ustawienie dla PCF8574

//----------------------------------------------------------------------------------------
//
//		Ustawienia sprzêtowe po³¹czeñ sterownika z mikrokontrolerem
//
//----------------------------------------------------------------------------------------
// tu konfigurujemy port i piny do jakich pod³¹czymy linie D7..D4 LCD
#if !USE_I2C
	#define LCD_D7PORT  A
	#define LCD_D7 6
	#define LCD_D6PORT  A
	#define LCD_D6 5
	#define LCD_D5PORT  A
	#define LCD_D5 4
	#define LCD_D4PORT  A
	#define LCD_D4 3


	// tu definiujemy piny procesora do których pod³¹czamy sygna³y RS,RW, E
	#define LCD_RSPORT A
	#define LCD_RS 0

	#define LCD_RWPORT A
	#define LCD_RW 1

	#define LCD_EPORT A
	#define LCD_E 2

	#if USE_BACKLIGHT == 1
		#define LCD_LED_PORT  A		// PODŒWIETLENIE LCD
		#define LCD_LED 7
	#endif

#endif

#if USE_I2C == 1 && USE_ST7032 == 0

/* definicja pinów dla konwertera I2C - LCD Arduino:
	#define LCD_D7 	7		// dane D7
	#define LCD_D6 	6		// dane D6
	#define LCD_D5 	5		// dane D5
	#define LCD_D4 	4		// dane D4
	#define LCD_LED 3		// PODŒWIETLENIE LCD
	#define LCD_E 	2		// E
	#define LCD_RW 	1		// RW
	#define LCD_RS 	0		// RS
 */

	// Tu definiujemy piny ekspandera do których pod³¹czamy sygna³y D7..D4 LCD
	#define LCD_D7 	7
	#define LCD_D6 	6
	#define LCD_D5 	5
	#define LCD_D4 	4

	// tu definiujemy piny ekspandera do których pod³¹czamy sygna³y RS,RW, E
	#define LCD_E 	2
	#define LCD_RW 	1
	#define LCD_RS 	0

	#if USE_BACKLIGHT == 1
		#define LCD_LED 3		// PODŒWIETLENIE LCD
	#endif

		#include "driver/mk_i2c.h"
#endif

#if USE_I2C == 1 && USE_ST7032 == 1

	#if USE_BACKLIGHT == 1
		#define LCD_LED_PORT  A		// PODŒWIETLENIE LCD
		#define LCD_LED 7
	#endif

	#include "driver/mk_i2c.h"
#endif


//------------------------------------------------  koniec ustawieñ sprzêtowych ---------------


//----------------------------------------------------------------------------------------
//****************************************************************************************
//*																						 *
//*		U S T A W I E N I A   KOMPILACJI												 *
//*																						 *
//*		W³¹czamy kompilacjê komend u¿ywanych lub wy³¹czamy nieu¿ywanych					 *
//*		(dziêki temu regulujemy zajêtoœæ pamiêci FLASH po kompilacji)					 *
//*																						 *
//*		1 - oznacza W£¥CZENIE do kompilacji												 *
//*		0 - oznacza wy³¹czenie z kompilacji (funkcja niedostêpna)						 *
//*																						 *
//****************************************************************************************


#define USE_EEPROM		0


//------------------------------------------------  koniec ustawieñ  ---------------




// definicje adresów w DDRAM dla ró¿nych wyœwietlaczy
// inne s¹ w wyœwietlaczach 2wierszowych i w 4wierszowych

#if ( (LCD_ROWS == 2) && (LCD_COLS == 16) ) || ( (LCD_ROWS == 2) && (LCD_COLS == 20) )
	#define LCD_LINE1 0x00		// adres 1 znaku 1 wiersza
	#define LCD_LINE2 0x40		// adres 1 znaku 2 wiersza
#endif

#if ( (LCD_ROWS == 4) && (LCD_COLS == 20) )
	#define LCD_LINE1 0x00		// adres 1 znaku 1 wiersza
	#define LCD_LINE2 0x40		// adres 1 znaku 2 wiersza
	#define LCD_LINE3 0x14  	// adres 1 znaku 3 wiersza
	#define LCD_LINE4 0x54  	// adres 1 znaku 4 wiersza
#endif

#if ( (LCD_ROWS == 4) && (LCD_COLS == 16) )
	#define LCD_LINE1 0x00		// adres 1 znaku 1 wiersza
	#define LCD_LINE2 0x40		// adres 1 znaku 2 wiersza
	#define LCD_LINE3 0x10  	// adres 1 znaku 3 wiersza
	#define LCD_LINE4 0x50  	// adres 1 znaku 4 wiersza
#endif

#if ( (LCD_ROWS == 2) && (LCD_COLS == 40) )
	#define LCD_LINE1 0x00		// adres 1 znaku 1 wiersza
	#define LCD_LINE2 0x40		// adres 1 znaku 2 wiersza
#endif

#if ( (LCD_ROWS == 4) && (LCD_COLS == 40) )
	#define LCD_LINE1 0x00		// adres 1 znaku 1 wiersza
	#define LCD_LINE2 0x40		// adres 1 znaku 2 wiersza
	#define LCD_LINE3 0x00  	// adres 1 znaku 3 wiersza
	#define LCD_LINE4 0x40  	// adres 1 znaku 4 wiersza
#endif

#if ( (LCD_ROWS == 1) && (LCD_COLS == 16) && LCD_TYPE == 1 )
	#define LCD_LINE1 0x00		// adres 1 znaku 1 wiersza
	#define LCD_LINE2 0x40		// adres 9 znaku 1 wiersza
#endif

#if ( (LCD_ROWS == 1) && (LCD_COLS == 16) && LCD_TYPE == 2 )
	#define LCD_LINE1 0x00		// adres 1 znaku 1 wiersza
#endif

#if ( (LCD_ROWS == 1) && (LCD_COLS == 8) )
	#define LCD_LINE1 0x00		// adres 1 znaku 1 wiersza
#endif




//Makra uproszczaj¹ce obs³ugê magistralii I2C		xxx
#define SEND_I2C 		i2c_send_byte(PCF8574_LCD_ADDR, mpxLCD)	//pcf8574_write( PCF8574_LCD_ADDR, mpxLCD )
#define RECEIVE_I2C  	i2c_read_byte(PCF8574_LCD_ADDR)	//pcf8574_read( PCF8574_LCD_ADDR )

// Makra upraszczaj¹ce dostêp do portów
#if USE_I2C == 0 || USE_ST7032 == 1
	// *** PORT
	#define MKLCD_PORT(x) MKLCD_SPORT(x)
	#define MKLCD_SPORT(x) (PORT##x)
	// *** PIN
	#define MKLCD_PIN(x) MKLCD_SPIN(x)
	#define MKLCD_SPIN(x) (PIN##x)
	// *** DDR
	#define MKLCD_DDR(x) MKLCD_SDDR(x)
	#define MKLCD_SDDR(x) (DDR##x)
#endif


// Komendy steruj¹ce
#define LCDC_CLS					0x01
#define LCDC_HOME					0x02
#define LCDC_ENTRY					0x04
	#define LCDC_ENTRYR					0x02
	#define LCDC_ENTRYL					0
	#define LCDC_MOVE					0x01
#define LCDC_ONOFF					0x08
	#define LCDC_DISPLAYON				0x04
	#define LCDC_CURSORON				0x02
	#define LCDC_CURSOROFF				0
	#define LCDC_BLINKON				0x01
#define LCDC_SHIFT					0x10
	#define LCDC_SHIFTDISP				0x08
	#define LCDC_SHIFTR					0x04
	#define LCDC_SHIFTL					0
#define LCDC_FUNC					0x20
	#define LCDC_FUNC8B					0x10
	#define LCDC_FUNC4B					0
	#define LCDC_FUNC2L					0x08
	#define LCDC_FUNC1L					0
	#define LCDC_FUNC5x10				0x04
	#define LCDC_FUNC5x7				0
#define LCDC_SET_CGRAM				0x40
#define LCDC_SET_DDRAM				0x80

enum { _left, _center, _right };





#if USE_ST7032 == 1


#define ST7032_ADDR			0x7c


/* Min contrast */
#define ST7032_CONTRAST_MIN		0

/* Max contrast */
#define ST7032_CONTRAST_MAX		63


#define CTRL_BYTE			0x00 // followed by command bytes
#define CTRL_BYTE_CO			0x80 // followed by 1 command byte
#define CTRL_BYTE_RS			0x40 // after last control byte, followed by DDRAM data byte(s)

#define CLEAR_DISPLAY			0x01 // Clear display

#define RETURN_HOME			0x02 // Cursor home to 00H

#define FUNCTION_SET			0x20 // DL: interface data is 8/4 bits, N: number of line is 2/1 DH: double height font, IS: instr
#define FUNCTION_SET_IS			0x01 // IS: instruction table select
#define FUNCTION_SET_DH			0x04 // DH: double height font
#define FUNCTION_SET_N			0x08 // N: number of line is 2/1
#define FUNCTION_SET_DL			0x10 // DL: interface data is 8/4 bits

#define INTERNAL_OSC_FREQ		0x10 // BS=1:1/4 bias, BS=0:1/5 bias, F2~0: adjust internal OSC frequency for FR frequency.
#define INTERNAL_OSC_FREQ_F0		0x01 // F2~0: adjust internal OSC frequency for FR frequency.
#define INTERNAL_OSC_FREQ_F1		0x02 // F2~0: adjust internal OSC frequency for FR frequency.
#define INTERNAL_OSC_FREQ_F2		0x04 // F2~0: adjust internal OSC frequency for FR frequency.
#define INTERNAL_OSC_FREQ_BS		0x08 // BS=1:1/4 bias (BS=0:1/5 bias)

#define POWER_ICON_BOST_CONTR		0x50 // Ion: ICON display on/off, Bon: set booster circuit on/off, C5,C4: Contrast set
#define POWER_ICON_BOST_CONTR_Bon	0x04 // Ion: ICON display on/off
#define POWER_ICON_BOST_CONTR_Ion	0x08 // Bon: set booster circuit on/off

#define FOLLOWER_CONTROL		0x60 // Fon: set follower circuit on/off, Rab2~0: select follower amplified ratio.
#define FOLLOWER_CONTROL_Rab0		0x01 // Rab2~0: select follower amplified ratio
#define FOLLOWER_CONTROL_Rab1		0x02 // Rab2~0: select follower amplified ratio
#define FOLLOWER_CONTROL_Rab2		0x04 // Rab2~0: select follower amplified ratio
#define FOLLOWER_CONTROL_Fon		0x08 // Fon: set follower circuit on/off

#define CONTRAST_SET			0x70 // C0-C3: Contrast set

#define DISPLAY_ON_OFF			0x08 // display on, cursor on, cursor position on
#define DISPLAY_ON_OFF_B		0x01 // cursor position on
#define DISPLAY_ON_OFF_C		0x02 // cursor on
#define DISPLAY_ON_OFF_D		0x04 // display on

#define SET_DDRAM_ADDRESS		0x80 // Set DDRAM address in address counter
#define SET_CGRAM_ADDRESS		0x40 //Set CGRAM address in address counter

#define LINE_1_ADDR			0x00
#define LINE_2_ADDR			0x40

#define WRITE_DELAY_US			30 // see data sheet
#define HOME_CLEAR_DELAY_US		1200 // see data sheet


#endif







#if USE_I2C == 0
	#undef USE_ST7032
	#define USE_ST7032 0
#endif


// deklaracje funkcji na potrzeby innych modu³ów
void ICACHE_FLASH_ATTR lcd_init(void);								// W£¥CZONA na sta³e do kompilacji
void ICACHE_FLASH_ATTR lcd_load_big_fonts( void );

void ICACHE_FLASH_ATTR lcd_cls( uint8_t line );						// W£¥CZONA na sta³e do kompilacji
void ICACHE_FLASH_ATTR lcd_str( char * str );							// W£¥CZONA na sta³e do kompilacji
void ICACHE_FLASH_ATTR lcd_str_al( uint8_t y, uint8_t x, char * str, uint8_t align );


void ICACHE_FLASH_ATTR lcd_locate(int y, int x);				// domyœlnie W£¥CZONA z kompilacji w pliku lcd.c

void ICACHE_FLASH_ATTR lcd_char(char c);								// domyœlnie wy³¹czona z kompilacji w pliku lcd.c
void ICACHE_FLASH_ATTR lcd_int( int32_t val );								// domyœlnie wy³¹czona z kompilacji w pliku lcd.c

void ICACHE_FLASH_ATTR lcd_big_dig( uint8_t font_nr, uint8_t y, uint8_t x, int8_t dig );
void ICACHE_FLASH_ATTR lcd_big_dig1( uint8_t font_nr, int8_t dig );
void ICACHE_FLASH_ATTR lcd_int_big( uint8_t font_nr, int32_t val );

void ICACHE_FLASH_ATTR set_current_big_font( uint8_t nr );

void ICACHE_FLASH_ATTR lcd_int_al( uint8_t y, uint8_t x, int32_t val, uint8_t align );
void ICACHE_FLASH_ATTR lcd_long( uint32_t val );
void ICACHE_FLASH_ATTR lcd_long_al( uint8_t y, uint8_t x, uint32_t val, uint8_t align );
void ICACHE_FLASH_ATTR lcd_bin( uint32_t val, uint8_t len );
void ICACHE_FLASH_ATTR lcd_bin_al( uint8_t x, uint8_t y, uint32_t val, uint8_t len, uint8_t align );
void ICACHE_FLASH_ATTR lcd_hex(int32_t val);								// domyœlnie wy³¹czona z kompilacji w pliku lcd.c
void ICACHE_FLASH_ATTR lcd_hex_al( uint8_t y, uint8_t x, int32_t val, uint8_t align );

// xxx:
void ICACHE_FLASH_ATTR lcd_defchar(uint8_t nr, const uint8_t *def_znak);	// domyœlnie wy³¹czona z kompilacji w pliku lcd.c

void ICACHE_FLASH_ATTR lcd_cursor_on(void);							// domyœlnie wy³¹czona z kompilacji w pliku lcd.c
void ICACHE_FLASH_ATTR lcd_cursor_off(void);							// domyœlnie wy³¹czona z kompilacji w pliku lcd.c
void ICACHE_FLASH_ATTR lcd_blink_on(void);							// domyœlnie wy³¹czona z kompilacji w pliku lcd.c
void ICACHE_FLASH_ATTR lcd_blink_off(void);							// domyœlnie wy³¹czona z kompilacji w pliku lcd.c

void ICACHE_FLASH_ATTR lcd_LED( uint8_t enable );							// domyœlnie wy³¹czona z kompilacji w pliku lcd.c

#endif /* LCD_H_ */
