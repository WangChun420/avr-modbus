/*
 * Modbus handling code
 */

#include "modbus.h"

void modbus_init()
{
    // init UART to BAUD rate
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    UCSR0C = (1<<UPM01) | (1<<UCSZ01) | (1<<UCSZ00); // even Parity, 8 Bits
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);

    // init RXTX pin as output
    DDRD |= (1<<MODBUS_RXTX_PIN);
    PORTD &= ~(1<<MODBUS_RXTX_PIN);

    // my addr
    modbus_addr = eeprom_read_byte((uint8_t*)EEPROM_MODBUS_ADDR);
    if (modbus_addr == 0xFF) {
        modbus_addr = 247;
    }
}

void modbus_stop()
{
    // stop UART
    UCSR0B = 0;
    UCSR0C = 0;

    // RXTX pin off, input
    PORTD &= ~(1<<MODBUS_RXTX_PIN);
    DDRD &= ~(1<<MODBUS_RXTX_PIN);
}

uint8_t modbus_recv(uint8_t* buf)
{
    static uint8_t recv_status = 0xF0;
    static uint8_t sub_recv_status = 0;
    static uint8_t recv_cnt;
    if (!(UCSR0A & (1<<RXC0))) { return 0; }
    // save current char
    uint8_t c = UDR0;
    switch (recv_status) {
        case 0xF0: // new message start
            recv_cnt = 0;
            if (c == modbus_addr || c == 0x00) {
                buf[recv_cnt++] = c;
                recv_status++;
            }
            break;
        case 0xF1: // command
            switch (c) {
                case MODBUS_READ_REG_CMD:
                case MODBUS_WRITE_REG_CMD:
                case MODBUS_WRITE_DATA_CMD:
                    recv_status = c;
                    sub_recv_status = 0;
                    buf[recv_cnt++] = c;
                    break;
                default:
                    recv_status = 0xF0;
            }
            break;
        case 0xFF: // CRC2 + check
            if (sub_recv_status == 0) {
                buf[recv_cnt++] = c;
                sub_recv_status++;
            } else {
                buf[recv_cnt] = c;
                recv_status = 0xF0;
                if (clcCRC16(buf, recv_cnt-1) == (buf[recv_cnt-1] | (buf[recv_cnt]<<8))) {
                    // CRC ok - return messeage length
                    return recv_cnt;
                }
            }
            break;

        /* MODBUS command IDs from here on */
        case MODBUS_READ_COIL_CMD:
        case MODBUS_READ_REG_CMD:
        case MODBUS_WRITE_COIL_CMD:
        case MODBUS_WRITE_REG_CMD:
            buf[recv_cnt++] = c;
            if (sub_recv_status == 3) {
                sub_recv_status = 0;
                recv_status = 0xFF;
                break;
            }
            sub_recv_status++;
            break;

        case MODBUS_WRITE_DATA_CMD:
            buf[recv_cnt++] = c;
            if (sub_recv_status > 4) { // receiving data
                if (sub_recv_status == 4 + buf[6]) { // data done
                    sub_recv_status = 0;
                    recv_status = 0xFF;
                    break;
                }
            }
            sub_recv_status++;
            break;

       default:
            recv_status = 0xF0;
    }
    return 0;
}

/* wrapper for modbus_recv
 *   calls modbus_recv
 *   handles messages every firmware must implement
 */
uint8_t modbus_poll(uint8_t* msg, uint16_t magic)
{
    uint8_t ret = modbus_recv(msg);
    if (ret > 0) {
        // MSB of start address must be zero always for simple commands
        // except for write register (used for EEPROM writes)
        if (msg[2] != 0 && msg[1] < 0x06) {
            send_modbus_response(msg, ret, MODBUS_EX_ILLEGAL_DATA_ADDRESS);
            return 0;
        }
        // only single reads/writes supported for all commands
        if ((msg[4] != 0 || msg[5] != 1) && msg[1] < 0x05) {
            send_modbus_response(msg, ret, MODBUS_EX_ILLEGAL_DATA_ADDRESS);
            return 0;
        }
        switch (msg[1]) { // command

            /* read coil cmd */
            case MODBUS_READ_COIL_CMD:
                break;

            /* read register
             *   0x00: get identification
             */
            case MODBUS_READ_REG_CMD:
                switch (msg[3]) {
                    case 0x00:
                        msg[2] = 2;
                        msg[3] = magic >> 8;
                        msg[4] = (uint8_t)magic;
                        send_modbus_response(msg, 3, 0);
                        return 0;
                }
                break;

            /* write single coil
             */
            case MODBUS_WRITE_COIL_CMD:
                break;


            /* write register
             *   0x8000 ... : write to EEPROM
             */
            case MODBUS_WRITE_REG_CMD:
                // update eeprom values
                // 0x8000 = EEPROM base
                // 0x8001 = EERPOM base + 1
                if (msg[2] >= 0x80) {
                    uint16_t addr = ((uint16_t)(msg[2] & 0x03) << 8) | msg[3];
                    eeprom_write_byte((uint8_t*)addr, msg[5]);
                    send_modbus_response(msg, 4, 0);
                    return 0;
                } else if (msg[2] != 0) {
                    send_modbus_response(msg, 0, MODBUS_EX_ILLEGAL_DATA_ADDRESS);
                    return 0;
                }
                break;
        }
    }
    return ret;
}

void send_modbus_array(uint8_t* msg, uint8_t len)
{
    /* inter frame gap */
    PORTD |= (1<<MODBUS_RXTX_PIN);
    for (uint8_t i=0; i<len; i++) {
        while (!(UCSR0A & (1<<UDRE0))) {}
        UDR0 = msg[i];
    }
    while (!(UCSR0A & (1<<UDRE0))) {}
    PORTD &= ~(1<<MODBUS_RXTX_PIN);
    TCNT0 = 0;
}

void send_modbus_response(uint8_t* msg, uint8_t len, uint8_t resp)
{
    if (msg[0] == 0x00) { return; }
    if (resp != 0) {
        uint8_t buf[3] = { msg[0], 0x80 + msg[1], resp };
        send_modbus_array(buf, 3);
    } else {
        uint16_t crc = clcCRC16(msg, len + 2);
        msg[len+2] = crc;
        msg[len+3] = crc >> 8;
        send_modbus_array(msg, len + 4);
    }
}
