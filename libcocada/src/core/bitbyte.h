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


#ifndef BYTE_T
#define BYTE_T

/**
 * @brief A required unsigned byte type.
 *
 * The C11 standard  IEC 9899:2011 defines a "byte" as an addressable unit
 * of data storage large enough to hold any member of the basic character
 * set of the execution environment.
 *
 * It also defines a char as "single-byte" character and so sizeof(char)
 * should always return 1.
 *
 * Moreover, the standard library <limits.h> defines CHAR_BIT to be the
 * number of bits for smallest object that is not a bit-field ("byte")
 * and specifies a minimum size of 8 (eight).
 *
 * Although a byte may be composed of more that eight bits,
 * ISO Norm IEC 80000-13:2008 (item 13.9 c) suggests that the word "byte"
 * be defined as a synonymm of octet, i.e. a sequence of eight bits.
 *
 * A byte_t type is therefore defined as an alias for unsigned char.
 * A BYTESIZE constant is defined as a CHAR_BIT synonym, and
 * accordingly a maximum value constant BYTE_MAX is defined as
 * UCHAR_MAX synonym.
 */
typedef unsigned char byte_t;

#define BYTESIZE CHAR_BIT

/*
 * Most of the code in this library is not dependent on a byte being an octet.
 * However it has only been tested on such conditions, so this is
 * included as a safeguard.
 * If this is removed to support larger bytes, at least the byte masks
 * below should be modified.
 */
#if BYTESIZE!=8
#error Error: this code requires an 8-bit byte_t type
#endif

#define BYTE_MAX UCHAR_MAX

#endif // BYTE_T

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



#define PRIbB "c%c%c%c%c%c%c%c"

#define BYTESTRB(byte)  \
	(byte & 0x80 ? '1' : '0'), \
	(byte & 0x40 ? '1' : '0'), \
	(byte & 0x20 ? '1' : '0'), \
	(byte & 0x10 ? '1' : '0'), \
	(byte & 0x08 ? '1' : '0'), \
	(byte & 0x04 ? '1' : '0'), \
	(byte & 0x02 ? '1' : '0'), \
	(byte & 0x01 ? '1' : '0')

#define PRIbX "02x"
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
 * @brief Same as byte_bitcount(n, 0)
 * @see byte_bitcount
 */
size_t byte_bitcount0(byte_t n);


/**
 * @brief Same as byte_bitcount(n, 1)
 * @see byte_bitcount
 */
size_t byte_bitcount1(byte_t n);


/**
 * @brief Returns the number of bits with value==@p bit of a given byte.
 */
size_t byte_bitcount(byte_t n, bool bit);


/**
 * @brief Same as uint16_bitcount(n, 0)
 * @see uint16_bitcount
 */
size_t uint16_bitcount0(uint16_t n);


/**
 * @brief Same as uint16_bitcount(n, 1)
 * @see uint16_bitcount
 */
size_t uint16_bitcount1(uint16_t n);


/**
 * @brief Returns the number of bits with value==@p bit of a given 16-bit uint.
 */
size_t uint16_bitcount(uint16_t n, bool bit);


/**
 * @brief Same as uint32_bitcount(n, 0)
 * @see uint32_bitcount
 */
size_t uint32_bitcount0(uint32_t n);


/**
 * @brief Same as uint32_bitcount(n, 1)
 * @see uint32_bitcount
 */
size_t uint32_bitcount1(uint32_t n);


/**
 * @brief Returns the number of bits with value==@p bit of a given 32-bit uint.
 */
size_t uint32_bitcount(uint32_t n, bool bit);


/**
 * @brief Same as uint64_bitcount(n, 0)
 * @see uint64_bitcount
 */
size_t uint64_bitcount0(uint64_t n);


/**
 * @brief Same as uint64_bitcount(n, 1)
 * @see uint64_bitcount
 */
size_t uint64_bitcount1(uint64_t n);


/**
 * @brief Returns the number of bits with value==@p bit of a given 64-bit uint.
 */
size_t uint64_bitcount(uint64_t n, bool bit);


/**
 * @brief Same as byte_rank(@p b, @p pos, 0)
 * @see byte_rank
 */
size_t byte_rank0(byte_t b, size_t pos);


/**
 * @brief Same as byte_rank(@p b, @p pos, 1)
 * @see byte_rank
 */
size_t byte_rank1(byte_t b, size_t pos);


/**
 * @brief Computes rank_@p bit(@p b, @p pos) = # positions j<@p pos
 * s.t. @p b[j]==@p bit,
 * where @p b[j] denotes the jth bit of byte @p b from the left.
 * If i>=BYTESIZE, returns the total number of positions with value == @p bit.
 */
size_t byte_rank(byte_t b, size_t pos, bool bit);


/**
 * @brief Same as byte_select(@p b, @p rank, 0)
 * @see byte_select
 */
size_t byte_select0(byte_t b, size_t rank);


/**
 * @brief Same as byte_select(@p b, @p rank, 1)
 * @see byte_select
 */
size_t byte_select1(byte_t b, size_t rank);


/**
 * @brief Computes select_@p bit(@p b, @p rank) = j s.t.
 * @p b[j]==@p bit and rank_@p bit(@p b, j)=@p rank,
 * where @p b[j] denotes the jth bit of byte @p b from the left.
 * If no such position exists, returns BYTESIZE.
 */
size_t byte_select(byte_t b, size_t rank, bool bit);


/**
 * @brief Returns the position of the highest order 1 bit of @p v.
 */
int uint32_hibit(uint32_t v);


/**
 * @brief Returns the position of the lowest order 1 bit of @p v.
 */
int uint32_lobit(uint32_t v);


/**
 * @brief Returns the position of the lowest order 1 bit of @p v.
 */
byte_t uint64_lobit(uint64_t v);

#endif