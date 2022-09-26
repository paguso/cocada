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

#include "bitbyte.h"
#include "coretype.h"
//#include "new.h"
#include "mathutil.h"


size_t nbytes(size_t nvalues)
{
	return (nvalues<2)?nvalues:(size_t)(DIVCEIL(log2((double)nvalues), BYTESIZE));
}


#if ENDIANNESS==LITTLE
static const uint64_t byte_as_uint64_str[256] = {
	0x3030303030303030,0x3130303030303030,0x3031303030303030,0x3131303030303030,
	0x3030313030303030,0x3130313030303030,0x3031313030303030,0x3131313030303030,
	0x3030303130303030,0x3130303130303030,0x3031303130303030,0x3131303130303030,
	0x3030313130303030,0x3130313130303030,0x3031313130303030,0x3131313130303030,
	0x3030303031303030,0x3130303031303030,0x3031303031303030,0x3131303031303030,
	0x3030313031303030,0x3130313031303030,0x3031313031303030,0x3131313031303030,
	0x3030303131303030,0x3130303131303030,0x3031303131303030,0x3131303131303030,
	0x3030313131303030,0x3130313131303030,0x3031313131303030,0x3131313131303030,
	0x3030303030313030,0x3130303030313030,0x3031303030313030,0x3131303030313030,
	0x3030313030313030,0x3130313030313030,0x3031313030313030,0x3131313030313030,
	0x3030303130313030,0x3130303130313030,0x3031303130313030,0x3131303130313030,
	0x3030313130313030,0x3130313130313030,0x3031313130313030,0x3131313130313030,
	0x3030303031313030,0x3130303031313030,0x3031303031313030,0x3131303031313030,
	0x3030313031313030,0x3130313031313030,0x3031313031313030,0x3131313031313030,
	0x3030303131313030,0x3130303131313030,0x3031303131313030,0x3131303131313030,
	0x3030313131313030,0x3130313131313030,0x3031313131313030,0x3131313131313030,
	0x3030303030303130,0x3130303030303130,0x3031303030303130,0x3131303030303130,
	0x3030313030303130,0x3130313030303130,0x3031313030303130,0x3131313030303130,
	0x3030303130303130,0x3130303130303130,0x3031303130303130,0x3131303130303130,
	0x3030313130303130,0x3130313130303130,0x3031313130303130,0x3131313130303130,
	0x3030303031303130,0x3130303031303130,0x3031303031303130,0x3131303031303130,
	0x3030313031303130,0x3130313031303130,0x3031313031303130,0x3131313031303130,
	0x3030303131303130,0x3130303131303130,0x3031303131303130,0x3131303131303130,
	0x3030313131303130,0x3130313131303130,0x3031313131303130,0x3131313131303130,
	0x3030303030313130,0x3130303030313130,0x3031303030313130,0x3131303030313130,
	0x3030313030313130,0x3130313030313130,0x3031313030313130,0x3131313030313130,
	0x3030303130313130,0x3130303130313130,0x3031303130313130,0x3131303130313130,
	0x3030313130313130,0x3130313130313130,0x3031313130313130,0x3131313130313130,
	0x3030303031313130,0x3130303031313130,0x3031303031313130,0x3131303031313130,
	0x3030313031313130,0x3130313031313130,0x3031313031313130,0x3131313031313130,
	0x3030303131313130,0x3130303131313130,0x3031303131313130,0x3131303131313130,
	0x3030313131313130,0x3130313131313130,0x3031313131313130,0x3131313131313130,
	0x3030303030303031,0x3130303030303031,0x3031303030303031,0x3131303030303031,
	0x3030313030303031,0x3130313030303031,0x3031313030303031,0x3131313030303031,
	0x3030303130303031,0x3130303130303031,0x3031303130303031,0x3131303130303031,
	0x3030313130303031,0x3130313130303031,0x3031313130303031,0x3131313130303031,
	0x3030303031303031,0x3130303031303031,0x3031303031303031,0x3131303031303031,
	0x3030313031303031,0x3130313031303031,0x3031313031303031,0x3131313031303031,
	0x3030303131303031,0x3130303131303031,0x3031303131303031,0x3131303131303031,
	0x3030313131303031,0x3130313131303031,0x3031313131303031,0x3131313131303031,
	0x3030303030313031,0x3130303030313031,0x3031303030313031,0x3131303030313031,
	0x3030313030313031,0x3130313030313031,0x3031313030313031,0x3131313030313031,
	0x3030303130313031,0x3130303130313031,0x3031303130313031,0x3131303130313031,
	0x3030313130313031,0x3130313130313031,0x3031313130313031,0x3131313130313031,
	0x3030303031313031,0x3130303031313031,0x3031303031313031,0x3131303031313031,
	0x3030313031313031,0x3130313031313031,0x3031313031313031,0x3131313031313031,
	0x3030303131313031,0x3130303131313031,0x3031303131313031,0x3131303131313031,
	0x3030313131313031,0x3130313131313031,0x3031313131313031,0x3131313131313031,
	0x3030303030303131,0x3130303030303131,0x3031303030303131,0x3131303030303131,
	0x3030313030303131,0x3130313030303131,0x3031313030303131,0x3131313030303131,
	0x3030303130303131,0x3130303130303131,0x3031303130303131,0x3131303130303131,
	0x3030313130303131,0x3130313130303131,0x3031313130303131,0x3131313130303131,
	0x3030303031303131,0x3130303031303131,0x3031303031303131,0x3131303031303131,
	0x3030313031303131,0x3130313031303131,0x3031313031303131,0x3131313031303131,
	0x3030303131303131,0x3130303131303131,0x3031303131303131,0x3131303131303131,
	0x3030313131303131,0x3130313131303131,0x3031313131303131,0x3131313131303131,
	0x3030303030313131,0x3130303030313131,0x3031303030313131,0x3131303030313131,
	0x3030313030313131,0x3130313030313131,0x3031313030313131,0x3131313030313131,
	0x3030303130313131,0x3130303130313131,0x3031303130313131,0x3131303130313131,
	0x3030313130313131,0x3130313130313131,0x3031313130313131,0x3131313130313131,
	0x3030303031313131,0x3130303031313131,0x3031303031313131,0x3131303031313131,
	0x3030313031313131,0x3130313031313131,0x3031313031313131,0x3131313031313131,
	0x3030303131313131,0x3130303131313131,0x3031303131313131,0x3131303131313131,
	0x3030313131313131,0x3130313131313131,0x3031313131313131,0x3131313131313131
};
#elif ENDIANNESS==BIG
static const uint64_t byte_as_uint64_str[256] = {
	0x3030303030303030,0x3030303030303031,0x3030303030303130,0x3030303030303131,
	0x3030303030313030,0x3030303030313031,0x3030303030313130,0x3030303030313131,
	0x3030303031303030,0x3030303031303031,0x3030303031303130,0x3030303031303131,
	0x3030303031313030,0x3030303031313031,0x3030303031313130,0x3030303031313131,
	0x3030303130303030,0x3030303130303031,0x3030303130303130,0x3030303130303131,
	0x3030303130313030,0x3030303130313031,0x3030303130313130,0x3030303130313131,
	0x3030303131303030,0x3030303131303031,0x3030303131303130,0x3030303131303131,
	0x3030303131313030,0x3030303131313031,0x3030303131313130,0x3030303131313131,
	0x3030313030303030,0x3030313030303031,0x3030313030303130,0x3030313030303131,
	0x3030313030313030,0x3030313030313031,0x3030313030313130,0x3030313030313131,
	0x3030313031303030,0x3030313031303031,0x3030313031303130,0x3030313031303131,
	0x3030313031313030,0x3030313031313031,0x3030313031313130,0x3030313031313131,
	0x3030313130303030,0x3030313130303031,0x3030313130303130,0x3030313130303131,
	0x3030313130313030,0x3030313130313031,0x3030313130313130,0x3030313130313131,
	0x3030313131303030,0x3030313131303031,0x3030313131303130,0x3030313131303131,
	0x3030313131313030,0x3030313131313031,0x3030313131313130,0x3030313131313131,
	0x3031303030303030,0x3031303030303031,0x3031303030303130,0x3031303030303131,
	0x3031303030313030,0x3031303030313031,0x3031303030313130,0x3031303030313131,
	0x3031303031303030,0x3031303031303031,0x3031303031303130,0x3031303031303131,
	0x3031303031313030,0x3031303031313031,0x3031303031313130,0x3031303031313131,
	0x3031303130303030,0x3031303130303031,0x3031303130303130,0x3031303130303131,
	0x3031303130313030,0x3031303130313031,0x3031303130313130,0x3031303130313131,
	0x3031303131303030,0x3031303131303031,0x3031303131303130,0x3031303131303131,
	0x3031303131313030,0x3031303131313031,0x3031303131313130,0x3031303131313131,
	0x3031313030303030,0x3031313030303031,0x3031313030303130,0x3031313030303131,
	0x3031313030313030,0x3031313030313031,0x3031313030313130,0x3031313030313131,
	0x3031313031303030,0x3031313031303031,0x3031313031303130,0x3031313031303131,
	0x3031313031313030,0x3031313031313031,0x3031313031313130,0x3031313031313131,
	0x3031313130303030,0x3031313130303031,0x3031313130303130,0x3031313130303131,
	0x3031313130313030,0x3031313130313031,0x3031313130313130,0x3031313130313131,
	0x3031313131303030,0x3031313131303031,0x3031313131303130,0x3031313131303131,
	0x3031313131313030,0x3031313131313031,0x3031313131313130,0x3031313131313131,
	0x3130303030303030,0x3130303030303031,0x3130303030303130,0x3130303030303131,
	0x3130303030313030,0x3130303030313031,0x3130303030313130,0x3130303030313131,
	0x3130303031303030,0x3130303031303031,0x3130303031303130,0x3130303031303131,
	0x3130303031313030,0x3130303031313031,0x3130303031313130,0x3130303031313131,
	0x3130303130303030,0x3130303130303031,0x3130303130303130,0x3130303130303131,
	0x3130303130313030,0x3130303130313031,0x3130303130313130,0x3130303130313131,
	0x3130303131303030,0x3130303131303031,0x3130303131303130,0x3130303131303131,
	0x3130303131313030,0x3130303131313031,0x3130303131313130,0x3130303131313131,
	0x3130313030303030,0x3130313030303031,0x3130313030303130,0x3130313030303131,
	0x3130313030313030,0x3130313030313031,0x3130313030313130,0x3130313030313131,
	0x3130313031303030,0x3130313031303031,0x3130313031303130,0x3130313031303131,
	0x3130313031313030,0x3130313031313031,0x3130313031313130,0x3130313031313131,
	0x3130313130303030,0x3130313130303031,0x3130313130303130,0x3130313130303131,
	0x3130313130313030,0x3130313130313031,0x3130313130313130,0x3130313130313131,
	0x3130313131303030,0x3130313131303031,0x3130313131303130,0x3130313131303131,
	0x3130313131313030,0x3130313131313031,0x3130313131313130,0x3130313131313131,
	0x3131303030303030,0x3131303030303031,0x3131303030303130,0x3131303030303131,
	0x3131303030313030,0x3131303030313031,0x3131303030313130,0x3131303030313131,
	0x3131303031303030,0x3131303031303031,0x3131303031303130,0x3131303031303131,
	0x3131303031313030,0x3131303031313031,0x3131303031313130,0x3131303031313131,
	0x3131303130303030,0x3131303130303031,0x3131303130303130,0x3131303130303131,
	0x3131303130313030,0x3131303130313031,0x3131303130313130,0x3131303130313131,
	0x3131303131303030,0x3131303131303031,0x3131303131303130,0x3131303131303131,
	0x3131303131313030,0x3131303131313031,0x3131303131313130,0x3131303131313131,
	0x3131313030303030,0x3131313030303031,0x3131313030303130,0x3131313030303131,
	0x3131313030313030,0x3131313030313031,0x3131313030313130,0x3131313030313131,
	0x3131313031303030,0x3131313031303031,0x3131313031303130,0x3131313031303131,
	0x3131313031313030,0x3131313031313031,0x3131313031313130,0x3131313031313131,
	0x3131313130303030,0x3131313130303031,0x3131313130303130,0x3131313130303131,
	0x3131313130313030,0x3131313130313031,0x3131313130313130,0x3131313130313131,
	0x3131313131303030,0x3131313131303031,0x3131313131303130,0x3131313131303131,
	0x3131313131313030,0x3131313131313031,0x3131313131313130,0x3131313131313131
};
#else
#error "Little or Big endianness required"
#endif

