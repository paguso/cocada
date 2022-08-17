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

#ifndef MEMDBG_H
#define MEMDBG_H

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * @file memdbg.h
 * @brief Memory allocation debug
 * @author Paulo Fonseca
 *
 * Including this header causes the stdlib.h memory allocation/deallocation
 * functions (malloc, calloc, realloc and free) be replaced by
 * corresponding wrapper functions that keep a tally of the allocated
 * memory chunks along with other debug info.
 *
 * <b>This change only takes place if the MEM_DEBUG macro is defined</b>.
 * This can be done locally in the code, but most likely this is
 * done via a compiler option. For example, in gcc
 *
 * ```
 * gcc -DMEM_DEBUG
 * ```
 * To deactivate memory debugging locally for a given compilation unit, the macro
 *
 * ```C
 * #define MEM_DEBUG_OFF
 * ```
 * must be inserted at the begining of the file, before this header gets
 * included. At the end of the file, the macro should be undefined
 *
 * ```C
 * #undef MEM_DEBUG_OFF
 * ```
 * to re-enable memory debugging.
 */


/**
 * @brief malloc wrapper. Not meant do be called directly.
 */
extern void *memdbg_malloc(size_t size, char *file, int line);


/**
 * @brief calloc wrapper. Not meant do be called directly.
 */
extern void *memdbg_calloc(size_t nmemb, size_t size, char *file, int line);


/**
 * @brief realloc wrapper. Not meant do be called directly.
 */
extern void *memdbg_realloc(void *ptr, size_t size, char *file, int line);


/**
 * @brief free wrapper. Not meant do be called directly.
 */
extern void memdbg_free(void *ptr, char *file, int line);


/**
 * @brief Print memory usage stats.
 * @param stream Output stream
 * @param print_chunks Display size and address of all allocated chunks
 */
extern void memdbg_print_stats(FILE *stream, bool print_chunks);


/**
 * @brief Resets the memory tally.
 */
extern void memdbg_reset();


/**
 * @brief Returns the total memory (in bytes) accounted for in the tally.
 */
extern size_t memdbg_total();


/**
 * @brief Return the current number of allocated chunks in the tally
 */
extern size_t memdbg_nchunks();


/**
 * @brief Returns true if the memory tally is empty, and false otherwise.
 * @warn If true, it doesn't mean that no memory is allocated, but only
 * that none is accounted for in the tally.
 */
extern bool memdbg_is_empty();



#ifdef MEM_DEBUG

#ifndef MEM_DEBUG_OFF

#define malloc(size) (memdbg_malloc(size, __FILE__ , __LINE__))

#define calloc(nmemb, size) (memdbg_calloc(nmemb, size, __FILE__ , __LINE__))

#define realloc(ptr, size) (memdbg_realloc(ptr, size, __FILE__ , __LINE__))

#define free(ptr) (memdbg_free(ptr,  __FILE__ , __LINE__))

#endif

#endif


#endif