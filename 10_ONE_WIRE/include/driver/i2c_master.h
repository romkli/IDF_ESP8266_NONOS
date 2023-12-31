/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __I2C_MASTER_H__
#define __I2C_MASTER_H__

#include "ets_sys.h"

#define BITRATE_100KHZ		0
#define BITRATE_200_400KHZ	1	// dla F_CPU=80MHz bitrate = 200 kHz, dla FCPU=160MHz bitrate = 400kHz



//#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_GPIO2_U
//#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_MTMS_U
//#define I2C_MASTER_SDA_GPIO 2
//#define I2C_MASTER_SCL_GPIO 14
//#define I2C_MASTER_SDA_FUNC FUNC_GPIO2
//#define I2C_MASTER_SCL_FUNC FUNC_GPIO14

#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_GPIO2_U
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_GPIO0_U
#define I2C_MASTER_SDA_GPIO 2
#define I2C_MASTER_SCL_GPIO 0
#define I2C_MASTER_SDA_FUNC FUNC_GPIO2
#define I2C_MASTER_SCL_FUNC FUNC_GPIO0

//#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_GPIO4_U
//#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_GPIO5_U
//#define I2C_MASTER_SDA_GPIO 4
//#define I2C_MASTER_SCL_GPIO 5
//#define I2C_MASTER_SDA_FUNC FUNC_GPIO4
//#define I2C_MASTER_SCL_FUNC FUNC_GPIO5

#if 0
#define I2C_MASTER_GPIO_SET(pin)  \
    gpio_output_set(1<<pin,0,1<<pin,0)

#define I2C_MASTER_GPIO_CLR(pin) \
    gpio_output_set(0,1<<pin,1<<pin,0)

#define I2C_MASTER_GPIO_OUT(pin,val) \
    if(val) I2C_MASTER_GPIO_SET(pin);\
    else I2C_MASTER_GPIO_CLR(pin)
#endif

#define I2C_MASTER_SDA_HIGH_SCL_HIGH()  \
    gpio_output_set(1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0)

#define I2C_MASTER_SDA_HIGH_SCL_LOW()  \
    gpio_output_set(1<<I2C_MASTER_SDA_GPIO, 1<<I2C_MASTER_SCL_GPIO, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0)

#define I2C_MASTER_SDA_LOW_SCL_HIGH()  \
    gpio_output_set(1<<I2C_MASTER_SCL_GPIO, 1<<I2C_MASTER_SDA_GPIO, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0)

#define I2C_MASTER_SDA_LOW_SCL_LOW()  \
    gpio_output_set(0, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0)

void i2c_master_gpio_init(void);

/* Funkcja MUSI BY� wywo�ywana osobno, wykasowano jej wywo�ywanie z i2c_master_gpio_init()
 *
 * Argumenty
 * #define BITRATE_100KHZ		0
 * #define BITRATE_200_400KHZ	1	// dla F_CPU=80MHz bitrate = 200 kHz, dla FCPU=160MHz bitrate = 400kHz
 *
 */
void i2c_master_init( uint8_t bitrate_mode );

#define i2c_master_wait    os_delay_us
void i2c_master_stop(void);
void i2c_master_start(void);
void i2c_master_setAck(uint8_t level);
uint8_t i2c_master_getAck(void);
uint8_t i2c_master_readByte(void);
void i2c_master_writeByte(uint8_t wrdata);

bool i2c_master_checkAck(void);
void i2c_master_send_ack(void);
void i2c_master_send_nack(void);

#endif