void byte_to_str(byte_t b, char *dest)
{
	*((uint64_t *)dest) = byte_as_uint64_str[b];
	dest[8]='\0';
}


void byte_to_strx(byte_t b, char *dest)
{
	snprintf(dest, 3, "%"PRIbX, BYTESTRX(b));
}



void byte_reverse(byte_t *b)
{
	*b = (*b<<4) | (*b>>4);
	*b = ((*b<<2) & 0xCC) | ((*b>>2) & 0x33);
	*b = ((*b<<1) & 0xAA) | ((*b>>1) & 0x55);
}


size_t byte_bitcount0(byte_t b)
{
	return byte_bitcount1(~b);
}


size_t byte_bitcount1(byte_t b)
{
#if GCC_BUILTINS
	return uint_bitcount1(b);
#else
	b = ((b>>1) & 0x55)+(b & 0x55);
	b = ((b>>2) & 0x33)+(b & 0x33);
	b = ((b>>4) & 0x0F)+(b & 0x0F);
	return (size_t)b;
#endif
}


size_t byte_bitcount(byte_t b, bool bit)
{
	if (bit) return byte_bitcount1(b);
	else return byte_bitcount0(b);
}


size_t uint16_bitcount0(uint16_t n)
{
	return uint16_bitcount1(~n);
}


