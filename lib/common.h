#ifndef __COMMON_H__
#define __COMMON_H__

#include <avr/io.h>
#include <util/setbaud.h>

#ifndef BAUD
#error BAUD not defined
#endif
#ifndef F_CPU
#error F_CPU not defined
#endif

#define EEPROM_MODBUS_ADDR  ((0x00))
#define LED_PIN             PB5

void init_timer ();
void stop_timer ();

#endif
