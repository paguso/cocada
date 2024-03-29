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

#ifndef BYTEARRAY_H
#define BYTEARRAY_H

#include <limits.h>
#include <stdlib.h>
#include <stdint.h>

#include "bitbyte.h"

/**
 * @file   bytearr.h
 * @author Paulo Fonseca
 *
 * @brief Byte array with read/write operations
 */


/**
 * @brief Creates a new bytearray of given length.
 * The array is filled with zeros by default.
 */
byte_t *bytearr_new(size_t len);


/**
 * @brief Sets each position  @p ba[j] to @p val, for j in the range
 * @p from <= j < @p to.
 */
void bytearr_fill(byte_t *ba, size_t from, size_t to, byte_t val);

/**
 * @brief Flips the bytes of the array @p ba in place, that is, swaps the
 * contents of positions @p ba[j] <--> @p ba[size-j],
 * for 0 <= j < @p size.
 */
void bytearr_reverse(byte_t *ba, size_t size);


/**
 * @brief Byte print format
 */
typedef enum {
	BIN = 0, /**< binaray */
	HEX = 1  /**< Hexadecimal */
}
byte_format;


/**
 * @brief Prints a string representations of a byte array do std output.
 * @param ba The array to print.
 * @param nbytes Array size in bytes.
 * @param bytes_per_line Number of bytes to be printed per line.
 * @param leftmargin Left margin string to be print at the start of each line.
 */
void bytearr_print(const byte_t *ba, size_t nbytes, byte_format fmt,
                   size_t bytes_per_line, const char *leftmargin);



/**
 * @brief Reads @p src[@p from_byte:@p from_bit+@p nbytes] as a
 *        signed char.
 *
 * @see bitarr_read_int for silimar remarks concerning the loss of information.
 * @see bytearr_write_char
 */
char bytearr_read_char(const byte_t *src, size_t from_byte,
                       size_t nbytes);



/**
 * @brief Reads @p src[@p from_byte:@p from_bit+@p nbytes] as an
 *        unsigned char.
 *
 * @see bitarr_read_int for silimar remarks concerning the loss of information.
 * @see bytearr_write_uchar
 */
unsigned char bytearr_read_uchar(const byte_t *src, size_t from_byte,
                                 size_t nbytes);


/**
 * @brief Reads @p src[@p from_byte:@p from_bit+@p nbytes] as a
 *        signed short.
 *
 * @see bitarr_read_int for silimar remarks concerning the loss of information.
 * @see bytearr_write_short.
 */
short bytearr_read_short(const byte_t *src, size_t from_byte,
                         size_t nbytes);


/**
 * @brief Reads @p src[@p from_byte:@p from_bit+@p nbytes] as an
 *        unsigned short.
 *
 * @see bitarr_read_int for silimar remarks concerning the loss of information.
 * @see bytearr_write_ushort.
 */
unsigned short bytearr_read_ushort(const byte_t *src, size_t from_byte,
                                   size_t nbytes);


/**
 * @brief Reads @p src[@p from_byte:@p from_bit+@p nbytes] as a
 *        signed int.
 *
 * This utility function is usually used in conjunction with bytetarr_write_int
 * for tightly storing ints using as few bits as necessary while respecting
 * byte boundaries for the sake of speed.
 *
 * @warning If not enough bytes are read, information may be lost concerning
 * the magnitude and/or signal of the value previously written.
 *
 * @param src The source byte array.
 * @param from_bytet The position at which read begins.
 * @param nbytes The number of bytes to be read.
 *
 * @see bitarr_read_int for silimar remarks concerning the loss of information.
 * @see bytearr_write_int.
 */
int bytearr_read_int(const byte_t *src, size_t from_byte,
                     size_t nbytes);


/**
 * @brief Reads @p src[@p from_byte:@p from_bit+@p nbytes] as an
 *        unsigned short.
 *
 * @see bitarr_read_int for silimar remarks concerning the loss of information.
 * @see bytearr_write_uint.
 */
unsigned int bytearr_read_uint(const byte_t *src, size_t from_byte,
                               size_t nbytes);


/**
 * @brief Reads @p src[@p from_byte:@p from_bit+@p nbytes] as a
 *        signed long int.
 *
 * @see bitarr_read_int for silimar remarks concerning the loss of information.
 * @see bytearr_write_long.
 */
long bytearr_read_long(const byte_t *src, size_t from_byte,
                       size_t nbytes);


/**
 * @brief Reads @p src[@p from_byte:@p from_bit+@p nbytes] as an
 *        unsigned long int.
 *
 * @see bitarr_read_int for silimar remarks concerning the loss of information.
 * @see bytearr_write_ulong.
 */
unsigned long bytearr_read_ulong(const byte_t *src, size_t from_byte,
                                 size_t nbytes);


/**
 * @brief Reads @p src[@p from_byte:@p from_bit+@p nbytes] as a
 *        signed long long int.
 *
 * @see bitarr_read_int for silimar remarks concerning the loss of information.
 * @see bytearr_write_longlong.
 */
long long bytearr_read_llong(const byte_t *src, size_t from_byte,
                             size_t nbytes);


/**
 * @brief Reads @p src[@p from_byte:@p from_bit+@p nbytes] as an
 *        unsigned long long int.
 *
 * @see bitarr_read_int for silimar remarks concerning the loss of information.
 * @see bytearr_write_ulonglong.
 */
