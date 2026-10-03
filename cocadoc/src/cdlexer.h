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

#ifndef CDLEXER_H
#define CDLEXER_H

#include <stddef.h>

#include "vec.h"

/**
 * @file cdlexer.h
 * @author Paulo Fonseca
 * @brief Lightweight C tokenizer for cocadoc.
 *
 * This is not a full C lexer. It splits a C source buffer into
 * the coarse tokens needed to pair documentation comments with
 * the declarations they document:
 *
 * - Documentation comments `/ **  ... * /` (#CDT_DOC) and trailing
 *   member documentation `/ **< ... * /` (#CDT_DOC_POST) are kept as
 *   single tokens. Ordinary comments are discarded.
 * - A preprocessor directive, including backslash-continued lines,
 *   is a single #CDT_PP token.
 * - Identifiers/keywords, numbers, string and char literals.
 * - Every other non-blank character is a single #CDT_PUNCT token.
 *
 * Tokens do not own text. They are (offset, length) slices of the
 * source buffer, which must outlive them.
 */

/**
 * @brief Token types.
 */
typedef enum {
	CDT_EOF = 0,    /**< End of input */
	CDT_IDENT,      /**< Identifier or keyword */
	CDT_NUMBER,     /**< Numeric literal */
	CDT_STRING,     /**< String literal (with quotes) */
	CDT_CHAR,       /**< Char literal (with quotes) */
	CDT_PUNCT,      /**< Single punctuation char */
	CDT_PP,         /**< Whole preprocessor directive */
	CDT_DOC,        /**< Documentation comment `/ ** ... * /` */
	CDT_DOC_POST    /**< Trailing documentation comment `/ **< ... * /` */
} cdtoken_type;


/**
 * @brief A token, as a slice of the source buffer.
 */
typedef struct {
	cdtoken_type type; /**< Token type */
	size_t pos;        /**< Offset of the first char in the source */
	size_t len;        /**< Length in chars */
	size_t line;       /**< 1-based line of the first char */
} cdtoken;


/**
 * @brief Lexer type (opaque).
 */
typedef struct _cdlexer cdlexer;


/**
 * @brief Creates a lexer over @p src.
 * @param src (**no transfer**) The source buffer. Must outlive the lexer
 *        and all tokens produced by it.
 * @param len The length of @p src.
 */
cdlexer *cdlexer_new(const char *src, size_t len);


/**
 * @brief Destructor.
 */
void cdlexer_free(cdlexer *self);


/**
 * @brief Returns the next token. After the end of input, returns
 *        #CDT_EOF tokens indefinitely.
 */
cdtoken cdlexer_next(cdlexer *self);


/**
 * @brief Tokenizes the whole buffer @p src.
 * @return A vector of ::cdtoken, not including the final #CDT_EOF.
 */
vec *cdlex_all(const char *src, size_t len);


/**
 * @brief Returns the name of a token type, for debugging.
 */
const char *cdtoken_type_name(cdtoken_type type);

#endif
