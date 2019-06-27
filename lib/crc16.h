#ifndef __CRC16_H__
#define __CRC16_H__

/*
  crc16.c (CRC16 various computation routines)
*/

#include <avr/io.h>
#include <avr/pgmspace.h>

uint16_t getCRC16(const uint8_t *buf, const uint8_t bufsize);
uint16_t memCRC16(const uint8_t *buf, const uint8_t bufsize);
uint16_t asuCRC16(const uint8_t b8, const uint16_t crc16);
void updCRC16(const uint8_t b8, uint16_t *crc16);
uint16_t clcCRC16(const uint8_t *buf, const uint8_t bufsize);

#endif
