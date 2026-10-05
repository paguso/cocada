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

#ifndef CDFILE_H
#define CDFILE_H

#include <stddef.h>

#include "vec.h"

/**
 * @file cdfile.h
 * @author Paulo Fonseca
 * @brief A source file loaded for documentation.
 *
 * Holds the contents of a file together with its tokens (#cdlex_all) and
 * declarations (#cddecl_match), which point into the contents.
 */


/**
 * @brief A loaded source file.
 */
typedef struct {
	char *path;    /**< Path, as given (heap) */
	char *name;    /**< File name, without directories (points into path) */
	char *src;     /**< Contents (heap, null-terminated) */
	size_t len;    /**< Length of the contents */
	vec *toks;     /**< Tokens (vec of #cdtoken) */
	vec *decls;    /**< Declarations (vec of #cddecl) */
} cdfile;


/**
 * @brief Reads, tokenizes and matches a file.
 * @param path The file path.
 * @return @move The file, or NULL if it cannot be read.
 */
cdfile *cdfile_load(const char *path);


/**
 * @brief Creates a file from a string, as if read from @p path.
 * @param path The path to use.
 * @param src The contents (copied).
 * @param len The length of @p src.
 * @return @move The file.
 */
cdfile *cdfile_new_from_str(const char *path, const char *src, size_t len);


/**
 * @brief Destructor.
 * @param @move self The file.
 */
void cdfile_free(cdfile *self);

#endif
