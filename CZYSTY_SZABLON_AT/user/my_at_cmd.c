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



extern void at_exeCmdCiupdate( uint8_t id );

at_funcationType at_custom_cmd[] = {
    {"+TEST", 5, at_testCmdTest, at_queryCmdTest, at_setupCmdTest, at_exeCmdTest},
#ifdef AT_UPGRADE_SUPPORT
    {"+CIUPDATE", 9,       NULL,            NULL,            NULL, at_exeCmdCiupdate}
#endif
};



void ICACHE_FLASH_ATTR my_custom_at_cmd_init( void ) {
	at_cmd_array_regist( &at_custom_cmd[0], sizeof(at_custom_cmd)/sizeof(at_custom_cmd[0]) );
}
