#ifndef __MODBUS_HPP__
#define __MODBUS_HPP__

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <vector>
#include <modbus/modbus-rtu.h>

modbus_t* my_modbus_connect(char* tty, int baud);
int my_modbus_read_reg(modbus_t *ctx, uint8_t addr, uint16_t reg);
int my_modbus_write_reg(modbus_t *ctx, uint8_t addr, uint16_t reg, uint16_t val);
int my_modbus_check_identification(modbus_t *ctx, uint8_t addr, uint16_t ident);

#endif
