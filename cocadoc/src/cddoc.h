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

#include <stdbool.h>
#include <stddef.h>

#include "vec.h"

/**
 * @file cddoc.h
 * @author Paulo Fonseca
 * @brief Parser for documentation comments.
 *
 * Turns the raw text of a `/ ** ... * /` (or `/ **< ... * /`) comment into
 * a structured #cddoc. The comment syntax (delimiters and the leading
 * `*` column) is removed, and Doxygen-style block commands are split into
 * fields:
 *
 * command                     | field
 * ----------------------------|----------------------------------
 * `@brief`                    | #cddoc.brief
 * `@param [@move] name text`  | #cddoc.params
 * `@return [@move] text`      | #cddoc.ret
 * `@see`                      | #cddoc.see
 * `@warning`                  | #cddoc.warnings
 * `@note`                     | #cddoc.notes
 * `@deprecated`               | #cddoc.deprecated
 * `@author`                   | #cddoc.authors
 *
 * A block command extends until a blank line or the next block command,
 * and may also start in the middle of a line. Text outside block commands
 * goes to #cddoc.details. Member docs (`/ **< ... * /`) have no block
 * commands: their first sentence is the brief.
 *
 * All text is kept as Markdown. Inline references (`@p x`, `#sym`) are
 * left as they are, to be resolved later. Nothing inside code spans or
 * code blocks is interpreted.
 *
 * The comment is also checked against the COCADA documentation comment
 * style (`doc/comment-style.md`). Deviations are reported in
 * #cddoc.diags, tagged with the rule they break (e.g. `DC6`). The parser
 * still accepts the deprecated forms it reports (e.g. `@returns`,
 * `(**move**)`), so that the documentation is complete in the meantime.
 */


/**
 * @brief A documented parameter.
 */
typedef struct {
	char *name;          /**< Parameter name (heap) */
	bool move;           /**< Ownership moves to the function (`@param @move`) */
	char *desc;          /**< Description (heap) */
	size_t line;         /**< 0-based line of the `@param` in the comment */
} cdparam;


/**
 * @brief A deviation from the documentation comment style.
 */
typedef struct {
	const char *rule;    /**< Rule ID, e.g. "DC6" (static) */
	size_t line;         /**< 0-based line in the comment */
	char *msg;           /**< Message (heap) */
} cddiag;


/**
 * @brief Kinds of references in a comment.
 */
typedef enum {
	CDR_SYMBOL = 0, /**< `#name` or `#type.member` in the text */
	CDR_PARAM,      /**< `@p name` */
	CDR_SEE         /**< An entry of a `@see` list */
} cdref_kind;


/**
 * @brief A reference found in a comment, to be resolved against the
 * declarations.
 */
typedef struct {
	cdref_kind kind;     /**< Kind of reference */
	char *target;        /**< Referenced name, without `#` (heap) */
	size_t line;         /**< 0-based line in the comment */
} cdref;


/**
 * @brief A parsed documentation comment.
 *
 * All strings are heap-allocated Markdown and never NULL, except
 * `ret` and `deprecated`, which are NULL when absent.
 */
typedef struct {
	char *brief;       /**< Brief description ("" if none) */
	char *details;     /**< Detailed description ("" if none) */
	vec  *params;      /**< Parameters (vec of #cdparam) */
	char *ret;         /**< Return value description, or NULL */
	bool ret_move;     /**< Ownership of the return value moves to the caller */
	vec  *see;         /**< `@see` entries (vec of char *) */
	vec  *warnings;    /**< Warnings (vec of char *) */
	vec  *notes;       /**< Notes (vec of char *) */
	char *deprecated;  /**< Deprecation text ("" if no text), or NULL */
	vec  *authors;     /**< Authors (vec of char *) */
	vec  *refs;        /**< References, in order (vec of #cdref) */
	vec  *diags;       /**< Style deviations (vec of #cddiag) */
} cddoc;


/**
 * @brief Parses a documentation comment.
 * @param raw The comment text, including its delimiters, e.g. as stored
 *        in #cddecl.
 * @param len The length of @p raw.
 * @return @move The parsed comment.
 */
cddoc *cddoc_parse(const char *raw, size_t len);


/**
 * @brief Destructor.
 * @param @move self The parsed comment.
 */
void cddoc_free(cddoc *self);


/**
 * @brief Returns the comment text with the comment syntax removed: the
 * delimiters, the leading `*` column and the common indentation.
 * @param raw The comment text.
 * @param len The length of @p raw.
 * @return @move A vector of lines (vec of heap char *), with no leading or
 *         trailing blank lines.
 */
vec *cddoc_strip(const char *raw, size_t len);



#endif
