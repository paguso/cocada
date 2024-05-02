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

#include "arrays.h"
#include "bitarr.h"
#include "bitbyte.h"
#include "bytearr.h"
#include "cstrutil.h"
#include "new.h"
#include "mathutil.h"

byte *bitarr_new(size_t len)
{
	return bytearr_new((size_t)DIVCEIL(len, BYTESIZE));
}


byte *bitarr_new_from_str(const char *str, size_t len)
{
	byte *ret;
	ret = ARR_NEW(byte, (size_t)DIVCEIL(len, BYTESIZE));
	bitarr_parse_str(ret, str, len);
	return ret;
}


void bitarr_parse_str(byte *dest, const char *src, size_t len)
{
	byte bt;
	size_t i = (size_t)DIVCEIL(len, BYTESIZE);
	i = 0;
	while (i + BYTESIZE <= len) {
		bt = 0x0;
		for (size_t j = 0; j < BYTESIZE; j++) {
			bt <<= 1;
			bt += (src[i + j] == '1') ? 0x1 : 0x0;
		}
		dest[i / BYTESIZE] = bt;
		i += BYTESIZE;
	}
	if (len % BYTESIZE) {
		bt = 0x0;
		for (size_t j = 0; j < len % BYTESIZE; j++) {
			bt <<= 1;
			bt += (src[i + j] == '1') ? 0x1 : 0x0;
		}
		dest[i / BYTESIZE] = ( bt << (BYTESIZE - (len % BYTESIZE)));
	}
}


inline bool bitarr_get_bit (const byte *ba, size_t pos)
{
	return ba[pos / BYTESIZE] & BITMASK(pos % BYTESIZE);
}

#define BYTE_MSB  0x80

inline void bitarr_set_bit (byte *ba, size_t pos, const bool bit)
{
	ba[pos / BYTESIZE] ^= ( ((-bit) ^ (ba[pos / BYTESIZE]))
	                        & (BYTE_MSB >> (pos % BYTESIZE)) );
	//if (bit)
	//    ba[pos/BYTESIZE] |= BITMASK(pos%BYTESIZE);
	//else
	//    ba[pos/BYTESIZE] &= ~BITMASK(pos%BYTESIZE);
}


#define BITARR_PRINT(TYPE)\
	int ret = 0;\
	size_t i, c, line_label_width, bits_per_line;\
	byte b, onemask;\
	line_label_width = indent + ceil(log10(nbits));\
	bits_per_line = bytes_per_line * BYTESIZE;\
	onemask = 1<<(BYTESIZE-1);\
	i = 0;\
	while (i < nbits) {\
		if (i%bits_per_line == 0) {\
			if (i) {\
				ret += TYPE##printf(out, "\n");\
			}\
			ret += TYPE##printf(out, "%*zu:", (int)line_label_width, i);\
		}\
		ret += TYPE##printf(out, " ");\
		b = ba[i/BYTESIZE];\
		/*printf ("%x ",b);*/\
		for (c=0; c<BYTESIZE; c++) {\
			if (i+c < nbits) {\
				if (b & onemask) {\
					ret += TYPE##printf(out, "1");\
				}\
				else {\
					ret += TYPE##printf(out, "0");\
				}\
			}\
			else {\
				ret += TYPE##printf(out, "*");\
			}\
			b <<= 1;\
		}\
		i += BYTESIZE;\
	}\
	ret += TYPE##printf(out, "\n");\
	return ret;



int bitarr_fprint(FILE *out, const byte *ba, size_t nbits,
                  uint bytes_per_line, uint indent)
{
	BITARR_PRINT(f);
}

int bitarr_sprint(char *out, const byte *ba, size_t nbits,
                  uint bytes_per_line, uint indent)
{
	BITARR_PRINT(s);
}

int bitarr_sbprint(StrBuf *out, const byte *ba, size_t nbits,
                   uint bytes_per_line, uint indent)
{
	BITARR_PRINT(sb);
}





