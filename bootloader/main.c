#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "modbus.h"
#include "common.h"

void do_reset()
{
    DDRB &= ~(1<<LED_PIN);
    asm("jmp 0");
}

int main (void)
{
    MCUSR = 0;
    wdt_enable(WDTO_1S);
    cli();
    modbus_init();
    init_timer();

    // set led as output
    DDRB |= (1<<LED_PIN);

    uint16_t start, len;
    uint8_t ret = 0;
    uint8_t buf[136];
    uint8_t state = 0;

    while (1) {
        wdt_reset();
        // timeout or reset request = jump to app
        // MUST read TCNT1L before TCNT1H !!
        ret = TCNT1L;
        if (TCNT1H > 0x10 || state == 255) {
            _delay_ms(10);
            do_reset();
        }
        // fake watchdog
        if (state > 0) {
            TCNT1H = 0;
            TCNT1L = 0;
        }
        ret = 0;
        len = modbus_poll(buf, MY_MAGIC);
        if (len) {
            switch (buf[1]) {

                /* read register
                 * 0x01: Pagesize
                 * 0x02: Flashsize
                 */
                case MODBUS_READ_REG_CMD:
                    // fill in response
                    buf[2] = 2;
                    switch (buf[3]) {
                        case 0x01: // devices pagesize
                            buf[3] = SPM_PAGESIZE >> 8;
                            buf[4] = SPM_PAGESIZE;
                            break;
                        case 0x02: // devices flash size
                            buf[3] = FLASHEND >> 8;
                            buf[4] = (uint8_t)FLASHEND;
                            break;
                        default:
                            ret = MODBUS_EX_ILLEGAL_DATA_ADDRESS;
                    }
                    len = 3;
                    break;

                /* write register
                 * 0x00: prevent from reset (magic value 0x42
                 * 0xFF: reset
                 */
                case MODBUS_WRITE_REG_CMD:
                    switch (buf[3]) {
                        case 0x00:
                            if (buf[5] == 0x42) { // prevent reset
                                state = 1;
                            } else {
                                ret = MODBUS_EX_ILLEGAL_DATA_VALUE;
                            }
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
                    len = 4;
                    break;

                case MODBUS_WRITE_DATA_CMD:
                    // sanity checks
                    // length must be one page
                    // start address is in bytes
                    // address must be before bootloader
                    start = (buf[3] | ((uint16_t)buf[2] << 8)) * 2;
                    if (buf[6] != 128 || start > FLASHEND-2048-128) {
                        ret = MODBUS_EX_ILLEGAL_DATA_ADDRESS;
                        break;
                    }

                    boot_page_erase(start);
                    boot_spm_busy_wait();
                    for (uint8_t i=0; i<SPM_PAGESIZE; i+=2) {
                        uint16_t w = buf[7+i] << 8;
                        w |= buf[7+i+1];
                        boot_page_fill(start + i, w);
                    }
                    boot_page_write(start);
                    boot_spm_busy_wait();
                    boot_rww_enable();

                    // fill in response
                    len = 4;
                    break;

                default:
                    ret = MODBUS_EX_ILLEGAL_FUNCTION;
            }
            send_modbus_response(buf, len, ret);
        }
    }
}
