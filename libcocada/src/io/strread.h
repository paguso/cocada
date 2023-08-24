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

#ifndef STRREAD_H
#define STRREAD_H

/**
 * @file strread.h
 * @brief String Read trait
 * @author Paulo Fonseca
 *
 * The String Read trait allows for reading characters and strings
 * of characters from a source stream. Implementors of this trait
 * are called String Readers.
 *
 * @see trait.h
 */

#include <stdbool.h>
#include <stddef.h>


typedef struct _strread strread;

/**
 * @brief String reader virtual table
 */
typedef struct {
	void    (*reset) (strread *self);
	int		(*getc) (strread *self);
	int		(*ungetc) (strread *self);
	size_t  (*read_str) (strread *self, char *dest, size_t n);
	size_t  (*read_str_until) (strread *self, char *dest, char delim);
}  strread_vt;


struct _strread {
	strread_vt *vt;
	void *impltor;
};


strread_vt strread_vt_new();


/**
 * @brief Resets the reader, that is moves cursor to initial position,
 * if supported by the underlying stream.
 */
void strread_reset(strread *self);


/**
 * @brief Reads the next char from a stream.
 * @returns The next character as an int, or EOF if the stream has
 *          reached its end.
 *
 */
int strread_getc(strread *self);


/**
 * @brief Attempts to put back the last char read into the stream to
 * be read by subsequent read operations.
 * If the base stream implementing the trait does not support this operation
 * or if no char has been read from the stream, the function returns 0.
 * Otherwise, it returns 1
 *
 * @returns int 0 if the operation is not supported or no char has been read,
 * 		   1 otherwise.
 * @warning Only one char can be put back into the stream. Once a char is put back,
 * a subsequent call to this function will return 0.
 */
int strread_ungetc(strread *self);


/**
 * @brief Attempts to read the next @p n chars into @p dest.
 *        Less than @p n characters can be read if the stream reaches its end.
 * @returns The number of chars actually read.
 */
size_t strread_read_str(strread *self, char *dest, size_t n);


/**
 * @brief Attempts to read the next chars into the @p dest until
 * 		  the next occurrence of the delimiter @p delim is found,
 * 		  or the end of the stream is reached.
 * @returns The number of chars actually read.
 */
size_t strread_read_str_until(strread *self, char *dest, char delim);



#endif