size_t uint16_bitcount1(uint16_t n)
{
	n = ((n>>1) & 0x5555)+(n & 0x5555);
	n = ((n>>2) & 0x3333)+(n & 0x3333);
	n = ((n>>4) & 0x0F0F)+(n & 0x0F0F);
	n = ((n>>8) & 0x00FF)+(n & 0x00FF);
	return (size_t)n;
}


size_t uint16_bitcount(uint16_t n, bool bit)
{
	if (bit)
		return uint16_bitcount1(n);
	else
		return uint16_bitcount0(n);
}


size_t uint32_bitcount0(uint32_t n)
{
	return uint32_bitcount1(~n);
}


size_t uint32_bitcount1(uint32_t n)
{
	n = ((n>>1) & 0x55555555)+(n & 0x55555555);
	n = ((n>>2) & 0x33333333)+(n & 0x33333333);
	n = ((n>>4) & 0x0F0F0F0F)+(n & 0x0F0F0F0F);
	n = ((n>>8) & 0x00FF00FF)+(n & 0x00FF00FF);
	n = ((n>>16) & 0x0000FFFF)+(n & 0x0000FFFF);
	return (size_t)n;
}


size_t uint32_bitcount(uint32_t n, bool bit)
{
	if (bit)
		return uint32_bitcount1(n);
	else
		return uint32_bitcount0(n);
}


