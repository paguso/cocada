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

#ifndef CDCONFIG_H
#define CDCONFIG_H

#include <stddef.h>

#include "vec.h"

/**
 * @file cdconfig.h
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 * @brief The configuration file, `cocadoc.config`.
 *
 * A configuration file has one `KEY = value` per line. Text after `//` is
 * a comment. List values are separated by spaces. An empty value means
 * unset. Paths are relative to the directory of the configuration file.
 *
 * ```
 * PROJECT_NAME = COCADA
 * VERSION = 0.5.0
 * OUTPUT_TYPE = markdown
 * OUTPUT_DIRECTORY = ./doc/markdown
 * BASE_DIR = ./
 * MODULES = libcocada libcocadaapp libcocadabio libcocadasketch libcocadastrproc
 * EXCLUDE_DIRS = thrdpty
 * EXCLUDE_FILES = *deprecated.h
 * ```
 *
 * The headers to document are the `*.h` files found recursively in the
 * `src/` directory of each module, `BASE_DIR/MODULE/src/`, except those in
 * directories named in `EXCLUDE_DIRS` (at any depth) and those whose file
 * names match a pattern of `EXCLUDE_FILES`.
 */


/**
 * @brief A configuration.
 */
typedef struct {
	char *project_name;  /**< `PROJECT_NAME` (heap), or NULL */
	char *version;       /**< `VERSION` (heap), or NULL */
	char *output_type;   /**< `OUTPUT_TYPE` (heap), "markdown" by default */
	char *output_dir;    /**< `OUTPUT_DIRECTORY` as a usable path (heap), or NULL */
	char *base_dir;      /**< `BASE_DIR` as a usable path (heap) */
	vec *modules;        /**< `MODULES` (vec of heap char *) */
	vec *exclude_dirs;   /**< `EXCLUDE_DIRS` (vec of heap char *) */
	vec *exclude_files;  /**< `EXCLUDE_FILES` patterns (vec of heap char *) */
} cdconfig;


/**
 * @brief Reads a configuration file.
 * @param path The file path.
 * @param err Where to write an error message, if any.
 * @param errsz The size of @p err.
 * @return @move The configuration, or NULL if the file cannot be read or
 *         is invalid (the reason is written to @p err).
 */
cdconfig *cdconfig_load(const char *path, char *err, size_t errsz);


/**
 * @brief Destructor.
 * @param @move self The configuration.
 */
void cdconfig_free(cdconfig *self);


/**
 * @brief Finds the headers to document.
 * @param self The configuration.
 * @return @move The header paths (vec of heap char *), module by module
 *         in the order of `MODULES`, sorted within each module.
 */
vec *cdconfig_find_headers(const cdconfig *self);


/**
 * @brief Joins two paths.
 *
 * Leading `./` in @p rel is dropped, and @p base is omitted if it is
 * empty or ".".
 *
 * @param base The base path.
 * @param rel A path relative to @p base, or an absolute path (returned
 *        as is).
 * @return @move The joined path.
 */
char *cdpath_join(const char *base, const char *rel);

#endif
