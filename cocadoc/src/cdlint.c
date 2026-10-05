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
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cddecl.h"
#include "cddoc.h"
#include "cdlexer.h"
#include "cdlint.h"
#include "cdsym.h"
#include "cstrutil.h"
#include "new.h"


typedef struct {
	cdwarn w;
	size_t seq; // insertion order, to sort stably
} seqwarn;


static void warn(vec *out, size_t line, const char *name, const char *rule,
                 const char *fmt, ...)
{
	char buf[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	seqwarn sw = {
		.w = {.line = line, .name = cstr_clone(name), .rule = rule, .msg = cstr_clone(buf)},
		.seq = vec_len(out)
	};
	vec_push(out, &sw);
}


static const char *kind_word(cddecl_kind k)
{
	switch (k) {
	case CDD_FUNC:
		return "function";
	case CDD_MACRO:
		return "macro";
	case CDD_TYPEDEF:
		return "type";
	case CDD_STRUCT:
		return "struct";
	case CDD_UNION:
		return "union";
	case CDD_ENUM:
		return "enum";
	case CDD_VAR:
		return "variable";
	case CDD_MEMBER:
		return "member";
	case CDD_MACROCALL:
		return "macro invocation";
	default:
		return "declaration";
	}
}


static inline bool is_ident_char(char c)
{
	return isalnum((unsigned char)c) || c == '_';
}


// Generator macros (named *_DECL or *_IMPL by convention) declare or
// define typed function families. Their docs describe the generated
// functions, so their parameters are not checked (DC14, not handled yet).
static bool is_generator(const cddecl *d)
{
	size_t n = strlen(d->name);
	return d->kind == CDD_MACRO && n > 5
	       && (strcmp(d->name + n - 5, "_DECL") == 0 || strcmp(d->name + n - 5, "_IMPL") == 0);
}


/*
 * Parameters from the signature
 */

// Position of "name(" in the signature, or -1
static long name_paren(const char *sig, const char *name)
{
	size_t n = strlen(name);
	for (const char *p = strstr(sig, name); p; p = strstr(p + 1, name)) {
		if ((p == sig || !is_ident_char(p[-1])) && p[n] == '(') {
			return p - sig;
		}
	}
	return -1;
}


static bool is_type_word(const char *w, size_t n)
{
	static const char *WORDS[] = {
		"const", "volatile", "restrict", "struct", "union", "enum", "signed",
		"unsigned", "register", "void", "char", "short", "int", "long",
		"float", "double", "_Bool", "bool"
	};
	for (size_t i = 0; i < sizeof(WORDS) / sizeof(WORDS[0]); i++) {
		if (strlen(WORDS[i]) == n && strncmp(WORDS[i], w, n) == 0) {
			return true;
		}
	}
	return false;
}


// Name declared by a function parameter, or NULL if unnamed
static char *param_name(const char *p, size_t len)
{
	// function pointer: ... (*name)(...)
	for (size_t i = 0; i + 1 < len; i++) {
		if (p[i] == '(' && p[i + 1] == '*') {
			size_t k = i + 1;
			while (k < len && (p[k] == '*' || p[k] == ' ')) k++;
			size_t b = k;
			while (k < len && is_ident_char(p[k])) k++;
			return (k > b) ? cstr_clone_len(p + b, k - b) : NULL;
		}
	}
	// otherwise the last identifier outside brackets, if there is more
	// than one (a lone identifier is a type, as in "size_t")
	size_t nids = 0, last = 0, last_len = 0;
	int depth = 0;
	for (size_t i = 0; i < len;) {
		if (p[i] == '[' || p[i] == '(') {
			depth++;
			i++;
		} else if (p[i] == ']' || p[i] == ')') {
			depth--;
			i++;
		} else if (depth == 0 && (isalpha((unsigned char)p[i]) || p[i] == '_')) {
			size_t b = i;
			while (i < len && is_ident_char(p[i])) i++;
			nids++;
			last = b;
			last_len = i - b;
		} else {
			i++;
		}
	}
	if (nids < 2 || is_type_word(p + last, last_len)) {
		return NULL;
	}
	return cstr_clone_len(p + last, last_len);
}


// Parameter names of a function or function-like macro (vec of heap
// char *), or NULL if the declaration has no parameter list. *named is
// set to false if some parameter has no name.
static vec *sig_params(const cddecl *d, bool *named)
{
	*named = true;
	const char *sig = d->sig;
	long open;
	bool macro = d->kind == CDD_MACRO;
	if (d->kind == CDD_FUNC) {
		open = name_paren(sig, d->name);
		if (open < 0) {
			return NULL;
		}
		open += strlen(d->name);
	} else if (d->kind == CDD_TYPEDEF) {
		// function pointer type: typedef T (*name)(params)
		char pat[256];
		snprintf(pat, sizeof(pat), "(*%s)(", d->name);
		const char *p = strstr(sig, pat);
		if (!p) {
			return NULL;
		}
		open = (p - sig) + strlen(pat) - 1;
	} else if (macro) {
		size_t k = strlen("#define ") + strlen(d->name);
		if (strncmp(sig, "#define ", 8) != 0 || strlen(sig) <= k || sig[k] != '(') {
			return NULL; // object-like macro
		}
		open = k;
	} else {
		return NULL;
	}
	vec *ret = vec_new(sizeof(char *));
	int depth = 0;
	size_t b = open + 1;
	for (size_t i = open; sig[i]; i++) {
		char c = sig[i];
		if (c == '(' || c == '[') {
			depth++;
		} else if (c == ')' || c == ']') {
			depth--;
		}
		if ((c == ',' && depth == 1) || (c == ')' && depth == 0)) {
			size_t s = b, e = i;
			while (s < e && isspace((unsigned char)sig[s])) s++;
			while (e > s && isspace((unsigned char)sig[e - 1])) e--;
			char *name = NULL;
			if (e - s == 3 && strncmp(sig + s, "...", 3) == 0) {
				name = cstr_clone("...");
			} else if (e > s && macro) {
				name = cstr_clone_len(sig + s, e - s);
			} else if (e > s && !(e - s == 4 && strncmp(sig + s, "void", 4) == 0)) {
				name = param_name(sig + s, e - s);
				if (!name) {
					*named = false;
				}
			}
			if (name) {
				vec_push_rawptr(ret, name);
			}
			b = i + 1;
			if (c == ')') {
				break;
			}
		}
	}
	return ret;
}


// Whether a function returns a value
static bool returns_value(const cddecl *d)
{
	long pos = name_paren(d->sig, d->name);
	if (pos < 0) {
		return false;
	}
	// return type: the words before the name, minus storage specifiers
	char words[256] = "";
	const char *s = d->sig;
	for (long i = 0; i < pos;) {
		if (isalpha((unsigned char)s[i]) || s[i] == '_') {
			long b = i;
			while (i < pos && is_ident_char(s[i])) i++;
			size_t n = i - b;
			if ((n == 6 && strncmp(s + b, "static", 6) == 0)
			        || (n == 6 && strncmp(s + b, "inline", 6) == 0)
			        || (n == 6 && strncmp(s + b, "extern", 6) == 0)) {
				continue;
			}
			strncat(words, s + b, sizeof(words) - strlen(words) - 1 < n ? 0 : n);
		} else {
			if (!isspace((unsigned char)s[i]) && strlen(words) < sizeof(words) - 1) {
				strncat(words, s + i, 1); // e.g. '*'
			}
			i++;
		}
	}
	return strcmp(words, "void") != 0;
}


static bool str_in(const vec *v, const char *s, size_t *pos)
{
	for (size_t i = 0, n = vec_len(v); i < n; i++) {
		if (strcmp(vec_get_rawptr(v, i), s) == 0) {
			if (pos) {
				*pos = i;
			}
			return true;
		}
	}
	return false;
}


// DC8: @param and @return against the declaration
static void check_signature(vec *out, const cddecl *d, const cddoc *doc)
{
	bool named;
	// (function pointer types have parameters too, but documenting them
	// is not required)
	vec *sp = ((d->kind == CDD_FUNC || d->kind == CDD_MACRO) && !is_generator(d))
	          ? sig_params(d, &named) : NULL;
	if (sp && named) {
		vec *documented = vec_new(sizeof(char *));
		for (size_t i = 0, n = vec_len(doc->params); i < n; i++) {
			const cdparam *p = vec_get(doc->params, i);
			if (p->name[0] == '\0') {
				continue;
			}
			vec_push_rawptr(documented, (void *)p->name);
			if (!str_in(sp, p->name, NULL)) {
				warn(out, d->doc_line + p->line, d->name, "DC8",
				     "@param %s is not a parameter of %s", p->name, d->name);
			}
		}
		for (size_t i = 0, n = vec_len(sp); i < n; i++) {
			const char *name = vec_get_rawptr(sp, i);
			if (!str_in(documented, name, NULL)) {
				warn(out, d->doc_line, d->name, "DC8", "parameter %s is not documented", name);
			}
		}
		// order
		size_t prev = 0;
		bool first = true;
		for (size_t i = 0, n = vec_len(documented); i < n; i++) {
			size_t pos;
			if (str_in(sp, vec_get_rawptr(documented, i), &pos)) {
				if (!first && pos < prev) {
					warn(out, d->doc_line, d->name, "DC8",
					     "@params are not in the order of the declaration");
					break;
				}
				prev = pos;
				first = false;
			}
		}
		DESTROY_FLAT(documented, vec);
	}
	if (sp) {
		DESTROY(sp, finaliser_cons(FNR(vec), finaliser_new_ptr()));
	}

	if (d->kind == CDD_FUNC) {
		bool rv = returns_value(d);
		if (rv && !doc->ret) {
			warn(out, d->doc_line, d->name, "DC8", "the return value is not documented");
		} else if (!rv && doc->ret) {
			warn(out, d->doc_line, d->name, "DC8", "@return for a function returning void");
		}
	}
}


static void add_doc_diags(vec *out, const cddoc *doc, size_t doc_line,
                          const char *name)
{
	for (size_t i = 0, n = vec_len(doc->diags); i < n; i++) {
		const cddiag *dg = vec_get(doc->diags, i);
		warn(out, doc_line + dg->line, name, dg->rule, "%s", dg->msg);
	}
}


// DC10, DC11: references
// d is the documented declaration (NULL for the file comment), whose
// comment starts at doc_line.
static void check_refs(vec *out, const cdfile *f, const cdsymtab *tab,
                       const cddecl *d, size_t doc_line, const cddoc *doc,
                       const char *name)
{
	bool named = false;
	vec *params = (d && d->kind != CDD_MEMBER) ? sig_params(d, &named) : NULL;
	for (size_t i = 0, n = vec_len(doc->refs); i < n; i++) {
		const cdref *r = vec_get(doc->refs, i);
		size_t line = doc_line + r->line;
		if (r->kind == CDR_PARAM) {
			if (d && is_generator(d)) {
				continue;
			}
			if (!params) {
				bool sym = tab && cdsymtab_resolve(tab, r->target, f, NULL);
				warn(out, line, name, "DC11", "@p %s: %s has no parameters%s%s%s", r->target, name,
				     sym ? " (did you mean #" : "", sym ? r->target : "", sym ? "?)" : "");
			} else if (named && !str_in(params, r->target, NULL)) {
				bool sym = tab && cdsymtab_resolve(tab, r->target, f, NULL);
				warn(out, line, name, "DC11", "@p %s is not a parameter of %s%s%s%s", r->target, name,
				     sym ? " (did you mean #" : "", sym ? r->target : "", sym ? "?)" : "");
			}
			continue;
		}
		if (!tab) {
			continue;
		}
		size_t ncands;
		const cdsym *s = cdsymtab_resolve(tab, r->target, f, &ncands);
		const char *what = (r->kind == CDR_SEE) ? "@see" : "reference";
		const char *rule = (r->kind == CDR_SEE) ? "DC10" : "DC11";
		const char *hash = (r->kind == CDR_SEE) ? "" : "#";
		if (!s) {
			warn(out, line, name, rule, "unknown %s %s%s", what, hash, r->target);
		} else if (ncands > 1 && s->file != f) {
			warn(out, line, name, rule, "ambiguous %s %s%s (declared %zu times)",
			     what, hash, r->target, ncands);
		}
	}
	if (params) {
		DESTROY(params, finaliser_cons(FNR(vec), finaliser_new_ptr()));
	}
}


static int cmp_warn(const void *a, const void *b)
{
	const seqwarn *x = a, *y = b;
	if (x->w.line != y->w.line) {
		return (x->w.line < y->w.line) ? -1 : 1;
	}
	return (x->seq < y->seq) ? -1 : (x->seq > y->seq);
}


vec *cdlint(const cdfile *file, const cdsymtab *tab)
{
	vec *out = vec_new(sizeof(seqwarn));
	const vec *decls = file->decls;
	const char *base = file->name;

	bool has_file = false;
	for (size_t i = 0, n = vec_len(decls); i < n; i++) {
		const cddecl *d = vec_get(decls, i);
		if (d->kind == CDD_FILE) {
			if (has_file) {
				warn(out, d->line, base, "DC3", "more than one file comment");
			}
			has_file = true;
			if (strcmp(d->name, base) != 0) {
				warn(out, d->line, base, "DC3", "@file %s does not match the file name %s",
				     d->name, base);
			}
			cddoc *doc = cddoc_parse(d->doc, strlen(d->doc));
			if (vec_len(doc->authors) == 0) {
				warn(out, d->line, base, "DC3", "no @author in the file comment");
			}
			add_doc_diags(out, doc, d->doc_line, base);
			check_refs(out, file, tab, NULL, d->doc_line, doc, base);
			cddoc_free(doc);
			continue;
		}

		// names starting with '_' are private by convention
		if (!d->doc && d->name[0] != '_') {
			warn(out, d->line, d->name, "DC4", "undocumented %s", kind_word(d->kind));
		} else if (d->doc) {
			cddoc *doc = cddoc_parse(d->doc, strlen(d->doc));
			add_doc_diags(out, doc, d->doc_line, d->name);
			check_signature(out, d, doc);
			check_refs(out, file, tab, d, d->doc_line, doc, d->name);
			cddoc_free(doc);
		}

		for (size_t j = 0, m = d->members ? vec_len(d->members) : 0; j < m; j++) {
			const cddecl *mb = vec_get(d->members, j);
			char name[256];
			snprintf(name, sizeof(name), "%s.%s", d->name, mb->name);
			if (!mb->doc) {
				warn(out, mb->line, name, "DC4", "undocumented member");
				continue;
			}
			if (strncmp(mb->doc, "/**<", 4) != 0) {
				warn(out, mb->doc_line, name, "DC2",
				     "document members with a trailing /**< comment");
			}
			cddoc *doc = cddoc_parse(mb->doc, strlen(mb->doc));
			add_doc_diags(out, doc, mb->doc_line, name);
			check_refs(out, file, tab, mb, mb->doc_line, doc, name);
			cddoc_free(doc);
		}
	}
	if (!has_file) {
		warn(out, 1, base, "DC3", "no file comment (@file, @author, @brief)");
	}

	vec_qsort(out, cmp_warn);
	vec *ret = vec_new(sizeof(cdwarn));
	for (size_t i = 0, n = vec_len(out); i < n; i++) {
		const seqwarn *sw = vec_get(out, i);
		vec_push(ret, &sw->w);
	}
	DESTROY_FLAT(out, vec);
	return ret;
}


static void cdwarn_finalise(void *ptr, const finaliser *fnr)
{
	cdwarn *w = (cdwarn *)ptr;
	FREE(w->name);
	FREE(w->msg);
}


void cdwarn_vec_free(vec *warns)
{
	DESTROY(warns, finaliser_cons(FNR(vec), finaliser_new(cdwarn_finalise)));
}
