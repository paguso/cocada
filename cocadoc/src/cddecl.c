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
#include <stdint.h>
#include <string.h>

#include "cddecl.h"
#include "cdlexer.h"
#include "cstrutil.h"
#include "strbuf.h"


#define NONE SIZE_MAX


// Matching context: the source and its tokens
typedef struct {
	const char *src;
	const vec *toks;
	size_t n;
} ctx;


static inline const cdtoken *T(const ctx *c, size_t i)
{
	return (const cdtoken *)vec_get(c->toks, i);
}


static inline bool is_punct(const ctx *c, size_t i, char p)
{
	const cdtoken *t = T(c, i);
	return t->type == CDT_PUNCT && c->src[t->pos] == p;
}


static inline bool is_word(const ctx *c, size_t i, const char *w)
{
	const cdtoken *t = T(c, i);
	return t->type == CDT_IDENT && t->len == strlen(w)
	       && strncmp(c->src + t->pos, w, t->len) == 0;
}


static inline bool is_open(const ctx *c, size_t i)
{
	return is_punct(c, i, '(') || is_punct(c, i, '[') || is_punct(c, i, '{');
}


static inline bool is_close(const ctx *c, size_t i)
{
	return is_punct(c, i, ')') || is_punct(c, i, ']') || is_punct(c, i, '}');
}


static inline bool is_doc(const ctx *c, size_t i)
{
	return T(c, i)->type == CDT_DOC || T(c, i)->type == CDT_DOC_POST;
}


static inline char *tok_str(const ctx *c, size_t i)
{
	return cstr_clone_len(c->src + T(c, i)->pos, T(c, i)->len);
}


// Index of the bracket closing the one at i, or the last token if unbalanced.
static size_t match_close(const ctx *c, size_t i)
{
	int depth = 0;
	for (size_t k = i; k < c->n; k++) {
		if (is_open(c, k)) {
			depth++;
		} else if (is_close(c, k)) {
			if (--depth == 0) {
				return k;
			}
		}
	}
	return c->n - 1;
}


// Next top-level index after i in [i, to], skipping bracketed groups.
static inline size_t next_top(const ctx *c, size_t i)
{
	return is_open(c, i) ? match_close(c, i) + 1 : i + 1;
}


/*
 * Signature text
 */

// Whether token i can end an operand (so that a following +/- is binary)
static bool ends_operand(const ctx *c, size_t i)
{
	cdtoken_type ty = T(c, i)->type;
	return ty == CDT_IDENT || ty == CDT_NUMBER || ty == CDT_STRING
	       || ty == CDT_CHAR || is_punct(c, i, ')') || is_punct(c, i, ']');
}


// Whether chars a, b form a two-char operator like `->`, `==`, `<<`
static bool is_compound_op(char a, char b)
{
	if (!a || !b) {
		return false;
	}
	if (b == '=') {
		return strchr("+-*/%&|^<>!=", a) != NULL;
	}
	return (a == '-' && b == '>') || (a == b && strchr("+-&|<>", a));
}


static bool need_space(const ctx *c, size_t prev, size_t cur)
{
	const cdtoken *p = T(c, prev), *t = T(c, cur);
	char pc = (p->type == CDT_PUNCT) ? c->src[p->pos] : 0;
	char tc = (t->type == CDT_PUNCT) ? c->src[t->pos] : 0;

	if (tc && strchr(",;)]", tc)) {
		return false;
	}
	if (pc && strchr("([", pc)) {
		return false;
	}
	if (tc == '[') {
		return false;
	}
	if (tc == '(') {
		// `void (*f)` vs `f(x)`
		return cur + 1 < c->n && is_punct(c, cur + 1, '*') && pc != ')';
	}
	if (pc == '*') {
		return false;
	}
	if (tc == '*') {
		return pc != '*' && pc != '(';
	}
	if (pc == '.' && tc == '.') {
		return false;
	}
	if (is_compound_op(pc, tc)) {
		return false;
	}
	if ((pc == '-' || pc == '+' || pc == '~' || pc == '!')
	        && (prev == 0 || !ends_operand(c, prev - 1))) {
		return false; // unary operator
	}
	return true;
}