/*
void bitarr_fprint_as_size_t(const byte *ba, size_t nbits,
                            size_t bits_per_entry)
{
	size_t i;
	printf ("[");
	for (i=0; i<nbits; i+=bits_per_entry) {
		printf ("%zi%s", bitarr_read_size_t(ba, i, bits_per_entry),
		        (i+bits_per_entry<nbits)?", ":"");
	}
	printf ("]\n");
}
*/


void bitarr_and(byte *ba, const byte *mask, size_t nbits)
{
	for (size_t i = 0; i < (nbits / BYTESIZE); i++) {
		ba[i] &= mask[i];
	}
	if (nbits % BYTESIZE) {
		ba[(nbits / BYTESIZE)] &= (LSBMASK(BYTESIZE - (nbits % BYTESIZE)) |
		                           mask[nbits / BYTESIZE]);
	}
}


void bitarr_or(byte *ba, const byte *mask, size_t nbits)
{
	for (size_t i = 0; i < (nbits / BYTESIZE); i++) {
		ba[i] |= mask[i];
	}
	if (nbits % BYTESIZE) {
		ba[(nbits / BYTESIZE)] |= (MSBMASK(nbits % BYTESIZE) & mask[nbits / BYTESIZE]);
	}
}


void bitarr_not(byte *ba, size_t nbits)
{
	for (size_t i = 0; i < (nbits / BYTESIZE); i++) {
		ba[i] = ~ba[i] ;
	}
	if (nbits % BYTESIZE) {
		ba[(nbits / BYTESIZE)] =
		    (~ba[(nbits / BYTESIZE)] & MSBMASK(nbits % BYTESIZE))
		    | (ba[(nbits / BYTESIZE)] & ~MSBMASK(nbits % BYTESIZE));
	}
}


char bitarr_read_char(const byte *src, size_t from_bit,
                      size_t nbits)
{
	char ret = 0;
	if (nbits > 0 && bitarr_get_bit(src, from_bit)) {
		ret = ~ret;
	}
	bitarr_write( (byte *)&ret, BYTESIZE * sizeof(char) - nbits, src, from_bit,
	              nbits );
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&ret, sizeof(char));
	}
	return ret;
}


unsigned char bitarr_read_uchar(const byte *src, size_t from_bit,
                                size_t nbits)
{
	unsigned char ret = 0;
	bitarr_write((byte *)&ret, BYTESIZE * sizeof(unsigned char) - nbits, src,
	             from_bit, nbits);
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&ret, sizeof(unsigned char));
	}
	return ret;
}


short bitarr_read_short(const byte *src, size_t from_bit,
                        size_t nbits)
{
	short ret = 0;
	if (nbits > 0 && bitarr_get_bit(src, from_bit)) {
		ret = ~ret;
	}
	bitarr_write((byte *)&ret, BYTESIZE * sizeof(short) - nbits, src, from_bit,
	             nbits);
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&ret, sizeof(short));
	}
	return ret;
}


unsigned short bitarr_read_ushort(const byte *src, size_t from_bit,
                                  size_t nbits)
{
	unsigned short ret = 0;
	bitarr_write((byte *)&ret, BYTESIZE * sizeof(unsigned short) - nbits, src,
	             from_bit, nbits);
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&ret, sizeof(unsigned short));
	}
	return ret;
}


int bitarr_read_int(const byte *src, size_t from_bit,
                    size_t nbits)
{
	int ret = 0;
	if (nbits > 0 && bitarr_get_bit(src, from_bit)) {
		ret = ~ret;
	}
	bitarr_write((byte *)&ret, BYTESIZE * sizeof(int) - nbits, src, from_bit,
	             nbits);
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&ret, sizeof(int));
	}
	return ret;
}


