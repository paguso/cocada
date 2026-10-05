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

#ifndef CDSYM_H
#define CDSYM_H

#include <stddef.h>

#include "cddecl.h"
#include "cdfile.h"
#include "vec.h"

/**
 * @file cdsym.h
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 * @brief Symbol table: what the documented names refer to.
 *
 * Indexes the declarations of a set of files by name:
 *
 * - functions, macros, types and variables by their names;
 * - struct and union fields as `type.field`;
 * - enum constants both as `type.CONSTANT` and as `CONSTANT` (in C, enum
 *   constants are global names);
 * - files by their file name, e.g. `vec.h`.
 *
 * A name may be declared more than once (in different files). References
 * are then resolved to the declaration in the referring file, if any.
 */


/**
 * @brief A named declaration.
 */
typedef struct {
	char *name;            /**< Indexed name, e.g. "vec_new", "cddoc.brief" (heap) */
	cddecl_kind kind;      /**< Kind of declaration */
	const cdfile *file;    /**< Declaring file */
	const cddecl *decl;    /**< The declaration (for #CDD_FILE, the file comment, or NULL) */
	const cddecl *parent;  /**< For members, the declaration of their type; else NULL */
} cdsym;


/**
 * @brief Symbol table type (opaque).
 */
typedef struct _cdsymtab cdsymtab;


/**
 * @brief Builds the symbol table of a set of files.
 * @param files The files (vec of #cdfile *). They must outlive the table.
 * @return @move The table.
 */
cdsymtab *cdsymtab_new(const vec *files);


/**
 * @brief Destructor.
 * @param @move self The table.
 */
void cdsymtab_free(cdsymtab *self);


/**
 * @brief Finds what a name refers to.
 *
 * If the name is declared more than once, the declaration in @p from is
 * preferred.
 *
 * @param self The table.
 * @param name The name, e.g. "vec_new", "cddoc.brief" or "vec.h".
 * @param from The referring file, or NULL.
 * @param ncands Set to the number of files declaring @p name (may be NULL).
 * @return The symbol, or NULL if @p name is not declared.
 */
const cdsym *cdsymtab_resolve(const cdsymtab *self, const char *name,
                              const cdfile *from, size_t *ncands);


/**
 * @brief Returns the number of symbols.
 * @param self The table.
 * @return The number of symbols.
 */
size_t cdsymtab_size(const cdsymtab *self);


/**
 * @brief Returns the symbol at position @p i, in the order the files and
 * declarations were indexed.
 * @param self The table.
 * @param i The position.
 * @return The symbol.
 */
const cdsym *cdsymtab_get(const cdsymtab *self, size_t i);

#endif
