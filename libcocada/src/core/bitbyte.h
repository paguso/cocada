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

#ifndef BITBYTE_H
#define BITBYTE_H


#include <endian.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "coretype.h"

/**
 * @file bitbyte.h
 * @author Paulo Fonseca
 *
 * @brief General definitions and functions for low level
 *        bit/byte operatios.
 *
 * @warning The code herein is not portable. Check the docs for
 *       'porting' instructions and issues.
 */


/*
 * 8-bit Byte masks:
 */
static const byte_t _BITMASK[8] = { 0x80, 0x40, 0x20, 0x10,
                                    0x08, 0x04, 0x02, 0x01
                                  };
#define BITMASK(I) (_BITMASK[I])

/*
 * Least significant bits masks _LSBMASK[j] = 0^(8-j)1^j
 */
static const byte_t _LSBMASK[9] = { 0x00, 0x01, 0x03, 0x07, 0x0f,
                                    0x1f, 0x3f, 0x7f, 0xff
                                  };
#define LSBMASK(N)  (_LSBMASK[N])

/*
 * Most significant bits masks _MSBMASK[j]=1^j0^(8-j)
 */
static const byte_t _MSBMASK[9] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0,
                                    0xf8,0xfc, 0xfe, 0xff
                                  };
#define MSBMASK(N)  (_MSBMASK[N])


/*
 * Some bitwise operations in this code can benefit from bit parallelism
 * at the processor level. It is useful to have the WORD_BIT/BYTEWORDSIZE
 * constants, which correspond to the size of a machine word in bits/bytes
 * known at compile time.
 *
 * We try to determine the values of these constants based on the processor
 * type, as shown in
 * http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
 *
 * Since I have only tested the code on x86 architectures, I have included
 * a guard to prevent compilation in other machines and force the developer
 * to be aware of this and take the appropriate measures to make the code
 * compatible with other platforms.
 */
enum _cl_processor { x86_32bits, x86_64bits };

#if defined(__x86_64__) || defined(_M_X64)
#define PROCESSOR x86_64bits
#define WORD_BIT 64
#define BYTEWORDSIZE 8
typedef uint64_t word_t;
#elif defined(__i386) || defined(_M_IX86)
#define PROCESSOR x86_32bits
#define WORD_BIT 32
#define BYTEWORDSIZE 4
typedef uint32_t word_t;
/*
 * You might want to add support for your platform here
 */
#else
#error "This code has only been tested on 32/64 bits x86 platforms."
#endif

//#define WORD_MAX (~((word_t)0))


/*
 * A constant ENDIANNESS is required to indicate byte endianness.
 *
 * As of 2016, there is no portable, standard way to check for platform
 * endianess at compile time. Since much of the code on this library depends on
 * packing numbers in bit (byte) arrays, there is a necessity of knowing
 * the details of the binary representation for marshalling/unmarshalling.
 * This could be done at runtime but it would require some tests which would
 * degrade performance. Instead, we choose to sacrifice some portability and
 * rely on some specific compilers and platforms. In particular, this code
 * only works for if byte endianness is BIG or LITTLE, as defined in the
 * endian.h.
 *
 * Some of the code herein assumes ENDIANESS to be either LITTLE (0) of
 * BIG (1). The code will not compile as is on platforms that use any other
 * byte ordering alternative.
 */
enum _cl_endianness { BIG, LITTLE };
#define LITTLE 0
#define BIG 1
#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
#define ENDIANNESS BIG
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define ENDIANNESS LITTLE
#else
#error "This code requires BIG or LITTLE byte endianess.\n\
See release notes for porting issues."
#endif
#else
#error "Unable to identify byte endianness.\n\
This code requires BIG or LITTLE byte endianess.\n\
See release notes for porting issues."
#endif


#define GCC_BUILTINS (defined(__GNUC__) && !defined(__clang__))

