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

#ifndef BITARR_H
#define BITARR_H

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "bitbyte.h"
#include "strbuf.h"

/**
 * @file bitarr.h
 * @author Paulo Fonseca
 *
 * @brief Tight bitarray implemented as a raw byte_t array
 * with read/write operations
 */


/**
 * @brief Creates a new raw bitarray with all bits set to zero
 * @warn The actual size will be the minimum number of bytes necessary
 *       to represent @p len bits
 */
byte_t *bitarr_new(size_t len);

/**
 * @brief Creates a new bit array from a 0-1 character string.
 * If the string contains characters other than {0,1} the result is undefined.
 * @param src (no transfer) The source 0-1 character string
 * @param len The length of the bitarray
 */
byte_t *bitarr_new_from_str(const char *src, size_t len);


/**
 * @brief Parses a 0-1 character string onto a bitarray.
 * If the string contains characters other than {0,1} the result is undefined.
 * @param dest The destination bitarray
 * @param src The source 0-1 character string
 * @param len The length of the bitarray
 */
void bitarr_parse_str(byte_t *dest, const char *src, size_t len);


/**
 * @brief Returns the bit at a given position
 */
bool bitarr_get_bit(const byte_t *ba, size_t pos);


/**
 * @brief Sets the bit at a given position
 */
void bitarr_set_bit(byte_t *ba, size_t pos, bool bit_val);


/**
 * @brief Prints a representation of a bitarray as text to an output FILE.
 * @return Upon success returns the number of chars printed
 */
int bitarr_fprint( FILE *stream, const byte_t *ba, size_t nbits,
                   uint bytes_per_line, uint indent);


/**
 * @brief Prints a representation of a bitarray as text to a string.
 * @return Upon success returns the number of chars printed
 */
int bitarr_sprint( char *str, const byte_t *ba, size_t nbits,
                   uint bytes_per_line, uint indent);


/**
 * @brief Prints a representation of a bitarray as text to a string buffer.
 * @return Upon success returns the number of chars printed
 */
int bitarr_sbprint(strbuf *buf, const byte_t *ba, size_t nbits,
                   uint bytes_per_line, uint indent);


/**
 * @brief ANDs a given number of bits of a bitarray with those of a given mask,
 * that is, ba[0:nbits] &= mask[0:nbits].
 * @param ba (no transfer) The target bitarray.
 * @param mask (no transfer) The mask bitarray.
 * @param nbits The number of bits to be AND'd.
 */
void bitarr_and(byte_t *ba, const byte_t *mask, size_t nbits);


/**
 * @brief ORs a given number of bits of a bitarray with those of a given mask,
 * that is, ba[0:nbits] |= mask[0:nbits].
 * @param ba (no transfer) The target bitarray.
 * @param mask (no transfer) The mask bitarray.
 * @param nbits The number of bits to be OR'd.
 */
void bitarr_or(byte_t *ba, const byte_t *mask, size_t nbits);


/**
 * @brief Inverts the first nbits bits of a bitarray.
 * @param ba The target bitarray.
 * @param nbits The number of bits to be flipped.
 */
void bitarr_not(byte_t *ba, size_t nbits);


/**
 * @brief Generic write operation:
 *        @p dest[@p from_bit_dest:@p from_bit_dest+@p nbits]
 *        = @p src[@p from_bit_src:@p from_bit_src+@p nbits]
 * @param dest The destination bitarray
 * @param from_bit_dest The initial position to be (over)written in
 *                      the destination bitarray
 * @param src The source bitarray
 * @param from_bit_src The initial position to be read from the source bitarray
 * @param nbits The number of bits to be written
 */
void bitarr_write( byte_t *dest, size_t from_bit_dest, const byte_t *src,
                   size_t from_bit_src, size_t nbits );


/**
 * @brief Writes the @p nbits least significant bits of a char @p val
 * to a bitarray @p dest.
 *
 * @see bitarr_write_int for similar details.
 */
void bitarr_write_char( byte_t *dest, size_t from_bit, char val,
                        size_t nbits );


