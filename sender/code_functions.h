#ifndef CRC32_H
#define CRC32_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>

typedef unsigned char      byte;    // Byte is a char
typedef unsigned short int uint16_t;  // 16-bit word is a short int
typedef unsigned int       word32;  // 32-bit word is an int



unsigned reverse(unsigned x);
uint32_t crc32a(unsigned char *message);
uint16_t gen_crc16(const uint8_t *data, uint16_t size);
uint16_t checksum(byte *addr, word32 count);

#endif //CRC32_h