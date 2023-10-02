/*
 * my_at_cmd.c
 *
 *  Created on: 6 lut 2022
 *      Author: Miros³aw Kardaœ
 */
#include "osapi.h"
#include "ets_sys.h"
#include "at_custom.h"
#include "user_interface.h"

#include "user_config.h"
#include "my_at_cmd.h"

#include "MK_LCD/mk_lcd44780.h"
#include "MODULES/mk_uart.h"



/************************************** TEST EXAMPLE - START ************************************************/
// test :AT+TEST=1,"abc"<,3>
void ICACHE_FLASH_ATTR
at_setupCmdTest(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    uint8 buffer[32] = {0};
    pPara++; // skip '='

    //get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    if (flag == FALSE) {
        at_response_error();
        return;
    }

    if (*pPara++ != ',') { // skip ','
        at_response_error();
        return;
    }

    os_sprintf(buffer, "the first parameter:%d\r\n", result);
    at_port_print(buffer);

    //get the second parameter
    // string
    at_data_str_copy(buffer, &pPara, 10);
    at_port_print_irom_str("the second parameter:");
    at_port_print(buffer);
    at_port_print_irom_str("\r\n");

    if (*pPara == ',') {
        pPara++; // skip ','
        result = 0;
        //there is the third parameter
        // digit
        flag = at_get_next_int_dec(&pPara, &result, &err);
        // we donot care of flag
        os_sprintf(buffer, "the third parameter:%d\r\n", result);
        at_port_print(buffer);
    }

    if (*pPara != '\r') {
        at_response_error();
        return;
    }

    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_testCmdTest(uint8_t id)
{
    at_port_print_irom_str("at_testCmdTest\r\n");
    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_queryCmdTest(uint8_t id)
{
    at_port_print_irom_str("at_queryCmdTest\r\n");
    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_exeCmdTest(uint8_t id)
{
    at_port_print_irom_str("at_exeCmdTest\r\n");
    at_response_ok();
}
/************************************** TEST EXAMPLE - STOP ************************************************/




//-----------------------[ AT+LCD=1,"mirekk36" ]---------------------------------------
// wyœwietl w linii y napis na LCD
void ICACHE_FLASH_ATTR at_setupCmdLcd(uint8_t id, char *pPara) {

//	*pPara  =1,"mirekk36"

	char buf[128];

	int result, err, flag, linia_lcd;

	pPara++; // skip '='

    if( *pPara == 0 ) {
        at_response_error();
        return;
    }

    //get the first parameter
    // digit
    flag = at_get_next_int_dec( &pPara, &linia_lcd, &err);

    // flag must be ture because there are more parameter
    if (flag == FALSE || linia_lcd > 2 ) {
        at_response_error();
        return;
    }

    if (*pPara++ != ',') { // skip ','
        at_response_error();
        return;
    }


    at_data_str_copy( buf, &pPara, 16 );

    if (*pPara != '\r') {
        at_response_error();
        return;
    }

    lcd_cls(linia_lcd);
    lcd_str(buf);





	at_response_ok();
}


//-----------------------[ AT+LCD ]---------------------------------------
// bez argumentów - ma tylko wyœwietliæ info
void ICACHE_FLASH_ATTR at_exeCmdLcd( uint8_t id ) {

	at_port_print_irom_str( "\r\nKomenda do wyœwietlania tekstów na COG\r\n" );
	at_response_ok();
}



//-----------------------[ AT+APNAME? ]---------------------------------------
// podaj ID/nazwê modu³u, domyœlna nazwa to: "ATNEL ESP-01m4"
void ICACHE_FLASH_ATTR at_queryCmdApName(uint8_t id) {

	struct softap_config config;

	wifi_softap_get_config( &config );

	at_port_print( config.ssid );
	at_response_ok();
}



//-----------------------[ AT+APNAME="ESP_Kurs" ]---------------------------------------
// ustaw nowe ID/nazwê modu³u, domyœlna nazwa to: "ATNEL ESP-01m4"
void ICACHE_FLASH_ATTR at_setupCmdApName(uint8_t id, char *pPara) {

	char buf[128];
	struct softap_config config;

	pPara++; // skip '='

    if( *pPara == 0 || os_strlen(pPara) > 34 ) {
        at_response_error();
        return;
    }

    at_data_str_copy(buf, &pPara, 32);

    if (*pPara != '\r') {
        at_response_error();
        return;
    }


	wifi_softap_get_config( &config );

	os_memset( config.ssid, 0, 32 );
	os_memcpy( config.ssid, buf, os_strlen(buf) );

	wifi_softap_set_config( &config );

	at_response_ok();
}









//-----------------------[ AT+UARTCFG? ]---------------------------------------
void ICACHE_FLASH_ATTR at_queryCmdUartCfg(uint8_t id) {

	uint8_t buffer[64] = {0};

//    os_sprintf( buffer, "\r\n+UART0:%d,%d,%d,%d,%d\r\n",
//    		mkEspConfig.uart0_cfg.baud,
//			mkEspConfig.uart0_cfg.data_bit,
//			mkEspConfig.uart0_cfg.stop_bit,
//			mkEspConfig.uart0_cfg.parity,
//			mkEspConfig.uart0_cfg.flow_ctrl
//	);
    at_port_print( buffer );

	at_response_ok();
}


//-----------------------[ AT+UARTCFG=115200,8,1,0,0 ]---------------------------------
// <baudrate>,<databits>,<stopbits>,<parity>,<flow control>
//           ,<5,6,7,8
void ICACHE_FLASH_ATTR at_setupCmdUartCfg(uint8_t id, char *pPara) {

	uint8_t buffer[64] = {0};
	int result = 0, err = 0, flag = 0;
	int baud, databits, stopbits, parity, flow;
	pPara++; // skip '='

    if( *pPara == 0 ) {
        at_response_error();
        return;
    }

    //.... baud ......
    flag = at_get_next_int_dec(&pPara, &baud, &err);
    if (flag == FALSE ) {
        at_response_error();
        return;
    }
    if (*pPara++ != ',') { // skip ','
        at_response_error();
        return;
    }

    //.... databits ......
    flag = at_get_next_int_dec(&pPara, &databits, &err);
    if (flag == FALSE ) {
        at_response_error();
        return;
    }
    if (*pPara++ != ',') { // skip ','
        at_response_error();
        return;
    }

    //.... stopbits ......
    flag = at_get_next_int_dec(&pPara, &stopbits, &err);
    if (flag == FALSE ) {
        at_response_error();
        return;
    }
    if (*pPara++ != ',') { // skip ','
        at_response_error();
        return;
    }

    //.... parity ......
    flag = at_get_next_int_dec(&pPara, &parity, &err);
    if (flag == FALSE ) {
        at_response_error();
        return;
    }
    if (*pPara++ != ',') { // skip ','
        at_response_error();
        return;
    }

    //.... flow control ......
    at_get_next_int_dec(&pPara, &flow, &err);

    if (*pPara != '\r') {
        at_response_error();
        return;
    }

//    mkEspConfig.uart0_cfg.baud 		= baud;
//    mkEspConfig.uart0_cfg.data_bit	= databits;
//    mkEspConfig.uart0_cfg.stop_bit	= stopbits;
//    mkEspConfig.uart0_cfg.parity	= parity;
//    mkEspConfig.uart0_cfg.flow_ctrl	= flow;

//    Flash_CFG_Save();

    at_port_print_irom_str("\r\nChamges will be done after restart\r\n");

    at_response_ok();
}




//-----------------------[ AT+ENTM ]---------------------------------------
// w³¹cz tryb transparentny
void ICACHE_FLASH_ATTR at_exeCmdEntm(uint8_t id) {

	change_transparent_mode( 1 );
	at_port_print_irom_str("\r\n+TRANSPARENT_MODE_ON:\r\n");

	at_response_ok();
}




//extern void at_exeCmdCiupdate( uint8_t id );
//
//at_funcationType at_custom_cmd[] = {
//    {"+TEST", 5, at_testCmdTest, at_queryCmdTest, at_setupCmdTest, at_exeCmdTest},
//#ifdef AT_UPGRADE_SUPPORT
//    {"+CIUPDATE", 9,       NULL,            NULL,            NULL, at_exeCmdCiupdate}
//#endif
//};


//						AT+CMD=?			   AT+CMD?					  AT+CMD=params              AT+CMD
at_funcationType at_custom_cmd[] = {
//	{"+TEST", 		5, at_testCmdTest, 		at_queryCmdTest, 			at_setupCmdTest, 			at_exeCmdTest		},
	{"+LCD", 		4, NULL,				NULL, 						at_setupCmdLcd, 			at_exeCmdLcd		},	// 1
	{"+APNAME", 	7, NULL,				at_queryCmdApName, 			at_setupCmdApName, 			NULL				},	// 2
	{"+UARTCFG",	8, NULL,				at_queryCmdUartCfg, 		at_setupCmdUartCfg, 		NULL				},	// 3
	{"+ENTM", 		5, NULL,				NULL, 						NULL, 						at_exeCmdEntm		},	// 4
};


void ICACHE_FLASH_ATTR my_custom_at_cmd_init( void ) {
	at_cmd_array_regist( &at_custom_cmd[0], sizeof(at_custom_cmd)/sizeof(at_custom_cmd[0]) );
}