/**
 * @brief Writes the @p nbits least significant bits of an unsigned char @p val
 * to a bitarray @p dest.
 *
 * @see bitarr_write_int for similar details.
 */
void bitarr_write_uchar( byte_t *dest, size_t from_bit, unsigned char val,
                         size_t nbits );


/**
 * @brief Writes the @p nbits least significant bits of a short @p val
 * to a bitarray @p dest.
 *
 * @see bitarr_write_int for similar details.
 */
void bitarr_write_short( byte_t *dest, size_t from_bit, short val,
                         size_t nbits );


/**
 * @brief Writes the @p nbits least significant bits of an unsigned short @p val
 * to a bitarray @p dest.
 *
 * @see bitarr_write_int for similar details.
 */
void bitarr_write_ushort( byte_t *dest, size_t from_bit,
                          unsigned short val, size_t nbits );


/**
 * @brief Writes the @p nbits least significant bits of an int @p val
 * to a bitarray @p dest.
 *
 * This utility function is usually used in conjunction with bitarr_read_int
 * for tightly storing ints using as few bits as necessary.
 *
 * @warning If not enough bits are written, information may be lost concerning
 * the magnitude and/or signal of @p val.
 * For example, the value <tt>15</tt> is represented
 * as <tt>00001111</tt> in binary two's-complement form is many architectures.
 * Writing only the 4 LSBs and subsequently reading them would result
 * <tt>1111</tt>, whose decimal value is <tt>-1</tt>.
 * @code
 * int x = 15;
 * bitarr_write_int(dest, 0, x, 4);
 * x = bitarr_read_int(dest, 0, x, 4);
 * printf("x = %d", x); // prints: x = -1
 * @endcode
 *
 * @param dest The destination bitarray
 * @param from_bit The initial position to be (over)written in the
 *                 destination bitarray
 * @param val The source int value.
 * @param nbits The number of bits to be written.
 *
 * @see bitarr_read_int
 */
void bitarr_write_int( byte_t *dest, size_t from_bit, int val,
                       size_t nbits );


/**
 * @brief Writes the @p nbits least significant bits of an unsigned int @p val
 * to a bitarray @p dest.
 *
 * @see bitarr_write_int for similar details.
 */
void bitarr_write_uint( byte_t *dest, size_t from_bit, unsigned int val,
                        size_t nbits );


/**
 * @brief Writes the @p nbits least significant bits of long @p val
 * to a bitarray @p dest.
 *
 * @see bitarr_write_int for similar details.
 */
void bitarr_write_long( byte_t *dest, size_t from_bit, long val,
                        size_t nbits );


/**
 * @brief Writes the @p nbits least significant bits of an unsigned long @p val
 * to a bitarray @p dest.
 *
 * @see bitarr_write_int for similar details.
 */
void bitarr_write_ulong( byte_t *dest, size_t from_bit, unsigned long val,
                         size_t nbits);

/**
 * @brief Writes the @p nbits least significant bits of a long long @p val
 * to a bitarray @p dest.
 *
 * @see bitarr_write_int for similar details.
 */
void bitarr_write_llong( byte_t *dest, size_t from_bit, long long val,
                         size_t nbits );


/**
 * @brief Writes the @p nbits least significant bits of an unsigned long long
 * @p val to a bitarray @p dest.
 *
 * @see bitarr_write_int for similar details.
 */
void bitarr_write_ullong( byte_t *dest, size_t from_bit,
                          unsigned long long val, size_t nbits );


/**
 * @brief Writes the @p nbits least significant bits of a byte_t @p val
 * to a bitarray @p dest.
 *
 * @see bitarr_write_int for similar details.
 */
void bitarr_write_byte_t( byte_t *dest, size_t from_bit, byte_t val,
                          size_t nbits );


/**
 * @brief Writes the @p nbits least significant bits of a size_t @p val
 * to a bitarray @p dest.
 *
 * @see bitarr_write_int for similar details.
 */
void bitarr_write_size_t( byte_t *dest, size_t from_bit, size_t val,
                          size_t nbits );


/**
 * @brief Reads @p src[@p from_bit:@p from_bit+@p nbits] as a signed char.
 *
 * @see bitarr_read_int for similar details
 */
