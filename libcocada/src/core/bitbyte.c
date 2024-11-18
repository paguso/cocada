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


usize nbytes(usize nvalues)
{
	return (nvalues < 2) ? nvalues : (usize)(DIVCEIL(log2((double)nvalues),
	        BYTESIZE));
}


void byte_to_str_bin(byte b, char *dest)
{
	snprintf(dest, 9, "%b", b);
}


void byte_to_str_hex(byte b, char *dest)
{
	snprintf(dest, 3, "%x", b);
}


void byte_reverse(byte *b)
{
	*b = (*b << 4) | (*b >> 4);
	*b = ((*b << 2) & 0xCC) | ((*b >> 2) & 0x33);
	*b = ((*b << 1) & 0xAA) | ((*b >> 1) & 0x55);
}


uint byte_bitcount0(byte b)
{
	return byte_bitcount1(~b);
}


uint byte_bitcount1(byte b)
{
#if GCC_BUILTINS
	return __builtin_popcount(b);
#else
	b = ((b >> 1) & 0x55) + (b & 0x55);
	b = ((b >> 2) & 0x33) + (b & 0x33);
	b = ((b >> 4) & 0x0F) + (b & 0x0F);
	return (uint)b;
#endif
}


uint byte_bitcount(byte b, bool bit)
{
	return bit ?
	       byte_bitcount1(b) :
	       byte_bitcount0(b);
}


uint byte_rank1(byte b, uint pos)
{
	return byte_bitcount1(b >> (pos < BYTESIZE ? BYTESIZE - pos : 0));
}


uint byte_rank0(byte b, uint pos)
{
	return byte_rank1(~b, pos);
}


uint byte_rank(byte b, uint pos, bool bit)
{
	return bit ?
	       byte_rank1(b, pos) :
	       byte_rank0(b, pos);
}


uint byte_select0(byte b, uint rank)
{
	return byte_select1(~b, rank);
}


uint byte_select1(byte b, uint rank)
{
	if (b == 0) return BYTESIZE;
	uint i = 0;
	rank++;
	while (b && rank >= 1) {
		// count leading zeros
		if (b <= 0x0F) {
			b <<= 4;
			i += 4;
		}
		if (b <= 0x3F) {
			b <<= 2;
			i += 2;
		}
		if (b <= 0x7F) {
			b <<= 1;
			i += 1;
		}
		rank--;
		b &= 0x7F;
	}
	return (rank == 0) ? i : BYTESIZE;
}


uint byte_select(byte b, uint rank, bool bit)
{
	return bit ?
	       byte_select1(b, rank) :
	       byte_select0(b, rank);
}


uint uint16_bitcount0(uint16 x)
{
	return uint16_bitcount1(~x);
}


uint uint16_bitcount1(uint16 x)
{
#if GCC_BUILTINS
	// In C11 uint is at least 16 bits
	return __builtin_popcount(x);
#else
	x = ((x >> 1) & 0x5555) + (x & 0x5555);
	x = ((x >> 2) & 0x3333) + (x & 0x3333);
	x = ((x >> 4) & 0x0F0F) + (x & 0x0F0F);
	x = ((x >> 8) & 0x00FF) + (x & 0x00FF);
	return (uint)x;
#endif
}


uint uint16_bitcount(uint16 x, bool bit)
{
	return bit ?
	       uint16_bitcount1(x) :
	       uint16_bitcount0(x);
}


uint uint32_bitcount0(uint32 x)
{
	return uint32_bitcount1(~x);
}


uint uint32_bitcount1(uint32 x)
{
#if GCC_BUILTINS
	return __builtin_popcountl(x);
#else
	x = ((x >> 1) & 0x55555555) + (x & 0x55555555);
	x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
	x = ((x >> 4) & 0x0F0F0F0F) + (x & 0x0F0F0F0F);
	x = ((x >> 8) & 0x00FF00FF) + (x & 0x00FF00FF);
	x = ((x >> 16) & 0x0000FFFF) + (x & 0x0000FFFF);
	return (uint)x;
#endif
}


uint uint32_bitcount(uint32 x, bool bit)
{
	if (bit)
		return uint32_bitcount1(x);
	else
		return uint32_bitcount0(x);
}


uint uint64_bitcount0(uint64 x)
{
	return uint64_bitcount1(~x);
}


uint uint64_bitcount1(uint64 x)
{
#if GCC_BUILTINS
	return __builtin_popcountll(x);
#else
	x = ((x >> 1) & 0x5555555555555555) + (x & 0x5555555555555555);
	x = ((x >> 2) & 0x3333333333333333) + (x & 0x3333333333333333);
	x = ((x >> 4) & 0x0F0F0F0F0F0F0F0F) + (x & 0x0F0F0F0F0F0F0F0F);
	x = ((x >> 8) & 0x00FF00FF00FF00FF) + (x & 0x00FF00FF00FF00FF);
	x = ((x >> 16) & 0x0000FFFF0000FFFF) + (x & 0x0000FFFF0000FFFF);
	x = ((x >> 32) & 0x00000000FFFFFFFF) + (x & 0x00000000FFFFFFFF);
	return (uint)x;
#endif
}


