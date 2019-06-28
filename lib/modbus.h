#ifndef __MODBUS_H__
#define __MODBUS_H__

#include <stdio.h>
#include <stdlib.h>

#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "crc16.h"
#include "common.h"

#ifndef MY_MAGIC
    #error "MY_MAGIC must be defined!"
#endif

#define MODBUS_RXTX_PIN     PD2

/* MODBUS Protocol Defines */
#define MODBUS_EX_ILLEGAL_FUNCTION      0x01
#define MODBUS_EX_ILLEGAL_DATA_ADDRESS  0x02
#define MODBUS_EX_ILLEGAL_DATA_VALUE    0x03
#define MODBUS_EX_SLAVE_DEVICE_FAILURE  0x04

#define MODBUS_READ_COIL_CMD            0x01
#define MODBUS_READ_REG_CMD             0x03
#define MODBUS_WRITE_COIL_CMD           0x05
#define MODBUS_WRITE_REG_CMD            0x06
#define MODBUS_WRITE_DATA_CMD           0x10

/* standard register offsets and magic values */
#define MB_RD_IDENT         0x00
#define MB_WR_RESET         0xFF
#define MB_WR_RESET_MAGIC   0x42


uint8_t modbus_addr;

void modbus_init();
uint8_t modbus_recv(uint8_t* buf);
uint8_t modbus_poll(uint8_t* buf, uint16_t magic);
void send_modbus_array(uint8_t* msg, uint8_t len);
void send_modbus_response(uint8_t* msg, uint8_t len, uint8_t resp);

#endif
