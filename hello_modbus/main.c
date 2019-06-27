#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "modbus.h"
#include "common.h"

void (*bootloader)( void ) = BOOTLOADER;

int main (void)
{
//    while(1) { ; }
    cli();
    modbus_init();
    init_timer();

    // set led as output
    DDRB |= (1<<LED_PIN);

    uint16_t len;
    uint8_t buf[16];
    uint8_t ret = 0;
    uint8_t state = 0;

    while (1) {
        wdt_reset();
        if (state == 255) {
            _delay_ms(10);
            bootloader();
        }
        len = modbus_poll(buf);
        ret = 0;
        if (len) {
            switch (buf[1]) {

                /* read register
                 *
                 * 0x01: Pagesize
                 * 0x02: Flashsize
                 */
                case MODBUS_READ_REG_CMD:
                    // fill in response
                    buf[2] = 2;
                    switch (buf[3]) {
                        case 0x01:
                            buf[3] = 0x42;
                            buf[4] = 0x24;
                            break;
                        default:
                            ret = MODBUS_EX_ILLEGAL_DATA_ADDRESS;
                    }
                    len = 7;
                    break;

                case MODBUS_WRITE_REG_CMD:
                    switch (buf[3]) {
                        case 0x01: // toggle led
                            PORTB ^= (1<<LED_PIN);
                            break;
                        case MB_WR_RESET:
                            if (buf[5] == MB_WR_RESET_MAGIC) {
                               state = 255;
                            } else {
                                ret = MODBUS_EX_ILLEGAL_DATA_VALUE;
                            }
                            break;
                        default:
                            ret = MODBUS_EX_ILLEGAL_DATA_ADDRESS;
                    }
                    len = 8;
                    break;

                default:
                    ret = MODBUS_EX_ILLEGAL_FUNCTION;
            }
            send_modbus_response(buf, len, ret);
        }
    }
}