uint uint64_bitcount(uint64 x, bool bit)
{
	return bit ?
	       uint64_bitcount1(x) :
	       uint64_bitcount0(x);
}


uint ushort_bitcount1(unsigned short x)
{
	return uint_bitcount1(x);
}


uint ushort_bitcount0(unsigned short x)
{
	return USHRT_BITS - ushort_bitcount1(x);
}


uint ushort_bitcount(unsigned short x, bool bit)
{
	return bit ?
	       ushort_bitcount1(x) :
	       USHRT_BITS - ushort_bitcount1(x);
}


uint uint_bitcount1(unsigned int n)
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


uint uint_bitcount0(unsigned int x)
{
	return UINT_BITS - uint_bitcount1(x);
}


uint uint_bitcount(unsigned int x, bool bit)
{
	return bit ?
	       uint_bitcount1(x) :
	       UINT_BITS - uint_bitcount1(x);
}


uint ulong_bitcount1(unsigned long x)
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


uint ulong_bitcount0(unsigned long x)
{
	return ULONG_BITS - ulong_bitcount1(x);
}


uint ulong_bitcount(unsigned long x, bool bit)
{
	return bit ?
	       ulong_bitcount1(x) :
	       ULONG_BITS - ulong_bitcount1(x);
}


uint ullong_bitcount1(unsigned long long x)
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


uint ullong_bitcount0(unsigned long long x)
{
	return ULLONG_BITS - ullong_bitcount1(x);
}


uint ullong_bitcount(unsigned long long x, bool bit)
{
	return bit ?
	       ullong_bitcount1(x) :
	       ULLONG_BITS - ullong_bitcount1(x);
}


static const byte _uint16_hibit_tbl[19] = {
	16, 0, 12, 1, 15, 13, 5, 2, 7, 16, 11,
	14, 4, 6, 10, 3, 9, 8, 16
};


uint uint16_hibit(uint16 x)
{
	x |= x >> 1; // first round down to one less than a power of 2
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	return (uint)_uint16_hibit_tbl[(uint16)(x % 19)];
}


static const byte _uint16_lobit_tbl[19] = {
	16, 0, 1, 13, 2, 16, 14, 6, 3, 8, 16,
	12, 15, 5, 7, 11, 4, 10, 9
};


uint uint16_lobit(uint16 x)
{
	return (uint)_uint16_lobit_tbl[(uint16)((x & -x) % 19)];
}


static const byte _uint32_hibit_tbl[37] = {
	32, 0, 25, 1, 22, 26, 31, 2, 15, 23,
	29, 27, 10, 32, 12, 3, 6, 16, 32, 24,
	21, 30, 14, 28, 9, 11, 5, 32, 20, 13,
	8, 4, 19, 7, 18, 17, 32
};

uint uint32_hibit(uint32 x)
{
	x |= x >> 1; // first round down to one less than a power of 2
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return (uint)_uint32_hibit_tbl[(uint32)(x % 37)];
}


static const byte _uint32_lobit_tbl[37] = {
	32, 0, 1, 26, 2, 23, 27, 32, 3, 16,
	24, 30, 28, 11, 32, 13, 4, 7, 17, 32,
	25, 22, 31, 15, 29, 10, 12, 6, 32, 21,
	14, 9, 5, 20, 8, 19, 18
};


uint uint32_lobit(uint32 x)
{
	return (uint)_uint32_lobit_tbl[(uint32)((x & -x) % 37)];
}


static const byte _uint64_lobit_tbl[67] = {
	64, 0, 1, 39, 2, 15, 40, 23, 3, 12,
	16, 59, 41, 19, 24, 54, 4, 128, 13, 10,
	17, 62, 60, 28, 42, 30, 20, 51, 25, 44,
	55, 47, 5, 32, 128, 38, 14, 22, 11, 58,
	18, 53, 63, 9, 61, 27, 29, 50, 43, 46,
	31, 37, 21, 57, 52, 8, 26, 49, 45, 36,
	56, 7, 48, 35, 6, 34, 33
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
uint uint64_lobit(uint64 v)
{
	return (uint)_uint64_lobit_tbl[(uint64)(v & -v) % 67];
}


static const byte _uint64_hibit_tbl[67] =  {
	64, 0, 1, 39, 2, 15, 40, 23, 3, 12,
	16, 59, 41, 19, 24, 54, 4, 64, 13, 10,
	17, 62, 60, 28, 42, 30, 20, 51, 25, 44,
	55, 47, 5, 32, 64, 38, 14, 22, 11, 58,
	18, 53, 63, 9, 61, 27, 29, 50, 43, 46,
	31, 37, 21, 57, 52, 8, 26, 49, 45, 36,
	56, 7, 48, 35, 6, 34, 33
};


uint uint64_hibit(uint64 x)
{
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x |= x >> 32;
	x ^= x >> 1;
	return (uint)_uint64_hibit_tbl[x % 67];
}
