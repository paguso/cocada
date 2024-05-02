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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arrays.h"
#include "bitbyte.h"
#include "bytearr.h"
#include "cstrutil.h"


byte *bytearr_new(usize len)
{
	byte *ret;
	ret = ARR_NEW(byte, len);
	bytearr_fill(ret, 0, len, 0);
	return ret;
}


void bytearr_fill(byte *ba, usize from, usize to, byte val)
{
	memset(ba + from, val, to - from);
}


void bytearr_reverse(byte *src, usize size)
{
	usize i = 0, j = size - 1;
	while (i < j) {
		src[i] = src[i] ^ src[j];
		src[j] = src[i] ^ src[j];
		src[i] = src[i] ^ src[j];
		i++;
		j--;
	}
}


void bytearr_print (const byte *ba, usize nbytes, byte_format fmt,
                    usize bytes_per_line, const char *left_margin)
{
	usize i, line_label_width;
	char *bytestr;
	line_label_width = ceil(log10(nbytes));
	bytestr = cstr_new(BYTESIZE);
	for (i = 0; i < nbytes; i++) {
		if (i % bytes_per_line == 0) {
			if (i) printf("\n");
			printf("%s%*zu:", left_margin, (int)line_label_width, i);
		}
		byteo_str(ba[i], bytestr);
		printf(" %s", bytestr);
	}
	printf("\n");
	free(bytestr);
}


char bytearr_read_char(const byte *src, usize from_byte,
                       usize nbytes)
{
	char ret = 0;
#if ENDIANNESS==BIG
	if (nbytes > 0 && src[from_byte]&MSBMASK(1)) {
		ret = ~ret;
	}
#elif ENDIANESS==LITTLE
	if (nbytes > 0 && src[from_byte + nbytes - 1]&MSBMASK(1)) {
		ret = ~ret;
	}
#endif
	bytearr_write((byte *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte *)&ret, sizeof(char));
#endif
	return ret;
}


unsigned char bytearr_read_uchar(const byte *src, usize from_byte,
                                 usize nbytes)
{
	unsigned char ret = 0;
	bytearr_write((byte *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte *)&ret, sizeof(unsigned char));
#endif
	return ret;
}


short bytearr_read_short(const byte *src, usize from_byte,
                         usize nbytes)
{
	short ret = 0;
#if ENDIANNESS==BIG
	if (nbytes > 0 && src[from_byte]&MSBMASK(1)) {
		ret = ~ret;
	}
#elif ENDIANESS==LITTLE
	if (nbytes > 0 && src[from_byte + nbytes - 1]&MSBMASK(1)) {
		ret = ~ret;
	}
#endif
	bytearr_write((byte *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte *)&ret, sizeof(short));
#endif
	return ret;
}


unsigned short bytearr_read_ushort(const byte *src, usize from_byte,
                                   usize nbytes)
{
	unsigned short ret = 0;
	bytearr_write((byte *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte *)&ret, sizeof(unsigned short));
#endif
	return ret;
}


int bytearr_read_int(const byte *src, usize from_byte,
                     usize nbytes)
{
	int ret = 0;
#if ENDIANNESS==BIG
	if (nbytes > 0 && src[from_byte]&MSBMASK(1)) {
		ret = ~ret;
	}
#elif ENDIANESS==LITTLE
	if (nbytes > 0 && src[from_byte + nbytes - 1]&MSBMASK(1)) {
		ret = ~ret;
	}
#endif
	bytearr_write((byte *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte *)&ret, sizeof(int));
#endif
	return ret;
}


unsigned int bytearr_read_uint(const byte *src, usize from_byte,
                               usize nbytes)
{
	unsigned int ret = 0;
	bytearr_write((byte *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte *)&ret, sizeof(unsigned int));
#endif
	return ret;
}


long bytearr_read_long(const byte *src, usize from_byte,
                       usize nbytes)
{
	long ret = 0;
#if ENDIANNESS==BIG
	if (nbytes > 0 && src[from_byte]&MSBMASK(1)) {
		ret = ~ret;
	}
#    elif ENDIANESS==LITTLE
	if (nbytes > 0 && src[from_byte + nbytes - 1]&MSBMASK(1)) {
		ret = ~ret;
	}
#endif
	bytearr_write((byte *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte *)&ret, sizeof(long));
#endif
	return ret;
}


