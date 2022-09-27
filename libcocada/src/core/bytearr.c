/*
 * COCADA - COCADA Collection of Algorithms and DAta Structures
 *
 * Copyright (C) 2016  Paulo G S Fonseca
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "arrays.h"
#include "bitbyte.h"
#include "bytearr.h"
#include "cstrutil.h"


byte_t *bytearr_new(size_t len)
{
	byte_t *ret;
	ret = ARR_NEW(byte_t, len);
	bytearr_fill(ret, 0, len, 0);
	return ret;
}


void bytearr_fill(byte_t *ba, size_t from, size_t to, byte_t val)
{
	memset(ba+from, val, to-from);
}


void bytearr_reverse(byte_t *src, size_t size)
{
	size_t i=0, j=size-1;
	while (i<j) {
		src[i] = src[i] ^ src[j];
		src[j] = src[i] ^ src[j];
		src[i] = src[i] ^ src[j];
		i++;
		j--;
	}
}


void bytearr_print (const byte_t *ba, size_t nbytes, byte_format fmt,
                    size_t bytes_per_line, const char *left_margin)
{
	size_t i, line_label_width;
	char *bytestr;
	line_label_width = ceil(log10(nbytes));
	bytestr = cstr_new(BYTESIZE);
	for (i=0; i<nbytes; i++) {
		if (i%bytes_per_line == 0) {
			if (i) printf("\n");
			printf("%s%*zu:", left_margin, (int)line_label_width, i);
		}
		byte_to_str(ba[i], bytestr);
		printf(" %s", bytestr);
	}
	printf("\n");
	free(bytestr);
}


char bytearr_read_char(const byte_t *src, size_t from_byte,
                       size_t nbytes)
{
	char ret=0;
#if ENDIANNESS==BIG
	if (nbytes>0 && src[from_byte]&MSBMASK(1)) {
		ret = ~ret;
	}
#elif ENDIANESS==LITTLE
	if (nbytes>0 && src[from_byte+nbytes-1]&MSBMASK(1)) {
		ret = ~ret;
	}
#endif
	bytearr_write((byte_t *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte_t *)&ret, sizeof(char));
#endif
	return ret;
}


unsigned char bytearr_read_uchar(const byte_t *src, size_t from_byte,
                                 size_t nbytes)
{
	unsigned char ret=0;
	bytearr_write((byte_t *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte_t *)&ret, sizeof(unsigned char));
#endif
	return ret;
}


short bytearr_read_short(const byte_t *src, size_t from_byte,
                         size_t nbytes)
{
	short ret=0;
#if ENDIANNESS==BIG
	if (nbytes>0 && src[from_byte]&MSBMASK(1)) {
		ret = ~ret;
	}
#elif ENDIANESS==LITTLE
	if (nbytes>0 && src[from_byte+nbytes-1]&MSBMASK(1)) {
		ret = ~ret;
	}
#endif
	bytearr_write((byte_t *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte_t *)&ret, sizeof(short));
#endif
	return ret;
}


unsigned short bytearr_read_ushort(const byte_t *src, size_t from_byte,
                                   size_t nbytes)
{
	unsigned short ret=0;
	bytearr_write((byte_t *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte_t *)&ret, sizeof(unsigned short));
#endif
	return ret;
}


int bytearr_read_int(const byte_t *src, size_t from_byte,
                     size_t nbytes)
{
	int ret=0;
#if ENDIANNESS==BIG
	if (nbytes>0 && src[from_byte]&MSBMASK(1)) {
		ret = ~ret;
	}
#elif ENDIANESS==LITTLE
	if (nbytes>0 && src[from_byte+nbytes-1]&MSBMASK(1)) {
		ret = ~ret;
	}
#endif
	bytearr_write((byte_t *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte_t *)&ret, sizeof(int));
#endif
	return ret;
}


unsigned int bytearr_read_uint(const byte_t *src, size_t from_byte,
                               size_t nbytes)
{
	unsigned int ret=0;
	bytearr_write((byte_t *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte_t *)&ret, sizeof(unsigned int));
#endif
	return ret;
}


long bytearr_read_long(const byte_t *src, size_t from_byte,
                       size_t nbytes)
{
	long ret=0;
#if ENDIANNESS==BIG
	if (nbytes>0 && src[from_byte]&MSBMASK(1)) {
		ret = ~ret;
	}
#    elif ENDIANESS==LITTLE
	if (nbytes>0 && src[from_byte+nbytes-1]&MSBMASK(1)) {
		ret = ~ret;
	}
#endif
	bytearr_write((byte_t *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte_t *)&ret, sizeof(long));
#endif
	return ret;
}


unsigned long bytearr_read_ulong(const byte_t *src, size_t from_byte,
                                 size_t nbytes)
{
	unsigned long ret=0;
	bytearr_write((byte_t *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte_t *)&ret, sizeof(unsigned long));
#endif
	return ret;
}


long long bytearr_read_llong(const byte_t *src, size_t from_byte,
                             size_t nbytes)
{
	long long ret=0;
#if ENDIANNESS==BIG
	if (nbytes>0 && src[from_byte]&MSBMASK(1)) {
		ret = ~ret;
	}
#elif ENDIANESS==LITTLE
	if (nbytes>0 && src[from_byte+nbytes-1]&MSBMASK(1)) {
		ret = ~ret;
	}
#endif
	bytearr_write((byte_t *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte_t *)&ret, sizeof(long long));
#endif
	return ret;
}


unsigned long long bytearr_read_ullong(const byte_t *src,
                                       size_t from_byte, size_t nbytes)
{
	unsigned long long ret=0;
	bytearr_write((byte_t *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte_t *)&ret, sizeof(unsigned long long));
#endif
	return ret;
}


size_t bytearr_read_size_t(const byte_t *src, size_t from_byte,
                           size_t nbytes)
{
	size_t ret=0;
	bytearr_write((byte_t *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte_t *)&ret, sizeof(size_t));
#endif
	return ret;
}


void bytearr_write(byte_t *dest, size_t from_byte_dest, const byte_t *src,
                   size_t from_byte_src, size_t nbytes)
{
	memcpy(dest+from_byte_dest, src+from_byte_src, nbytes);
}


void bytearr_write_char(byte_t *dest, size_t from_byte, char val,
                        size_t nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte_t *)&val, sizeof(char));
#endif
	bytearr_write(dest, from_byte, (byte_t *)&val, 0, nbytes);
}


void bytearr_write_uchar(byte_t *dest, size_t from_byte,
                         unsigned char val, size_t nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte_t *)&val, sizeof(unsigned char));
#endif
	bytearr_write(dest, from_byte, (byte_t *)&val, 0, nbytes);
}


void bytearr_write_short(byte_t *dest, size_t from_byte, short val,
                         size_t nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte_t *)&val, sizeof(short));
#endif
	bytearr_write(dest, from_byte, (byte_t *)&val, 0, nbytes);
}


void bytearr_write_ushort(byte_t *dest, size_t from_byte,
                          unsigned short val, size_t nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte_t *)&val, sizeof(unsigned short));
#endif
	bytearr_write(dest, from_byte, (byte_t *)&val, 0, nbytes);
}


void bytearr_write_int(byte_t *dest, size_t from_byte, int val,
                       size_t nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte_t *)&val, sizeof(int));
#endif
	bytearr_write(dest, from_byte, (byte_t *)&val, 0, nbytes);
}


void bytearr_write_uint(byte_t *dest, size_t from_byte, unsigned int val,
                        size_t nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte_t *)&val, sizeof(unsigned int));
#endif
	bytearr_write(dest, from_byte, (byte_t *)&val, 0, nbytes);
}


void bytearr_write_long(byte_t *dest, size_t from_byte, long val,
                        size_t nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte_t *)&val, sizeof(long));
#endif
	bytearr_write(dest, from_byte, (byte_t *)&val, 0, nbytes);
}


void bytearr_write_ulong(byte_t *dest, size_t from_byte,
                         unsigned long val, size_t nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte_t *)&val, sizeof(unsigned long));
#endif
	bytearr_write(dest, from_byte, (byte_t *)&val, 0, nbytes);
}


void bytearr_write_llong(byte_t *dest, size_t from_byte, long long val,
                         size_t nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte_t *)&val, sizeof(long long));
#endif
	bytearr_write(dest, from_byte, (byte_t *)&val, 0, nbytes);
}


void bytearr_write_ullong(byte_t *dest, size_t from_byte,
                          unsigned long long val, size_t nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte_t *)&val, sizeof(unsigned long long));
#endif
	bytearr_write(dest, from_byte, (byte_t *)&val, 0, nbytes);
}


void bytearr_write_size_t(byte_t *dest, size_t from_byte, size_t val,
                          size_t nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte_t *)&val, sizeof(size_t));
#endif
	bytearr_write(dest, from_byte, (byte_t *)&val, 0, nbytes);
}