// Joins tokens [from, to] (inclusive), eliding the body [bopen, bclose] if given.
static char *join(const ctx *c, size_t from, size_t to, size_t bopen,
                  size_t bclose)
{
	strbuf *sb = strbuf_new();
	size_t prev = NONE;
	for (size_t i = from; i <= to && i < c->n; i++) {
		cdtoken_type ty = T(c, i)->type;
		if (ty == CDT_PP || ty == CDT_DOC || ty == CDT_DOC_POST) {
			continue;
		}
		if (prev != NONE && need_space(c, prev, i)) {
			strbuf_append_char(sb, ' ');
		}
		if (i == bopen) {
			strbuf_append(sb, "{...}");
			i = bclose;
		} else {
			strbuf_nappend(sb, c->src + T(c, i)->pos, T(c, i)->len);
		}
		prev = i;
	}
	return strbuf_detach(sb);
}


// Copies a preprocessor directive, collapsing whitespace, line continuations
// and comments into single spaces. Stops after n chars if n != NONE.
static char *pp_normalise(const char *s, size_t len, size_t n)
{
	strbuf *sb = strbuf_new();
	bool space = false;
	for (size_t i = 0; i < len && (n == NONE || i < n); i++) {
		if (s[i] == '/' && i + 1 < len && s[i + 1] == '*') {
			for (i += 2; i + 1 < len && !(s[i] == '*' && s[i + 1] == '/'); i++);
			i++;
			space = true;
		} else if (s[i] == '/' && i + 1 < len && s[i + 1] == '/') {
			while (i + 1 < len && s[i + 1] != '\n') {
				i++;
			}
			space = true;
		} else if (s[i] == '\\' || isspace((unsigned char)s[i])) {
			space = true;
		} else {
			if (space && strbuf_len(sb) > 0) {
				strbuf_append_char(sb, ' ');
			}
			space = false;
			strbuf_append_char(sb, s[i]);
		}
	}
	return strbuf_detach(sb);
}


/*
 * Names
 */

// Name of a declarator in [from, to]: the function-pointer name in
// `(*name)`, otherwise the last top-level identifier before any `=` or `:`.
static char *declarator_name(const ctx *c, size_t from, size_t to)
{
	size_t last = NONE;
	for (size_t i = from; i <= to; i = next_top(c, i)) {
		if (is_punct(c, i, '(') && i + 1 <= to && is_punct(c, i + 1, '*')) {
			size_t k = i + 1;
			while (k <= to && is_punct(c, k, '*')) {
				k++;
			}
			if (k <= to && T(c, k)->type == CDT_IDENT) {
				return tok_str(c, k);
			}
		}
		if (is_punct(c, i, '=') || is_punct(c, i, ':')) {
			break;
		}
		if (T(c, i)->type == CDT_IDENT) {
			last = i;
		}
	}
	return (last != NONE) ? tok_str(c, last) : cstr_new(0);
}


static cddecl new_decl(cddecl_kind kind, size_t line, char *doc)
{
	cddecl d = {.kind = kind, .name = NULL, .sig = NULL, .doc = doc,
	            .line = line, .members = NULL
	           };
	return d;
}


/*
 * Aggregates
 */

static void match_members(const ctx *c, size_t bopen, size_t bclose,
                          bool is_enum, cddecl *agg)
{
	agg->members = vec_new(sizeof(cddecl));
	char sep = is_enum ? ',' : ';';
	char *pending = NULL;
	size_t k = bopen + 1;
	while (k < bclose) {
		const cdtoken *t = T(c, k);
		if (t->type == CDT_DOC) {
			FREE(pending);
			pending = tok_str(c, k);
			k++;
			continue;
		}
		if (t->type == CDT_DOC_POST) {
			// documents the previous member
			size_t nm = vec_len(agg->members);
			cddecl *last = nm ? vec_get_mut(agg->members, nm - 1) : NULL;
			if (last && !last->doc) {
				last->doc = tok_str(c, k);
			}
			k++;
			continue;
		}
		if (t->type == CDT_PP || is_punct(c, k, sep)) {
			k++;
			continue;
		}

		size_t start = k, end = k;
		char *post = NULL;
		while (k < bclose && !is_punct(c, k, sep)) {
			if (T(c, k)->type == CDT_DOC_POST && !post) {
				post = tok_str(c, k);
			} else if (!is_doc(c, k)) {
				end = is_open(c, k) ? match_close(c, k) : k;
			}
			k = next_top(c, k);
		}

		cddecl m = new_decl(CDD_MEMBER, T(c, start)->line, pending);
		pending = NULL;
		if (post) {
			if (m.doc) {
				FREE(post);
			} else {
				m.doc = post;
			}
		}
		m.name = is_enum ? tok_str(c, start) : declarator_name(c, start, end);
		m.sig = join(c, start, end, NONE, NONE);
		vec_push(agg->members, &m);
	}
	FREE(pending);
}


