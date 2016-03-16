
#ifndef UTILS_H
#define UTILS_H

#include<stdint.h>

char* ConcatString(	char* source_a, char* source_b,char* source_c);
unsigned char* serialize_uint32_t(unsigned char *buffer, uint32_t value);
unsigned char* serialize_uint16_t(unsigned char *buffer, uint16_t value);

#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )
#endif