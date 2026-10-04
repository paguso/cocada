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

#ifndef CDDOC_H
#define CDDOC_H

#include <stddef.h>

#include "vec.h"

/**
 * @file cddoc.h
 * @author Paulo Fonseca
 * @brief Parser for documentation comments.
 *
 * Turns the raw text of a `/ ** ... * /` (or `/ **< ... * /`) comment into
 * a structured ::cddoc. The comment syntax (delimiters and the leading
 * `*` column) is removed, and Doxygen-style block commands are split into
 * fields:
 *
 * command                     | field
 * ----------------------------|----------------------------------
 * `@brief`                    | cddoc::brief
 * `@param`                    | cddoc::params
 * `@return`, `@returns`       | cddoc::ret
 * `@see`                      | cddoc::see
 * `@warning`, `@warn`         | cddoc::warnings
 * `@note`                     | cddoc::notes
 * `@deprecated`               | cddoc::deprecated
 * `@author`                   | cddoc::authors
 * `@par title`                | a bold title in cddoc::details
 * `@code`/`@endcode`          | a fenced code block
 *
 * As in Doxygen, a block command extends until a blank line or the next
 * block command, and may also start in the middle of a line. Text outside
 * block commands goes to cddoc::details. Without `@brief`, the first
 * sentence of the details is the brief (Doxygen's `JAVADOC_AUTOBRIEF`).
 *
 * All text is kept as Markdown. Inline commands (`@p x`, `#sym`, `::sym`)
 * are left as they are, to be resolved later. Nothing inside code spans
 * or code blocks is interpreted.
 *
 * Problems found in the comment (unknown commands, likely misuse of
 * `@par`, `@param` without a name) are reported in cddoc::diags.
 */


/**
 * @brief Ownership of a parameter, as annotated in its description,
 * e.g. `@param buf (**move**) The buffer`.
 */
typedef enum {
	CDO_UNSPECIFIED = 0, /**< No annotation */
	CDO_NO_TRANSFER,     /**< `(**no transfer**)` */
	CDO_TRANSFER,        /**< `(**transfer**)` */
	CDO_MOVE             /**< `(**move**)` */
} cdownership;


/**
 * @brief A documented parameter.
 */
typedef struct {
	char *name;          /**< Parameter name (heap) */
	cdownership own;     /**< Ownership annotation */
	char *desc;          /**< Description, without the annotation (heap) */
} cdparam;


/**
 * @brief A parsed documentation comment.
 *
 * All strings are heap-allocated Markdown and never NULL, except
 * `ret` and `deprecated`, which are NULL when absent.
 */
typedef struct {
	char *brief;       /**< Brief description ("" if none) */
	char *details;     /**< Detailed description ("" if none) */
	vec  *params;      /**< Parameters (vec of ::cdparam) */
	char *ret;         /**< Return value description, or NULL */
	vec  *see;         /**< `@see` entries (vec of char *) */
	vec  *warnings;    /**< Warnings (vec of char *) */
	vec  *notes;       /**< Notes (vec of char *) */
	char *deprecated;  /**< Deprecation text ("" if no text), or NULL */
	vec  *authors;     /**< Authors (vec of char *) */
	vec  *diags;       /**< Problems found in the comment (vec of char *) */
} cddoc;


/**
 * @brief Parses a documentation comment.
 * @param raw (**no transfer**) The comment text, including its
 *        delimiters, e.g. as stored in ::cddecl.
 * @param len The length of @p raw.
 */
cddoc *cddoc_parse(const char *raw, size_t len);


/**
 * @brief Destructor.
 */
void cddoc_free(cddoc *self);


/**
 * @brief Returns the comment text with the comment syntax removed: the
 * delimiters, the leading `*` column and the common indentation.
 * @param raw (**no transfer**) The comment text.
 * @param len The length of @p raw.
 * @return A vector of lines (vec of heap char *), with no leading or
 *         trailing blank lines.
 */
vec *cddoc_strip(const char *raw, size_t len);


/**
 * @brief Returns the name of an ownership annotation, e.g. "move".
 */
const char *cdownership_name(cdownership own);

#endif