/*
 * Declarations
 */

// Matches the declaration starting at j documented by doc (moved).
// Pushes the result to out. Returns the index after the declaration.
static size_t match_decl(const ctx *c, size_t j, char *doc, vec *out)
{
	const cdtoken *first = T(c, j);
	cddecl d = new_decl(CDD_UNKNOWN, first->line, doc);

	// #define
	if (first->type == CDT_PP) {
		const char *s = c->src + first->pos;
		size_t len = first->len, i = 1;
		while (i < len && isblank((unsigned char)s[i])) i++;
		if (strncmp(s + i, "if", 2) == 0 && j + 1 < c->n) {
			// guard, e.g. #ifndef X / #define X: document what follows
			return match_decl(c, j + 1, doc, out);
		}
		if (strncmp(s + i, "define", 6) != 0) {
			FREE(doc); // documents some other directive: ignore
			return j;
		}
		i += 6;
		while (i < len && isblank((unsigned char)s[i])) i++;
		size_t nm = i;
		while (i < len && (isalnum((unsigned char)s[i]) || s[i] == '_')) i++;
		d.kind = CDD_MACRO;
		d.name = cstr_clone_len(s + nm, i - nm);
		if (i < len && s[i] == '(') {
			while (i < len && s[i] != ')') i++;
			d.sig = pp_normalise(s, len, i + 1);
		} else {
			d.sig = pp_normalise(s, len, NONE);
		}
		vec_push(out, &d);
		return j + 1;
	}

	// NAME( ... ) at file scope: macro invocation
	if (first->type == CDT_IDENT && j + 1 < c->n && is_punct(c, j + 1, '(')) {
		size_t close = match_close(c, j + 1);
		d.kind = CDD_MACROCALL;
		d.name = tok_str(c, j);
		d.sig = join(c, j, close, NONE, NONE);
		vec_push(out, &d);
		return (close + 1 < c->n && is_punct(c, close + 1, ';')) ? close + 2 : close + 1;
	}

	// Ordinary declaration: scan top-level tokens up to ';' or a function body
	size_t k = j, end = NONE, next = NONE;
	size_t paren = NONE, bopen = NONE, bclose = NONE, agg_kw = NONE, eq = NONE;
	bool has_typedef = false;
	while (k < c->n) {
		if (is_punct(c, k, ';')) {
			end = k - 1;
			next = k + 1;
			break;
		}
		if (is_punct(c, k, '{')) {
			if (k > j && is_punct(c, k - 1, ')') && eq == NONE) {
				// function definition: skip the body
				end = k - 1;
				next = match_close(c, k) + 1;
				break;
			}
			if (bopen == NONE) {
				bopen = k;
				bclose = match_close(c, k);
			}
		} else if (is_punct(c, k, '(') && paren == NONE && bopen == NONE) {
			paren = k;
		} else if (is_punct(c, k, '=') && eq == NONE) {
			eq = k;
		} else if (is_word(c, k, "typedef")) {
			has_typedef = true;
		} else if (agg_kw == NONE && bopen == NONE && (is_word(c, k, "struct")
		           || is_word(c, k, "union") || is_word(c, k, "enum"))) {
			agg_kw = k;
		} else if (T(c, k)->type == CDT_DOC) {
			// undelimited declaration ran into the next documented one
			end = k - 1;
			next = k;
			break;
		}
		k = next_top(c, k);
	}
	if (end == NONE) {
		end = c->n - 1;
		next = c->n;
	}

	bool has_body = bopen != NONE && bopen <= end;
	bool fptr = paren != NONE && paren + 1 <= end && is_punct(c, paren + 1, '*');
	if (has_typedef) {
		d.kind = CDD_TYPEDEF;
		d.name = declarator_name(c, has_body ? bclose + 1 : j, end);
	} else if (agg_kw != NONE && ((has_body && bclose == end)
	                              || (!has_body && paren == NONE && end == agg_kw + 1))) {
		d.kind = is_word(c, agg_kw, "struct") ? CDD_STRUCT
		         : is_word(c, agg_kw, "union") ? CDD_UNION : CDD_ENUM;
		d.name = (agg_kw + 1 <= end && T(c, agg_kw + 1)->type == CDT_IDENT)
		         ? tok_str(c, agg_kw + 1) : cstr_new(0);
	} else if (paren != NONE && !fptr && (eq == NONE || paren < eq)
	           && paren > j && T(c, paren - 1)->type == CDT_IDENT) {
		d.kind = CDD_FUNC;
		d.name = tok_str(c, paren - 1);
	} else {
		d.kind = CDD_VAR;
		d.name = declarator_name(c, has_body ? bclose + 1 : j, end);
	}
	d.sig = join(c, j, end, has_body ? bopen : NONE, bclose);
	if (has_body && agg_kw != NONE && agg_kw < bopen) {
		match_members(c, bopen, bclose, is_word(c, agg_kw, "enum"), &d);
	}
	vec_push(out, &d);
	return next;
}