/**
 * @brief Binary print format specifier for the byte_t type to be used 
 * in conjunction with BYTESTRB conversion macro.
 * @see BYTESTRB
 */
#define PRIbB "c%c%c%c%c%c%c%c"

/**
 * @brief Converts a byte to a binary string for printing. To be used with the PRIbB format.
 * Example
 * ```C
 * byte_t b = 0xaa;
 * printf("byte=%"PRIbB"\n", BYTESTRB(b)); // prints "byte=10101010"
 * ```
 * @see PRIbB
 */
#define BYTESTRB(byte)  \
	(byte & 0x80 ? '1' : '0'), \
	(byte & 0x40 ? '1' : '0'), \
	(byte & 0x20 ? '1' : '0'), \
	(byte & 0x10 ? '1' : '0'), \
	(byte & 0x08 ? '1' : '0'), \
	(byte & 0x04 ? '1' : '0'), \
	(byte & 0x02 ? '1' : '0'), \
	(byte & 0x01 ? '1' : '0')

/**
 * @brief Hexadecimal print format specifier for the byte_t type to be used 
 * in conjunction with BYTESTRX conversion macro.
 * @see BYTESTRX
 */
#define PRIbX "02x"


/**
 * @brief Converts a byte to an hexadecimal string for printing. 
 * To be used with the PRIbB format.
 * Example
 * ```C
 * byte_t b = 0xaa;
 * printf("byte=%"PRIbX"\n", BYTESTRX(b)); // prints "byte=0xaa"
 * ```
 * @see PRIbB
 */
#define BYTESTRX(byte) ((int)byte)


/**
 * @brief returns the minimal number of bytes required to represent @p nvalues
 * distinct values, that is ceil(log2(nvalues)/8).
 */
size_t nbytes(size_t nvalues);


/**
 * @brief Converts a byte to a binary string.
 */
void byte_to_str(byte_t b, char *dest);


/**
 * @brief Converts a byte to an hexadecimal string.
 */
void byte_to_strx(byte_t b, char *dest);


/**
 * @brief Reverts the bits of a byte in-place.
 */
void byte_reverse(byte_t *b);


/**
 * @brief Same as byte_bitcount(x, 0)
 * @see byte_bitcount
 */
uint byte_bitcount0(byte_t x);


/**
 * @brief Same as byte_bitcount(x, 1)
 * @see byte_bitcount
 */
uint byte_bitcount1(byte_t x);


/**
 * @brief Returns the number of bits with value==@p bit of a given byte.
 */
uint byte_bitcount(byte_t x, bool bit);


/**
 * @brief Same as byte_rank(@p b, @p pos, 0)
 * @see byte_rank
 */
uint byte_rank0(byte_t b, uint pos);


/**
 * @brief Same as byte_rank(@p b, @p pos, 1)
 * @see byte_rank
 */
uint byte_rank1(byte_t b, uint pos);


/**
 * @brief Computes rank_@p bit(@p b, @p pos) = # positions j<@p pos
 * s.t. @p b[j]==@p bit,
 * where @p b[j] denotes the jth bit of byte @p b from the left.
 * If i>=BYTESIZE, returns the total number of positions with value == @p bit.
 */
uint byte_rank(byte_t b, uint pos, bool bit);


/**
 * @brief Same as byte_select(@p b, @p rank, 0)
 * @see byte_select
 */
uint byte_select0(byte_t b, uint rank);


/**
 * @brief Same as byte_select(@p b, @p rank, 1)
 * @see byte_select
 */
uint byte_select1(byte_t b, uint rank);


/**
 * @brief Computes select_@p bit(@p b, @p rank) = j s.t.
 * @p b[j]==@p bit and rank_@p bit(@p b, j)=@p rank,
 * where @p b[j] denotes the jth bit of byte @p b from the left.
 * If no such position exists, returns BYTESIZE.
 */
uint byte_select(byte_t b, uint rank, bool bit);


/**
 * @brief Same as uint16_bitcount(n, 0)
 * @see uint16_bitcount
 */