char bitarr_read_char( const byte_t *src, size_t from_bit,
                       size_t nbits );


/**
 * @brief Reads @p src[@p from_bit:@p from_bit+@p nbits] as an unsigned char.
 *
 * @see bitarr_read_int for similar details
 */
unsigned char bitarr_read_uchar( const byte_t *src, size_t from_bit,
                                 size_t nbits );


/**
 * @brief Reads @p src[@p from_bit:@p from_bit+@p nbits] as a signed short.
 *
 * @see bitarr_read_int for similar details
 */
short bitarr_read_short(const byte_t *src, size_t from_bit,
                        size_t nbits);


/**
 * @brief Reads @p src[@p from_bit:@p from_bit+@p nbits] as an unsigned short.
 *
 * @see bitarr_read_int for similar details
 */
unsigned short bitarr_read_ushort(const byte_t *src, size_t from_bit,
                                  size_t nbits);


/**
 * @brief Reads @p src[@p from_bit:@p from_bit+@p nbits] as a signed integer.
 *
 * This utility function is usually used in conjunction with bitarr_write_int
 * for tightly storing ints using as few bits as necessary.
 *
 * @warning If not enough bits are written, information may be lost concerning
 * the magnitude and/or signal of the value previously written.
 * For example, the decimal value <tt>26</tt> is represented
 * as <tt>00011010</tt> and so, only the <tt>5</tt> LSBs are sufficient for
 * representing its magnitude. However, the <b>signed</b> 5-bit int
 * <tt>11010</tt> corresponds to the decimal value <tt>-6</tt> in binary
 * two's-complement form.
 * <b>The default implementation of this function assumes two's-complement
 * representation</b>.
 * @code
 * int x = 26;
 * bitarr_write_int(dest, 0, 5);
 * x = bitarr_read_int(dest, 0, 5);
 * printf("x = %d", x); // prints: x = -6
 * @endcode
 *
 * @param src The source bitarray.
 * @param from_bit The position at which read begins.
 * @param nbits The number of bits to be written.
 *
 * @see bitarr_write_int
 */
int bitarr_read_int(const byte_t *src, size_t from_bit,
                    size_t nbits);


/**
 * @brief Reads @p src[@p from_bit:@p from_bit+@p nbits] as an unsigned short.
 *
 * @see bitarr_read_int for similar details
 */
unsigned int bitarr_read_uint(const byte_t *src, size_t from_bit,
                              size_t nbits);


/**
 * @brief Reads @p src[@p from_bit:@p from_bit+@p nbits] as a signed long.
 *
 * @see bitarr_read_int for similar details
 */
long bitarr_read_long(const byte_t *src, size_t from_bit,
                      size_t nbits);


/**
 * @brief Reads @p src[@p from_bit:@p from_bit+@p nbits] as an unsigned long.
 *
 * @see bitarr_read_int for similar details
 */
unsigned long bitarr_read_ulong(const byte_t *src, size_t from_bit,
                                size_t nbits);


/**
 * @brief Reads @p src[@p from_bit:@p from_bit+@p nbits] as a signed long long.
 *
 * @see bitarr_read_int for similar details
 */
long long bitarr_read_llong(const byte_t *src, size_t from_bit,
                            size_t nbits);


/**
 * @brief Reads @p src[@p from_bit:@p from_bit+@p nbits] as an unsigned long long.
 *
 * @see bitarr_read_int for similar details
 */
unsigned long long bitarr_read_ullong(const byte_t *src,
                                      size_t from_bit, size_t nbits);


/**
 * @brief Reads @p src[@p from_bit:@p from_bit+@p nbits] as a size_t.
 *
 * @see bitarr_read_int for similar details
 */
size_t bitarr_read_size_t(const byte_t *src, size_t from_bit,
                          size_t nbits);


/**
 * @brief Reads @p src[@p from_bit:@p from_bit+@p nbits] as a byte_t.
 *
 * @see bitarr_read_int for similar details
 */
byte_t bitarr_read_byte_t(const byte_t *src, size_t from_bit,
                          size_t nbits);


#endif