size_t uint64_bitcount0(uint64_t n)
{
	return uint64_bitcount1(~n);
}


size_t uint64_bitcount1(uint64_t n)
{
	n = ((n>>1) & 0x5555555555555555)+(n & 0x5555555555555555);
	n = ((n>>2) & 0x3333333333333333)+(n & 0x3333333333333333);
	n = ((n>>4) & 0x0F0F0F0F0F0F0F0F)+(n & 0x0F0F0F0F0F0F0F0F);
	n = ((n>>8) & 0x00FF00FF00FF00FF)+(n & 0x00FF00FF00FF00FF);
	n = ((n>>16) & 0x0000FFFF0000FFFF)+(n & 0x0000FFFF0000FFFF);
	n = ((n>>32) & 0x00000000FFFFFFFF)+(n & 0x00000000FFFFFFFF);
	return (size_t)n;
}


size_t uint64_bitcount(uint64_t n, bool bit)
{
	if (bit)
		return uint64_bitcount1(n);
	else
		return uint64_bitcount0(n);
}


int ushort_bitcount1(unsigned short x)
{
	return uint_bitcount1(x);
}

int ushort_bitcount0(unsigned short x)
{
	return USHRT_BITS - ushort_bitcount1(x);
}

int ushort_bitcount(unsigned short x, bool bit)
{
	return bit ?
		ushort_bitcount1(x) :
		USHRT_BITS - ushort_bitcount1(x);
}



int uint_bitcount1(unsigned int n) 
{
#if GCC_BUILTINS
	return __builtin_popcount(n);
#elif UINT_BITS==8
	return byte_bitcount1(n);
#elif UINT_BITS==16
	return uint16_bitcount1(n);
#elif UINT_BITS==32
	return uint32_bitcount1(n);
#elif UINT_BITS==64
	return uint64_bitcount1(n);
#else 
#error "Unknown uint size"
#endif
}


int uint_bitcount0(unsigned int x) {
	return UINT_BITS - uint_bitcount1(x);
}

int uint_bitcount(unsigned int x, bool bit)
{
	return bit ?
		uint_bitcount1(x) :
		UINT_BITS - uint_bitcount1(x);
}


