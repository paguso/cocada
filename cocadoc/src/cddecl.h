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

#ifndef CDDECL_H
#define CDDECL_H

#include <stddef.h>

#include "new.h"
#include "vec.h"

/**
 * @file cddecl.h
 * @author Paulo Fonseca
 * @brief Pairs documentation comments with the declarations they document.
 *
 * The matcher walks the token stream produced by #cdlex_all and, for
 * every documentation comment found at file scope, collects the
 * declaration that immediately follows it. It does not parse C: it only
 * balances brackets and looks at a few landmarks (`typedef`, aggregate
 * keywords, the first top-level parenthesis, the final `;`) to classify
 * the declaration and find its name.
 *
 * Aggregates (struct/union/enum, possibly typedef'd) also get their
 * members, each with its own leading `/ ** * /` or trailing `/ **< * /`
 * documentation, if any. Members are recorded whether documented or not.
 *
 * All file-scope declarations are returned, documented or not (#cddecl.doc
 * is NULL for undocumented ones), except include guards and undocumented
 * macro invocations (macro-generated APIs are not handled yet).
 */

/**
 * @brief Declaration kinds.
 */
typedef enum {
	CDD_FILE = 0,   /**< `@file` block (no declaration) */
	CDD_FUNC,       /**< Function prototype or definition */
	CDD_MACRO,      /**< `#define` */
	CDD_MACROCALL,  /**< File-scope macro invocation, e.g. `DECL_TRAIT(a, b)` */
	CDD_TYPEDEF,    /**< Typedef (including typedef'd aggregates) */
	CDD_STRUCT,     /**< Struct definition */
	CDD_UNION,      /**< Union definition */
	CDD_ENUM,       /**< Enum definition */
	CDD_VAR,        /**< Variable */
	CDD_MEMBER,     /**< Struct/union field or enum constant */
	CDD_UNKNOWN     /**< Could not be classified */
} cddecl_kind;


/**
 * @brief A declaration and its documentation comment.
 */
typedef struct {
	cddecl_kind kind; /**< Declaration kind */
	char *name;       /**< Declared name (heap). For #CDD_FILE, the file name. */
	char *sig;        /**< Declaration text, with bodies elided as `{...}` (heap) */
	char *doc;        /**< Raw documentation comment text (heap) or NULL */
	size_t line;      /**< 1-based line of the declaration */
	size_t doc_line;  /**< 1-based line where the doc comment starts (0 if none) */
	vec *members;     /**< Members of aggregates (vec of #cddecl), or NULL */
} cddecl;


/**
 * @brief Finaliser: frees the strings and members of a #cddecl.
 * @param ptr The declaration.
 * @param fnr Not used.
 * @see new.h
 */
void cddecl_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Finds the declarations in a source buffer.
 * @param src The source buffer.
 * @param toks The tokens of @p src, as returned by #cdlex_all.
 * @return @move A vector of #cddecl in source order. Destroy with
 *         #cddecl_vec_free.
 */
vec *cddecl_match(const char *src, const vec *toks);


/**
 * @brief Destroys a vector returned by #cddecl_match.
 * @param @move decls The vector.
 */
void cddecl_vec_free(vec *decls);


/**
 * @brief Returns the name of a declaration kind.
 * @param kind The kind.
 * @return The name (static string).
 */
const char *cddecl_kind_name(cddecl_kind kind);

#endif
