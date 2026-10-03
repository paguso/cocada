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

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "cdlexer.h"
#include "new.h"


struct _cdlexer {
	const char *src;
	size_t len;
	size_t pos;
	size_t line;
	bool at_line_start; // only blanks seen since last newline
};


cdlexer *cdlexer_new(const char *src, size_t len)
{
	cdlexer *ret = NEW(cdlexer);
	ret->src = src;
	ret->len = len;
	ret->pos = 0;
	ret->line = 1;
	ret->at_line_start = true;
	return ret;
}


void cdlexer_free(cdlexer *self)
{
	FREE(self);
}


static inline int peek(const cdlexer *lx, size_t off)
{
	return (lx->pos + off < lx->len) ? (unsigned char)lx->src[lx->pos + off] : EOF;
}


static inline void adv(cdlexer *lx)
{
	if (lx->src[lx->pos] == '\n') {
		lx->line++;
	}
	lx->pos++;
}


// assumes current pos is at "/*"; consumes up to and including "*/"
static void skip_block_comment(cdlexer *lx)
{
	adv(lx);
	adv(lx);
	while (lx->pos < lx->len) {
		if (peek(lx, 0) == '*' && peek(lx, 1) == '/') {
			adv(lx);
			adv(lx);
			return;
		}
		adv(lx);
	}
}


static void skip_line_comment(cdlexer *lx)
{
	while (lx->pos < lx->len && peek(lx, 0) != '\n') {
		adv(lx);
	}
}


// assumes current pos is at the opening quote
static void skip_quoted(cdlexer *lx)
{
	int q = peek(lx, 0);
	adv(lx);
	while (lx->pos < lx->len) {
		int c = peek(lx, 0);
		if (c == '\\' && lx->pos + 1 < lx->len) {
			adv(lx);
			adv(lx);
		} else if (c == q) {
			adv(lx);
			return;
		} else if (c == '\n') {
			return; // unterminated literal: stop at end of line
		} else {
			adv(lx);
		}
	}
}


// assumes current pos is at '#'; consumes logical line (not the final '\n')
static void skip_pp(cdlexer *lx)
{
	while (lx->pos < lx->len) {
		int c = peek(lx, 0);
		if (c == '\\' && peek(lx, 1) == '\n') {
			adv(lx);
			adv(lx);
		} else if (c == '\\' && peek(lx, 1) == '\r' && peek(lx, 2) == '\n') {
			adv(lx);
			adv(lx);
			adv(lx);
		} else if (c == '\n') {
			return;
		} else if (c == '/' && peek(lx, 1) == '*') {
			skip_block_comment(lx);
		} else if (c == '/' && peek(lx, 1) == '/') {
			skip_line_comment(lx);
		} else if (c == '"' || c == '\'') {
			skip_quoted(lx);
		} else {
			adv(lx);
		}
	}
}


static inline bool is_ident_start(int c)
{
	return c == '_' || isalpha(c);
}


static inline bool is_ident_char(int c)
{
	return c == '_' || isalnum(c);
}


cdtoken cdlexer_next(cdlexer *lx)
{
	while (lx->pos < lx->len) {
		int c = peek(lx, 0);

		if (c == '\n') {
			adv(lx);
			lx->at_line_start = true;
			continue;
		}
		if (isspace(c)) {
			adv(lx);
			continue;
		}

		cdtoken tk = {.type = CDT_EOF, .pos = lx->pos, .line = lx->line};

		if (c == '/' && peek(lx, 1) == '*') {
			// "/**/" is an empty plain comment, "/***..." is a separator line
			bool doc = peek(lx, 2) == '*' && peek(lx, 3) != '/'
			           && peek(lx, 3) != '*';
			bool post = peek(lx, 2) == '*' && peek(lx, 3) == '<';
			skip_block_comment(lx);
			if (!doc) {
				continue;
			}
			tk.type = post ? CDT_DOC_POST : CDT_DOC;
		} else if (c == '/' && peek(lx, 1) == '/') {
			skip_line_comment(lx);
			continue;
		} else if (c == '#' && lx->at_line_start) {
			skip_pp(lx);
			tk.type = CDT_PP;
		} else if (is_ident_start(c)) {
			while (lx->pos < lx->len && is_ident_char(peek(lx, 0))) {
				adv(lx);
			}
			// string/char literal prefixes: L"", u8"", etc
			if (peek(lx, 0) == '"' || peek(lx, 0) == '\'') {
				tk.type = (peek(lx, 0) == '"') ? CDT_STRING : CDT_CHAR;
				skip_quoted(lx);
			} else {
				tk.type = CDT_IDENT;
			}
		} else if (isdigit(c) || (c == '.' && isdigit(peek(lx, 1)))) {
			// pp-number: digits, letters, '.', and exponent signs
			while (lx->pos < lx->len) {
				int d = peek(lx, 0);
				if ((d == '+' || d == '-')
				        && strchr("eEpP", lx->src[lx->pos - 1])) {
					adv(lx);
				} else if (is_ident_char(d) || d == '.') {
					adv(lx);
				} else {
					break;
				}
			}
			tk.type = CDT_NUMBER;
		} else if (c == '"' || c == '\'') {
			skip_quoted(lx);
			tk.type = (c == '"') ? CDT_STRING : CDT_CHAR;
		} else {
			adv(lx);
			tk.type = CDT_PUNCT;
		}

		lx->at_line_start = false;
		tk.len = lx->pos - tk.pos;
		return tk;
	}
	cdtoken eof = {.type = CDT_EOF, .pos = lx->len, .len = 0, .line = lx->line};
	return eof;
}


vec *cdlex_all(const char *src, size_t len)
{
	vec *ret = vec_new(sizeof(cdtoken));
	cdlexer *lx = cdlexer_new(src, len);
	for (cdtoken tk = cdlexer_next(lx); tk.type != CDT_EOF; tk = cdlexer_next(lx)) {
		vec_push(ret, &tk);
	}
	cdlexer_free(lx);
	return ret;
}


const char *cdtoken_type_name(cdtoken_type type)
{
	switch (type) {
	case CDT_EOF:
		return "EOF";
	case CDT_IDENT:
		return "IDENT";
	case CDT_NUMBER:
		return "NUMBER";
	case CDT_STRING:
		return "STRING";
	case CDT_CHAR:
		return "CHAR";
	case CDT_PUNCT:
		return "PUNCT";
	case CDT_PP:
		return "PP";
	case CDT_DOC:
		return "DOC";
	case CDT_DOC_POST:
		return "DOC_POST";
	}
	return "?";
}