uint uint16_bitcount0(uint16_t x);


/**
 * @brief Same as uint16_bitcount(n, 1)
 * @see uint16_bitcount
 */
uint uint16_bitcount1(uint16_t x);


/**
 * @brief Returns the number of bits with value==@p bit of a given 16-bit uint.
 */
uint uint16_bitcount(uint16_t x, bool bit);


/**
 * @brief Same as uint32_bitcount(x, 0)
 * @see uint32_bitcount
 */
uint uint32_bitcount0(uint32_t x);


/**
 * @brief Same as uint32_bitcount(x, 1)
 * @see uint32_bitcount
 */
uint uint32_bitcount1(uint32_t x);


/**
 * @brief Returns the number of bits with value==@p bit of a given 32-bit uint.
 */
uint uint32_bitcount(uint32_t x, bool bit);


/**
 * @brief Same as uint64_bitcount(x, 0)
 * @see uint64_bitcount
 */
uint uint64_bitcount0(uint64_t x);


/**
 * @brief Same as uint64_bitcount(n, 1)
 * @see uint64_bitcount
 */
uint uint64_bitcount1(uint64_t x);


/**
 * @brief Returns the number of bits with value==@p bit of a given 64-bit uint.
 */
uint uint64_bitcount(uint64_t x, bool bit);


/**
 * @brief Same as ushort_bitcount(x, 1)
 * @see ushort_bitcount()
 */
uint ushort_bitcount1(unsigned short x);


/**
 * @brief Same as ushort_bitcount(x, 0)
 * @see ushort_bitcount()
 */
uint ushort_bitcount0(unsigned short x);


/**
 * @brief Returns the number of bits with value ==@p bit of a given ushort value.
 */
uint ushort_bitcount(unsigned short x, bool bit);


/**
 * @brief Same as uint_bitcount(x, 1)
 * @see uint_bitcount()
 */
uint uint_bitcount1(unsigned int x);


/**
 * @brief Same as uint_bitcount(x, 0)
 * @see uint_bitcount()
 */
uint uint_bitcount0(unsigned int x);


/**
 * @brief Returns the number of bits with value==@p bit of a given uint value.
 */
uint uint_bitcount(unsigned int x, bool bit);


/**
 * @brief Same as ulong_bitcount(x, 1)
 * @see ulong_bitcount()
 */
uint ulong_bitcount1(unsigned long x);


/**
 * @brief Same as ulong_bitcount(x, 0)
 * @see ulong_bitcount()
 */
uint ulong_bitcount0(unsigned long x);


/**
 * @brief Returns the number of bits with value==@p bit of a given ulong value.
 */
uint ulong_bitcount(unsigned long x, bool bit);


/**
 * @brief Same as ullong_bitcount(x, 1)
 * @see ushort_bitcount()
 */
uint ullong_bitcount1(unsigned long long x);


/**
 * @brief Same as ullong_bitcount(x, 0)
 * @see ullong_bitcount()
 */
uint ullong_bitcount0(unsigned long long x);


/**
 * @brief Returns the number of bits with value ==@p bit of a given ullong value.
 */
uint ullong_bitcount(unsigned long long x, bool bit);


/**
 * @brief Returns the position of the highest order 1 bit of the 32-bit uint @p x.
 * If x==0 returns 32.
 */
uint uint32_hibit(uint32_t x);


/**
 * @brief Returns the position of the lowest order 1 bit of the 32-bit uint @p x.
 * If x==0 returns 32.
 */
uint uint32_lobit(uint32_t x);


/**
 * @brief Returns the position of the highest order 1 bit of the 64-bit uint @p x.
 * If x==0 returns 64.
 */
uint uint64_hibit(uint64_t x);


/**
 * @brief Returns the position of the lowest order 1 bit of the 64-bit uint @p x.
 * If x==0 returns 64.
 */
uint uint64_lobit(uint64_t x);



#endif