unsigned long bytearr_read_ulong(const byte *src, usize from_byte,
                                 usize nbytes)
{
	unsigned long ret = 0;
	bytearr_write((byte *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte *)&ret, sizeof(unsigned long));
#endif
	return ret;
}


long long bytearr_read_llong(const byte *src, usize from_byte,
                             usize nbytes)
{
	long long ret = 0;
#if ENDIANNESS==BIG
	if (nbytes > 0 && src[from_byte]&MSBMASK(1)) {
		ret = ~ret;
	}
#elif ENDIANESS==LITTLE
	if (nbytes > 0 && src[from_byte + nbytes - 1]&MSBMASK(1)) {
		ret = ~ret;
	}
#endif
	bytearr_write((byte *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte *)&ret, sizeof(long long));
#endif
	return ret;
}


unsigned long long bytearr_read_ullong(const byte *src,
                                       usize from_byte, usize nbytes)
{
	unsigned long long ret = 0;
	bytearr_write((byte *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte *)&ret, sizeof(unsigned long long));
#endif
	return ret;
}


usize bytearr_read_usize(const byte *src, usize from_byte,
                         usize nbytes)
{
	usize ret = 0;
	bytearr_write((byte *)&ret, 0, src, from_byte, nbytes);
#if ENDIANNESS==BIG
	bytearr_reverse((byte *)&ret, sizeof(usize));
#endif
	return ret;
}


void bytearr_write(byte *dest, usize from_byte_dest, const byte *src,
                   usize from_byte_src, usize nbytes)
{
	memcpy(dest + from_byte_dest, src + from_byte_src, nbytes);
}


void bytearr_write_char(byte *dest, usize from_byte, char val,
                        usize nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte *)&val, sizeof(char));
#endif
	bytearr_write(dest, from_byte, (byte *)&val, 0, nbytes);
}


void bytearr_write_uchar(byte *dest, usize from_byte,
                         unsigned char val, usize nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte *)&val, sizeof(unsigned char));
#endif
	bytearr_write(dest, from_byte, (byte *)&val, 0, nbytes);
}


void bytearr_write_short(byte *dest, usize from_byte, short val,
                         usize nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte *)&val, sizeof(short));
#endif
	bytearr_write(dest, from_byte, (byte *)&val, 0, nbytes);
}


void bytearr_write_ushort(byte *dest, usize from_byte,
                          unsigned short val, usize nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte *)&val, sizeof(unsigned short));
#endif
	bytearr_write(dest, from_byte, (byte *)&val, 0, nbytes);
}


void bytearr_write_int(byte *dest, usize from_byte, int val,
                       usize nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte *)&val, sizeof(int));
#endif
	bytearr_write(dest, from_byte, (byte *)&val, 0, nbytes);
}


void bytearr_write_uint(byte *dest, usize from_byte, unsigned int val,
                        usize nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte *)&val, sizeof(unsigned int));
#endif
	bytearr_write(dest, from_byte, (byte *)&val, 0, nbytes);
}


void bytearr_write_long(byte *dest, usize from_byte, long val,
                        usize nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte *)&val, sizeof(long));
#endif
	bytearr_write(dest, from_byte, (byte *)&val, 0, nbytes);
}


void bytearr_write_ulong(byte *dest, usize from_byte,
                         unsigned long val, usize nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte *)&val, sizeof(unsigned long));
#endif
	bytearr_write(dest, from_byte, (byte *)&val, 0, nbytes);
}


void bytearr_write_llong(byte *dest, usize from_byte, long long val,
                         usize nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte *)&val, sizeof(long long));
#endif
	bytearr_write(dest, from_byte, (byte *)&val, 0, nbytes);
}


void bytearr_write_ullong(byte *dest, usize from_byte,
                          unsigned long long val, usize nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte *)&val, sizeof(unsigned long long));
#endif
	bytearr_write(dest, from_byte, (byte *)&val, 0, nbytes);
}


void bytearr_write_usize(byte *dest, usize from_byte, usize val,
                         usize nbytes)
{
#if ENDIANNESS == BIG
	bytearr_reverse((byte *)&val, sizeof(usize));
#endif
	bytearr_write(dest, from_byte, (byte *)&val, 0, nbytes);
}
