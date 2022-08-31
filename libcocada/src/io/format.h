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


#ifndef FORMAT_H
#define FORMAT_H

#include <stdio.h>

#include "trait.h"

/**
 * @file format.h
 * @author Paulo Fonseca
 * @brief Format trait
 *
 * Objects implementing this trait print formatted text to an output stream.
 * They are usually wrappers that provide a common interface for printing
 * other objects, relieving these from having to implement specific
 * I/O operation themselves.
 */


/**
 * @brief format trait type
 */
typedef struct _format format;

/**
 * @brief format virtual table
 */
typedef struct {
	void (*fprint)(format *, FILE *);
} format_vt;


struct _format {
	format_vt vt;
	void *impltor;
};

/**
 * @brief Prints formatted text to the standard output.
 */
void format_print(format *self);

/**
 * @brief Prints formatted text to a given @p stream.
 */
void format_fprint(format *self, FILE *stream);

#endif