unsigned int bitarr_read_uint(const byte *src, size_t from_bit,
                              size_t nbits)
{
	unsigned int ret = 0;
	bitarr_write( (byte *)&ret, BYTESIZE * sizeof(unsigned int) - nbits, src,
	              from_bit, nbits );
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&ret, sizeof(unsigned int));
	}
	return ret;
}


long bitarr_read_long(const byte *src, size_t from_bit,
                      size_t nbits)
{
	long ret = 0;
	if (nbits > 0 && bitarr_get_bit(src, from_bit)) {
		ret = ~ret;
	}
	bitarr_write( (byte *)&ret, BYTESIZE * sizeof(long) - nbits, src, from_bit,
	              nbits );
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&ret, sizeof(long));
	}
	return ret;
}


unsigned long bitarr_read_ulong(const byte *src, size_t from_bit,
                                size_t nbits)
{
	unsigned long ret = 0;
	bitarr_write((byte *)&ret, BYTESIZE * sizeof(unsigned long) - nbits, src,
	             from_bit, nbits);
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&ret, sizeof(unsigned long));
	}
	return ret;
}


long long bitarr_read_llong(const byte *src, size_t from_bit,
                            size_t nbits)
{
	long long ret = 0;
	if (nbits > 0 && bitarr_get_bit(src, from_bit)) {
		ret = ~ret;
	}
	bitarr_write( (byte *)&ret, BYTESIZE * sizeof(long long) - nbits, src,
	              from_bit, nbits );
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&ret, sizeof(long long));
	}
	return ret;
}


unsigned long long bitarr_read_ullong(const byte *src,
                                      size_t from_bit, size_t nbits)
{
	unsigned long long ret = 0;
	bitarr_write((byte *)&ret, BYTESIZE * sizeof(unsigned long long) - nbits, src,
	             from_bit, nbits);
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&ret, sizeof(unsigned long long));
	}
	return ret;
}


size_t bitarr_read_size_t(const byte *src, size_t from_bit,
                          size_t nbits)
{
	size_t ret = 0;
	bitarr_write((byte *)&ret, BYTESIZE * sizeof(size_t) - nbits, src, from_bit,
	             nbits);
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&ret, sizeof(size_t));
	}
	return ret;
}


byte bitarr_read_byte(const byte *src, size_t from_bit,
                      size_t nbits)
{
	byte ret = 0;
	bitarr_write((byte *)&ret, BYTESIZE * sizeof(byte) - nbits, src, from_bit,
	             nbits);
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&ret, sizeof(byte));
	}
	return ret;
}


