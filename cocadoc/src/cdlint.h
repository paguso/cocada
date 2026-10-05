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

#ifndef CDLINT_H
#define CDLINT_H

#include <stddef.h>

#include "cdfile.h"
#include "cdsym.h"
#include "vec.h"

/**
 * @file cdlint.h
 * @author Paulo Fonseca
 * @brief Checks a header against the COCADA documentation comment style.
 *
 * The rules are described in `doc/comment-style.md`. All deviations are
 * warnings. Checks on the text of each comment are done by #cddoc_parse;
 * this module adds the checks that need the declarations: file comments
 * (DC3), undocumented declarations and members (DC4), `@param` and
 * `@return` against the actual parameters and return type (DC8), and the
 * targets of `@see` (DC10), `#name` and `@p name` (DC11).
 */


/**
 * @brief A style warning.
 */
typedef struct {
	size_t line;       /**< 1-based line in the file */
	char *name;        /**< Name of the declaration concerned (heap) */
	const char *rule;  /**< Rule ID, e.g. "DC4" (static) */
	char *msg;         /**< Message (heap) */
} cdwarn;


/**
 * @brief Checks the documentation comments of a header.
 * @param file The header.
 * @param tab The symbol table to resolve references with, or NULL to skip
 *        the reference checks.
 * @return @move The warnings (vec of #cdwarn), ordered by line.
 *         Destroy with #cdwarn_vec_free.
 */
vec *cdlint(const cdfile *file, const cdsymtab *tab);


/**
 * @brief Destroys a vector returned by #cdlint.
 * @param @move warns The vector.
 */
void cdwarn_vec_free(vec *warns);

#endif