int ulong_bitcount1(unsigned long x) 
{
#if GCC_BUILTINS
	return __builtin_popcountl(x);
#elif ULONG_BITS==8
	return byte_bitcount1(x);
#elif ULONG_BITS==16
	return uint16_bitcount1(x);
#elif ULONG_BITS==32
	return uint32_bitcount1(x);
#elif ULONG_BITS==64
	return uint64_bitcount1(x);
#else 
#error "Unknown uint size"
#endif
}


int ulong_bitcount0(unsigned long x) {
	return ULONG_BITS - ulong_bitcount1(x);
}


int ulong_bitcount(unsigned long x, bool bit)
{
	return bit ?
		ulong_bitcount1(x) :
		ULONG_BITS - ulong_bitcount1(x);
}


int ullong_bitcount1(unsigned long long x) 
{
#if GCC_BUILTINS
	return __builtin_popcountll(x);
#elif ULLONG_BITS==8
	return byte_bitcount1(x);
#elif ULLONG_BITS==16
	return uint16_bitcount1(x);
#elif ULLONG_BITS==32
	return uint32_bitcount1(x);
#elif ULLONG_BITS==64
	return uint64_bitcount1(x);
#else 
#error "Unknown uint size"
#endif
}


int ullong_bitcount0(unsigned long long x) {
	return ULLONG_BITS - ullong_bitcount1(x);
}


int ullong_bitcount(unsigned long long x, bool bit)
{
	return bit ?
		ullong_bitcount1(x) :
		ULLONG_BITS - ullong_bitcount1(x);
}



size_t byte_rank0(byte_t b, size_t pos)
{
	return byte_rank1(~b, pos);
}


size_t byte_rank1(byte_t b, size_t pos)
{
	return byte_bitcount1(b>>(pos<BYTESIZE?BYTESIZE-pos:0));
}


size_t byte_rank(byte_t b, size_t pos, bool bit)
{
	if (bit)
		return byte_rank1(b, pos);
	else
		return byte_rank0(b, pos);
}


size_t byte_select0(byte_t b, size_t rank)
{
	return byte_select1(~b, rank);
}


size_t byte_select1(byte_t b, size_t rank)
{
	if (b==0) return BYTESIZE;
	size_t i=0;
	rank++;
	while (b && rank>=1) {
		// count leading zeros
		if (b <= 0x0F) {
			b<<=4;
			i+=4;
		}
		if (b <= 0x3F) {
			b<<=2;
			i+=2;
		}
		if (b <= 0x7F) {
			b<<=1;
			i+=1;
		}
		rank--;
		b&=0x7F;
	}
	return (rank==0)?i:BYTESIZE;
}


size_t byte_select(byte_t b, size_t rank, bool bit)
{
	if (bit)
		return byte_select1(b, rank);
	else
		return byte_select0(b, rank);
}


static const int table_hi[32] = {
	0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
	8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
};

int uint32_hibit(uint32_t v)
{
	v |= v >> 1; // first round down to one less than a power of 2
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return table_hi[(uint32_t)(v * 0x07C4ACDDU) >> 27];
}


static const int _uint32_lobit_tbl[32] = {
	0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
	31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

int uint32_lobit(uint32_t v)
{
	return _uint32_lobit_tbl[((uint32_t)((v & -v) * 0x077CB531U)) >> 27];
}


static const byte_t _uint64_lobit_tbl[67] = {
	64, 0, 1, 39, 2, 15, 40, 23, 3, 12, 16, 59, 41, 19, 24, 54, 4,
	128, 13, 10, 17, 62, 60, 28, 42, 30, 20, 51, 25, 44, 55, 47, 5, 32,
	128, 38, 14, 22, 11, 58, 18, 53, 63, 9, 61, 27, 29, 50, 43, 46, 31,
	37, 21, 57, 52, 8, 26, 49, 45, 36, 56, 7, 48, 35, 6, 34, 33
};

/*
 * w := (v & -v) has all but the lobit to 0
 * e.g. v = 01010110000
 *     -v = 10101010000
 *      w = 00000010000
 * so, there are 64 distinct results of the form w=2^q.
 * because 67 is a prime greater than 64, the values
 * of w % 67 are all distinct. hence we can determine w,
 * and hence q, from w % 67.
 */
byte_t uint64_lobit(uint64_t v)
{
	return _uint64_lobit_tbl[(uint64_t)(v & -v) % 67];
}