void bitarr_write(byte *dest, size_t from_bit_dest, const byte *src,
                  size_t from_bit_src, size_t nbits)
{
	size_t curr_byte_src, curr_byte_dest, last_byte_src;
	size_t loff_src, loff_dest, hang, overlap, last_byte_content;

	if (nbits <= 0) return;

	curr_byte_src = from_bit_src / BYTESIZE;
	last_byte_src = ((from_bit_src + nbits - 1) / BYTESIZE);
	loff_src = from_bit_src % BYTESIZE;
	curr_byte_dest = from_bit_dest / BYTESIZE;
	loff_dest = from_bit_dest % BYTESIZE;

	if (curr_byte_src != last_byte_src) {
		if (loff_src < loff_dest) {
			hang = loff_dest - loff_src;
			overlap = BYTESIZE - hang;
			// first byte
			dest[curr_byte_dest] &= MSBMASK(loff_dest);
			dest[curr_byte_dest] |= ( (src[curr_byte_src] &
			                           LSBMASK(BYTESIZE - loff_src)) >> hang );
			dest[curr_byte_dest + 1] &= LSBMASK(overlap);
			dest[curr_byte_dest + 1] |= (src[curr_byte_src] << overlap);
			curr_byte_src++;
			curr_byte_dest++;
			// 2nd to 2nd to last byte
			while (curr_byte_src != last_byte_src) {
				dest[curr_byte_dest] &= MSBMASK(hang);
				dest[curr_byte_dest] |= (src[curr_byte_src] >> hang);
				dest[curr_byte_dest + 1] &= LSBMASK(overlap);
				dest[curr_byte_dest + 1] |= (src[curr_byte_src] << overlap);
				curr_byte_src++;
				curr_byte_dest++;
			}
			// last byte
			last_byte_content = ((from_bit_src + nbits - 1) % BYTESIZE) + 1;
			if (last_byte_content <= overlap) {
				dest[curr_byte_dest] &= ~( LSBMASK(last_byte_content) <<
				                           (BYTESIZE - hang - last_byte_content) );
				dest[curr_byte_dest] |= ( (src[curr_byte_src] &
				                           MSBMASK(last_byte_content)) >> hang );
			}
			else {
				dest[curr_byte_dest] &= MSBMASK(hang);
				dest[curr_byte_dest] |= src[curr_byte_src] >> hang;
				dest[curr_byte_dest + 1] &= LSBMASK(BYTESIZE + overlap
				                                    - last_byte_content);
				dest[curr_byte_dest + 1] |= ( ( src[curr_byte_src]
				                                & MSBMASK(last_byte_content) )
				                              << overlap );
			}
		}
		else {   // loff_src >= loff_dest
			hang = loff_src - loff_dest;
			overlap = BYTESIZE - hang;
			// first byte
			dest[curr_byte_dest] &= ~(LSBMASK(BYTESIZE - loff_src) << hang);
			dest[curr_byte_dest] |= ( src[curr_byte_src]
			                          & LSBMASK(BYTESIZE - loff_src) ) << hang;
			curr_byte_src++;
			curr_byte_dest++;
			// 2nd to 2nd to last byte
			while (curr_byte_src != last_byte_src) {
				dest[curr_byte_dest - 1] &= MSBMASK(overlap);
				dest[curr_byte_dest - 1] |= src[curr_byte_src] >> overlap;
				dest[curr_byte_dest] &= LSBMASK(hang);
				dest[curr_byte_dest] |= src[curr_byte_src] << hang;
				curr_byte_src++;
				curr_byte_dest++;
			}
			// last byte
			last_byte_content = ((from_bit_src + nbits - 1) % BYTESIZE) + 1;
			if (last_byte_content <= hang) {
				dest[curr_byte_dest - 1] &= ~(LSBMASK(last_byte_content) <<
				                              (hang - last_byte_content));
				dest[curr_byte_dest - 1] |= ( src[curr_byte_src]
				                              & MSBMASK(last_byte_content) )
				                            >> overlap;
			}
			else {
				dest[curr_byte_dest - 1] &= MSBMASK(overlap);
				dest[curr_byte_dest - 1] |= src[curr_byte_src] >> overlap;
				dest[curr_byte_dest] &= LSBMASK(BYTESIZE + hang - last_byte_content);
				dest[curr_byte_dest] |= ( ( src[curr_byte_src]
				                            & MSBMASK(last_byte_content) )
				                          << hang );
			}
		}
	}
	else {
		if (loff_src < loff_dest) {
			hang = loff_dest - loff_src;
			if (nbits <= (BYTESIZE - loff_dest)) {
				dest[curr_byte_dest] &= ~(MSBMASK(nbits) >> loff_dest);
				dest[curr_byte_dest] |= ( src[curr_byte_src]
				                          & (MSBMASK(nbits) >> loff_src) ) >> hang;
			}
			else {
				dest[curr_byte_dest] &= MSBMASK(loff_dest);
				dest[curr_byte_dest] |= ( src[curr_byte_src]
				                          & LSBMASK(BYTESIZE - loff_src) ) >> hang;
				dest[curr_byte_dest + 1] &= LSBMASK(2 * BYTESIZE - loff_dest - nbits);        ;
				dest[curr_byte_dest + 1] |= ( src[curr_byte_src]
				                              & MSBMASK(nbits) >> loff_src )
				                            << (BYTESIZE - hang);
			}
		}
		else {   // loff_src >= loff_dest
			hang = loff_src - loff_dest;
			dest[curr_byte_dest] &= ~(MSBMASK(nbits) >> loff_dest);
			dest[curr_byte_dest] |= ( src[curr_byte_src]
			                          & LSBMASK(BYTESIZE - loff_src) ) << hang;
		}
	}
}


