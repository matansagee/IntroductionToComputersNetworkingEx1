#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>

uint32_t crc32(uint32_t crc, const void *buf, size_t size);
unsigned reverse(unsigned x);
unsigned int crc32a(unsigned char *message);
uint16_t gen_crc16(const uint8_t *data, uint16_t size);


#endif //CRC32_h