// Returns the name following @file in the doc comment, or NULL.
static char *file_tag(const char *s, size_t len)
{
	for (size_t i = 0; i + 5 <= len; i++) {
		if ((s[i] == '@' || s[i] == '\\') && strncmp(s + i + 1, "file", 4) == 0
		        && (i + 5 == len || !isalnum((unsigned char)s[i + 5]))) {
			size_t k = i + 5;
			while (k < len && isblank((unsigned char)s[k])) k++;
			size_t st = k;
			while (k < len && !isspace((unsigned char)s[k])
			        && !(s[k] == '*' && k + 1 < len && s[k + 1] == '/')) k++;
			return cstr_clone_len(s + st, k - st);
		}
	}
	return NULL;
}


vec *cddecl_match(const char *src, const vec *toks)
{
	ctx c = {.src = src, .toks = toks, .n = vec_len(toks)};
	vec *out = vec_new(sizeof(cddecl));
	int depth = 0;
	size_t i = 0;
	while (i < c.n) {
		const cdtoken *t = T(&c, i);
		if (t->type == CDT_DOC && depth == 0) {
			char *fname = file_tag(src + t->pos, t->len);
			if (fname) {
				cddecl d = new_decl(CDD_FILE, t->line, tok_str(&c, i));
				d.name = fname;
				d.sig = cstr_new(0);
				vec_push(out, &d);
				i++;
			} else if (i + 1 < c.n && T(&c, i + 1)->type != CDT_DOC
			           && !is_close(&c, i + 1)) {
				i = match_decl(&c, i + 1, tok_str(&c, i), out);
			} else {
				i++; // orphan doc comment
			}
			continue;
		}
		if (is_punct(&c, i, '{')) {
			depth++;
		} else if (is_punct(&c, i, '}') && depth > 0) {
			depth--;
		}
		i++;
	}
	return out;
}


void cddecl_finalise(void *ptr, const finaliser *fnr)
{
	cddecl *d = (cddecl *)ptr;
	FREE(d->name);
	FREE(d->sig);
	FREE(d->doc);
	if (d->members) {
		cddecl_vec_free(d->members);
	}
}


void cddecl_vec_free(vec *decls)
{
	DESTROY(decls, finaliser_cons(FNR(vec), FNR(cddecl)));
}


const char *cddecl_kind_name(cddecl_kind kind)
{
	switch (kind) {
	case CDD_FILE:
		return "FILE";
	case CDD_FUNC:
		return "FUNC";
	case CDD_MACRO:
		return "MACRO";
	case CDD_MACROCALL:
		return "MACROCALL";
	case CDD_TYPEDEF:
		return "TYPEDEF";
	case CDD_STRUCT:
		return "STRUCT";
	case CDD_UNION:
		return "UNION";
	case CDD_ENUM:
		return "ENUM";
	case CDD_VAR:
		return "VAR";
	case CDD_MEMBER:
		return "MEMBER";
	case CDD_UNKNOWN:
		return "UNKNOWN";
	}
	return "?";
}