void bitarr_write_char(byte *dest, size_t from_bit, char val,
                       size_t nbits)
{
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&val, sizeof(char));
	}
	bitarr_write(dest, from_bit, (byte *)&val, BYTESIZE * sizeof(char) - nbits,
	             nbits);
}


void bitarr_write_uchar(byte *dest, size_t from_bit, unsigned char val,
                        size_t nbits)
{
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&val, sizeof(unsigned char));
	}
	bitarr_write(dest, from_bit, (byte *)&val,
	             BYTESIZE * sizeof(unsigned char) - nbits, nbits);
}


void bitarr_write_short(byte *dest, size_t from_bit, short val,
                        size_t nbits)
{
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&val, sizeof(short));
	}
	bitarr_write(dest, from_bit, (byte *)&val, BYTESIZE * sizeof(short) - nbits,
	             nbits);
}


void bitarr_write_ushort(byte *dest, size_t from_bit,
                         unsigned short val, size_t nbits)
{
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&val, sizeof(unsigned short));
	}
	bitarr_write(dest, from_bit, (byte *)&val,
	             BYTESIZE * sizeof(unsigned short) - nbits, nbits);
}


void bitarr_write_int(byte *dest, size_t from_bit, int val,
                      size_t nbits)
{
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&val, sizeof(int));
	}
	bitarr_write(dest, from_bit, (byte *)&val, BYTESIZE * sizeof(int) - nbits,
	             nbits);
}


void bitarr_write_uint(byte *dest, size_t from_bit, unsigned int val,
                       size_t nbits)
{
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&val, sizeof(unsigned int));
	}
	bitarr_write(dest, from_bit, (byte *)&val,
	             BYTESIZE * sizeof(unsigned int) - nbits, nbits);
}


void bitarr_write_long(byte *dest, size_t from_bit, long val,
                       size_t nbits)
{
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&val, sizeof(long));
	}
	bitarr_write(dest, from_bit, (byte *)&val, BYTESIZE * sizeof(long) - nbits,
	             nbits);
}


void bitarr_write_ulong(byte *dest, size_t from_bit, unsigned long val,
                        size_t nbits)
{
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&val, sizeof(unsigned long));
	}
	bitarr_write(dest, from_bit, (byte *)&val,
	             BYTESIZE * sizeof(unsigned long) - nbits, nbits);
}


void bitarr_write_llong(byte *dest, size_t from_bit, long long val,
                        size_t nbits)
{
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&val, sizeof(long long));
	}
	bitarr_write( dest, from_bit, (byte *)&val,
	              BYTESIZE * sizeof(long long) - nbits, nbits );
}


void bitarr_write_ullong(byte *dest, size_t from_bit,
                         unsigned long long val, size_t nbits)
{
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&val, sizeof(unsigned long long));
	}
	bitarr_write(dest, from_bit, (byte *)&val,
	             BYTESIZE * sizeof(unsigned long long) - nbits, nbits);
}


void bitarr_write_byte(byte *dest, size_t from_bit, byte val,
                       size_t nbits)
{
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&val, sizeof(byte));
	}
	bitarr_write(dest, from_bit, (byte *)&val, BYTESIZE * sizeof(byte) - nbits,
	             nbits);
}


void bitarr_write_size_t(byte *dest, size_t from_bit, size_t val,
                         size_t nbits)
{
	if (ENDIANNESS == LITTLE) {
		bytearr_reverse((byte *)&val, sizeof(size_t));
	}
	bitarr_write(dest, from_bit, (byte *)&val, BYTESIZE * sizeof(size_t) - nbits,
	             nbits);
}