unsigned long long bytearr_read_ullong(const byte_t *src,
                                       size_t from_byte, size_t nbytes);


/**
 * @brief Reads @p src[@p from_byte:@p from_bit+@p nbytes] as a size_t.
 *
 * @see bitarr_read_int for silimar remarks concerning the loss of information.
 * @see bytearr_write_size.
 */
size_t bytearr_read_size_t(const byte_t *src, size_t from_byte,
                           size_t nbytes);


/**
 * @brief Generic write operation:
 *        @p dest[@p from_byte_dest:@p from_byte_dest+@p nbytes]
 *        = @p src[@p from_byte_src:@p from_byte_src+@p nbits]
 * @param dest The destination byte array
 * @param from_byte_dest The initial position to be (over)written in
 *                      the destination byte array
 * @param src The source byte array
 * @param from_byte_src The initial position to be read from the source array
 * @param nbytes The number of bytes to be written
 */
void bytearr_write(byte_t *dest, size_t from_byte_dest, const byte_t *src,
                   size_t from_byte_src, size_t nbytes);


/**
 * @brief Writes the @p nbytes least significant bytes of a
 *        char @p val to a bytearray @p dest.
 *
 * @see bitarr_write_int for similar remarks about loss of information.
 * @see bytearr_read_char
 */
void bytearr_write_char(byte_t *dest, size_t from_byte, char val,
                        size_t nbytes);


/**
 * @brief Writes the @p nbytes least significant bytes of an
 * unsigned char @p val to a bytearray @p dest.
 *
 * @see bitarr_write_int for similar remarks about loss of information.
 * @see bytearr_read_uchar
 */
void bytearr_write_uchar(byte_t *dest, size_t from_byte,
                         unsigned char val, size_t nbytes);


/**
 * @brief Writes the @p nbytes least significant bytes of a
 * short int @p val to a bytearray @p dest.
 *
 * @see bitarr_write_int for similar remarks about loss of information.
 * @see bytearr_read_short
 */
void bytearr_write_short(byte_t *dest, size_t from_byte, short val,
                         size_t nbytes);


/**
 * @brief Writes the @p nbytes least significant bytes of an
 * unsigned short int @p val to a bytearray @p dest.
 *
 * @see bitarr_write_int for similar remarks about loss of information.
 * @see bytearr_read_ushort
 */
void bytearr_write_ushort(byte_t *dest, size_t from_byte,
                          unsigned short val, size_t nbytes);

/**
 * @brief Writes the @p nbytes least significant bytes of an int @p val
 * to a bytearray @p dest.
 *
 * This utility function is usually used in conjunction with bytearr_read_int
 * for tightly storing ints using as few bits as necessary while respecting
 * byte boundaries for speed sake.
 *
 * @warning If not enough bytes are written, information may be lost concerning
 * the magnitude and/or signal of @p val.
 *
 * @param dest The destination bitarray
 * @param from_bit The initial position to be (over)written in the
 *                 destination bitarray
 * @param val The source int value.
 * @param nbits The number of bits to be written.
 *
 * @see bitarr_write_int for similar remarks about loss of information.
 * @see bytearr_read_int
 */
void bytearr_write_int(byte_t *dest, size_t from_byte, int val,
                       size_t nbytes);


/**
 * @brief Writes the @p nbytes least significant bytes of an unsigned int @p val
 * to a bytearray @p dest.
 *
 * @see bitarr_write_int for similar remarks about loss of information.
 * @see bytearr_read_uint
 */
void bytearr_write_uint(byte_t *dest, size_t from_byte, unsigned int val,
                        size_t nbytes);


/**
 * @brief Writes the @p nbytes least significant bytes of a long int @p val
 * to a bytearray @p dest.
 *
 * @see bitarr_write_int for similar remarks about loss of information.
 * @see bytearr_read_long
 */
void bytearr_write_long(byte_t *dest, size_t from_byte, long val,
                        size_t nbytes);


/**
 * @brief Writes the @p nbytes least significant bytes of an unsigned
 * long int @p val to a bytearray @p dest.
 *
 * @see bitarr_write_int for similar remarks about loss of information.
 * @see bytearr_read_ulong
 */
void bytearr_write_ulong(byte_t *dest, size_t from_byte,
                         unsigned long val, size_t nbytes);


/**
 * @brief Writes the @p nbytes least significant bytes of a
 * long long int @p val to a bytearray @p dest.
 *
 * @see bitarr_write_int for similar remarks about loss of information.
 * @see bytearr_read_longlong
 */
void bytearr_write_llong(byte_t *dest, size_t from_byte, long long val,
                         size_t nbytes);


/**
 * @brief Writes the @p nbytes least significant bytes of an
 * unsigned long long int @p val to a bytearray @p dest.
 *
 * @see bitarr_write_int for similar remarks about loss of information.
 * @see bytearr_read_ulonglong
 */
void bytearr_write_ullong(byte_t *dest, size_t from_byte,
                          unsigned long long val, size_t nbytes);


/**
 * @brief Writes the @p nbytes least significant bytes of an
 *        size_t @p val to a bytearray @p dest.
 *
 * @see bitarr_write_int for similar remarks about loss of information.
 * @see bytearr_read_size
 */
void bytearr_write_size_t(byte_t *dest, size_t from_byte, size_t val,
                          size_t nbytes);

#endif