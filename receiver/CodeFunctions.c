#include "CodeFunctions.h"
//credit is due to hackersdelight.org
// ---------------------------- reverse --------------------------------

// Reverses (reflects) bits in a 32-bit word.
unsigned reverse(unsigned x) {
	x = ((x & 0x55555555) << 1) | ((x >> 1) & 0x55555555);
	x = ((x & 0x33333333) << 2) | ((x >> 2) & 0x33333333);
	x = ((x & 0x0F0F0F0F) << 4) | ((x >> 4) & 0x0F0F0F0F);
	x = (x << 24) | ((x & 0xFF00) << 8) |
		((x >> 8) & 0xFF00) | (x >> 24);
	return x;
}
//Credit is due to hackersdelight.org
// ----------------------------- crc32a --------------------------------

/* This is the basic CRC algorithm with no optimizations. It follows the
logic circuit as closely as possible. */

uint32_t crc32a(unsigned char *message) {
	int i, j;
	unsigned int byte, crc;

	i = 0;
	crc = 0xFFFFFFFF;
	while (message[i] != 0) {
		byte = message[i];            // Get next byte.
		byte = reverse(byte);         // 32-bit reversal.
		for (j = 0; j <= 7; j++) {    // Do eight times.
			if ((int)(crc ^ byte) < 0)
				crc = (crc << 1) ^ 0x04C11DB7;
			else crc = crc << 1;
			byte = byte << 1;          // Ready next msg bit.
		}
		i = i + 1;
	}
	return reverse(~crc);
}


#define CRC16 0x8005
//Credit to Michael Burr from stackoverflow
uint16_t gen_crc16(const uint8_t *data, uint16_t size)
{
	uint16_t out = 0;
	int bits_read = 0, bit_flag;

	/* Sanity check: */
	if (data == NULL)
		return 0;

	while (size > 0)
	{
		bit_flag = out >> 15;

		/* Get next bit: */
		out <<= 1;
		out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

		/* Increment bit counter: */
		bits_read++;
		if (bits_read > 7)
		{
			bits_read = 0;
			data++;
			size--;
		}

		/* Cycle check: */
		if (bit_flag)
			out ^= CRC16;

	}

	// item b) "push out" the last 16 bits
	int i;
	for (i = 0; i < 16; ++i) {
		bit_flag = out >> 15;
		out <<= 1;
		if (bit_flag)
			out ^= CRC16;
	}

	// item c) reverse the bits
	uint16_t crc = 0;
	i = 0x8000;
	int j = 0x0001;
	for (; i != 0; i >>= 1, j <<= 1) {
		if (i & out) crc |= j;
	}

	return crc;
}

word16 checksum(byte *addr, word32 count)
{
	register word32 sum = 0;
	//check if count is even, if not add 0x00
	if (count % 2 != 0){

	}
	// Main summing loop
	while (count > 1)
	{
		sum = sum + *((word16 *)addr)++;
		count = count - 2;
	}

	// Add left-over byte, if any
	if (count > 0)
		sum = sum + *((byte *)addr);

	// Fold 32-bit sum to 16 bits
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return(~sum);
}