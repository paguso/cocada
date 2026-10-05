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

/**
 * @file cdmacro.c
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cddecl.h"
#include "cdlexer.h"
#include "cdmacro.h"
#include "cstrutil.h"
#include "hash.h"
#include "hashmap.h"
#include "new.h"
#include "strbuf.h"


#define MAX_DEPTH 64
#define MAX_TOKENS 1000000


/*
 * Tokens
 */

typedef struct {
	char *s;
	bool ident;
} tok;


static vec *toks_new()
{
	return vec_new(sizeof(tok));
}


static void toks_push(vec *v, const char *s, size_t len, bool ident)
{
	tok t = {.s = cstr_clone_len(s, len), .ident = ident};
	vec_push(v, &t);
}


static void toks_free(vec *v)
{
	if (!v) {
		return;
	}
	for (size_t i = 0, n = vec_len(v); i < n; i++) {
		FREE(((tok *)vec_get_mut(v, i))->s);
	}
	DESTROY_FLAT(v, vec);
}


static inline const tok *T(const vec *v, size_t i)
{
	return (const tok *)vec_get(v, i);
}


static inline bool is_p(const vec *v, size_t i, const char *s)
{
	return i < vec_len(v) && !T(v, i)->ident && strcmp(T(v, i)->s, s) == 0;
}


// Tokens of a C text, without comments except trailing member docs
// (/**< ... */), which are kept: in a generated struct or enum, the macro
// body is the only place for them
static vec *lex(const char *text)
{
	vec *ret = toks_new();
	vec *cts = cdlex_all(text, strlen(text));
	for (size_t i = 0, n = vec_len(cts); i < n; i++) {
		const cdtoken *ct = vec_get(cts, i);
		if (ct->type == CDT_DOC || ct->type == CDT_PP) {
			continue;
		}
		toks_push(ret, text + ct->pos, ct->len, ct->type == CDT_IDENT);
	}
	DESTROY_FLAT(cts, vec);
	return ret;
}


static char *render(const vec *v)
{
	strbuf *sb = strbuf_new();
	for (size_t i = 0, n = vec_len(v); i < n; i++) {
		if (i) {
			strbuf_append_char(sb, ' ');
		}
		strbuf_append(sb, T(v, i)->s);
	}
	return strbuf_detach(sb);
}


/*
 * Macros
 */

typedef struct {
	const cddecl *decl;
	const cdfile *file;
	vec *params;         // vec of char *
	bool variadic;
	vec *body;           // tokens
	// as a generator (computed on demand)
	bool analysed;
	cdgen_kind kind;
	vec *patterns;       // vec of cddecl
} macro;


struct _cdmacrotab {
	vec *macros;         // vec of macro
	hashmap *index;      // name (char *) -> position in macros (size_t)
	hashmap *invoked;    // names of macros invoked at file scope (char *) -> 0
	hashmap *used;       // identifiers in macro bodies and invocation arguments -> 0
	vec *names;          // the keys of index and invoked (vec of heap char *)
};


static uint64_t hash_str(const void *ptr)
{
	const char *s = *((const char **)ptr);
	return fnv1a_64bit_hash(s, strlen(s));
}


static bool eq_str(const void *a, const void *b)
{
	return strcmp(*((const char **)a), *((const char **)b)) == 0;
}


// Parses "#define NAME(P1, P2, ...) body". Returns false if not function-like.
static bool parse_def(const char *def, macro *m)
{
	const char *s = def;
	while (*s == '#' || isblank((unsigned char)*s)) s++;
	if (strncmp(s, "define", 6) != 0) {
		return false;
	}
	s += 6;
	while (isblank((unsigned char)*s)) s++;
	while (isalnum((unsigned char)*s) || *s == '_') s++;
	if (*s != '(') {
		return false; // object-like
	}
	s++;
	m->params = vec_new(sizeof(char *));
	m->variadic = false;
	while (*s && *s != ')') {
		while (isspace((unsigned char)*s) || *s == '\\') s++;
		const char *b = s;
		while (*s && *s != ',' && *s != ')') s++;
		const char *e = s;
		while (e > b && (isspace((unsigned char)e[-1]) || e[-1] == '\\')) e--;
		if (e - b == 3 && strncmp(b, "...", 3) == 0) {
			m->variadic = true;
		} else if (e > b) {
			vec_push_rawptr(m->params, cstr_clone_len(b, e - b));
		}
		if (*s == ',') s++;
	}
	if (*s == ')') s++;
	// body, without line continuations
	strbuf *body = strbuf_new();
	for (; *s; s++) {
		if (*s == '\\' && (s[1] == '\n' || (s[1] == '\r' && s[2] == '\n'))) {
			s += (s[1] == '\r') ? 2 : 1;
			strbuf_append_char(body, ' ');
		} else {
			strbuf_append_char(body, *s);
		}
	}
	m->body = lex(strbuf_as_str(body));
	strbuf_free(body);
	return true;
}


static const macro *find_macro(const cdmacrotab *t, const char *name)
{
	if (!hashmap_contains(t->index, &name)) {
		return NULL;
	}
	return vec_get(t->macros, hashmap_get_size_t(t->index, &name));
}


static void mark_used(cdmacrotab *t, const vec *v, size_t from)
{
	for (size_t i = from, n = vec_len(v); i < n; i++) {
		const tok *k = vec_get(v, i);
		if (k->ident && !hashmap_contains(t->used, &k->s)) {
			char *name = cstr_clone(k->s);
			vec_push_rawptr(t->names, name);
			hashmap_ins_size_t(t->used, &name, 0);
		}
	}
}


cdmacrotab *cdmacrotab_new(const vec *files)
{
	cdmacrotab *t = NEW(cdmacrotab);
	t->macros = vec_new(sizeof(macro));
	t->index = hashmap_new(sizeof(char *), sizeof(size_t), hash_str, eq_str);
	t->invoked = hashmap_new(sizeof(char *), sizeof(size_t), hash_str, eq_str);
	t->used = hashmap_new(sizeof(char *), sizeof(size_t), hash_str, eq_str);
	t->names = vec_new(sizeof(char *));
	for (size_t i = 0, n = vec_len(files); i < n; i++) {
		const cdfile *f = vec_get_rawptr(files, i);
		for (size_t j = 0, m = vec_len(f->decls); j < m; j++) {
			const cddecl *d = vec_get(f->decls, j);
			if (d->kind == CDD_MACROCALL && !hashmap_contains(t->invoked, &d->name)) {
				char *name = cstr_clone(d->name);
				vec_push_rawptr(t->names, name);
				hashmap_ins_size_t(t->invoked, &name, 0);
			}
			if (d->kind != CDD_MACRO || !d->def || hashmap_contains(t->index, &d->name)) {
				continue; // (the first definition is used)
			}
			macro mc = {.decl = d, .file = f, .params = NULL, .body = NULL,
			            .analysed = false, .kind = CDG_NONE, .patterns = NULL
			           };
			if (!parse_def(d->def, &mc)) {
				if (mc.params) {
					DESTROY(mc.params, finaliser_cons(FNR(vec), finaliser_new_ptr()));
				}
				continue;
			}
			char *name = cstr_clone(d->name);
			vec_push_rawptr(t->names, name);
			hashmap_ins_size_t(t->index, &name, vec_len(t->macros));
			vec_push(t->macros, &mc);
		}
	}
	// identifiers used by macros: in bodies, and in invocation arguments
	for (size_t i = 0, n = vec_len(t->macros); i < n; i++) {
		const macro *m = vec_get(t->macros, i);
		mark_used(t, m->body, 0);
	}
	for (size_t i = 0, n = vec_len(files); i < n; i++) {
		const cdfile *f = vec_get_rawptr(files, i);
		for (size_t j = 0, m = vec_len(f->decls); j < m; j++) {
			const cddecl *d = vec_get(f->decls, j);
			if (d->kind == CDD_MACROCALL) {
				vec *v = lex(d->sig);
				mark_used(t, v, 1); // (not the invoked macro itself)
				toks_free(v);
			}
		}
	}
	return t;
}


void cdmacrotab_free(cdmacrotab *self)
{
	if (!self) {
		return;
	}
	for (size_t i = 0, n = vec_len(self->macros); i < n; i++) {
		macro *m = vec_get_mut(self->macros, i);
		DESTROY(m->params, finaliser_cons(FNR(vec), finaliser_new_ptr()));
		toks_free(m->body);
		if (m->patterns) {
			cddecl_vec_free(m->patterns);
		}
	}
	DESTROY_FLAT(self->macros, vec);
	DESTROY_FLAT(self->index, hashmap);
	DESTROY_FLAT(self->invoked, hashmap);
	DESTROY_FLAT(self->used, hashmap);
	DESTROY(self->names, finaliser_cons(FNR(vec), finaliser_new_ptr()));
	FREE(self);
}


bool cdmacro_used_by_macros(const cdmacrotab *self, const char *name)
{
	return hashmap_contains(self->used, &name);
}


bool cdmacro_invoked(const cdmacrotab *self, const char *name)
{
	return hashmap_contains(self->invoked, &name);
}


/*
 * Expansion
 */

typedef struct {
	const cdmacrotab *t;
	bool failed;         // a limit was exceeded
	char err[256];
} xstate;


static bool disabled(const vec *dis, const char *name)
{
	for (size_t i = 0, n = vec_len(dis); i < n; i++) {
		if (strcmp(vec_get_rawptr(dis, i), name) == 0) {
			return true;
		}
	}
	return false;
}


// If v[i] starts a call of a function-like macro, returns it and sets
// *close to the index of the closing parenthesis and args to the arguments
// (vec of token vecs). Otherwise returns NULL.
static const macro *call_at(const cdmacrotab *t, const vec *v, size_t i, const vec *dis,
                            size_t *close, vec **args)
{
	if (!T(v, i)->ident || !is_p(v, i + 1, "(")) {
		return NULL;
	}
	const macro *m = find_macro(t, T(v, i)->s);
	if (!m || (dis && disabled(dis, T(v, i)->s))) {
		return NULL;
	}
	*args = vec_new(sizeof(vec *));
	vec *cur = toks_new();
	int depth = 0;
	size_t k = i + 2;
	for (; k < vec_len(v); k++) {
		if (is_p(v, k, "(")) {
			depth++;
		} else if (is_p(v, k, ")")) {
			if (depth == 0) {
				break;
			}
			depth--;
		} else if (is_p(v, k, ",") && depth == 0) {
			vec_push_rawptr(*args, cur);
			cur = toks_new();
			continue;
		}
		toks_push(cur, T(v, k)->s, strlen(T(v, k)->s), T(v, k)->ident);
	}
	vec_push_rawptr(*args, cur);
	if (vec_len(*args) == 1 && vec_len(cur) == 0 && vec_len(m->params) == 0) {
		toks_free(cur); // NAME() with no parameters
		vec_pop_rawptr(*args, 0);
	}
	*close = k;
	return m;
}


static void args_free(vec *args)
{
	if (!args) {
		return;
	}
	for (size_t i = 0, n = vec_len(args); i < n; i++) {
		toks_free(vec_get_rawptr(args, i));
	}
	DESTROY_FLAT(args, vec);
}


static bool ident_like(const char *s)
{
	if (!*s || !(isalpha((unsigned char)*s) || *s == '_')) {
		return false;
	}
	for (; *s; s++) {
		if (!isalnum((unsigned char)*s) && *s != '_') {
			return false;
		}
	}
	return true;
}


// Appends t to out, pasting it to the last token of out if paste
static void emit(vec *out, const tok *t, bool *paste)
{
	if (*paste && vec_len(out) > 0) {
		tok *last = vec_get_mut(out, vec_len(out) - 1);
		char *s = cstr_join("", 2, last->s, t->s);
		FREE(last->s);
		last->s = s;
		last->ident = ident_like(s);
	} else {
		toks_push(out, t->s, strlen(t->s), t->ident);
	}
	*paste = false;
}


// The tokens of the argument named name (empty if not given), or NULL if
// name is not a parameter
static const vec *arg_of(const macro *m, const vec *args, const char *name, const vec *va,
                         const vec *empty)
{
	for (size_t p = 0, n = vec_len(m->params); p < n; p++) {
		if (strcmp(vec_get_rawptr(m->params, p), name) == 0) {
			return (p < vec_len(args)) ? vec_get_rawptr(args, p) : empty;
		}
	}
	if (m->variadic && strcmp(name, "__VA_ARGS__") == 0) {
		return va;
	}
	return NULL;
}


// The body of m with its parameters replaced by args, and ## pasted
static vec *substitute(const macro *m, const vec *args)
{
	// __VA_ARGS__: the arguments after the named ones, separated by commas
	vec *va = toks_new();
	for (size_t a = vec_len(m->params); m->variadic && a < vec_len(args); a++) {
		if (a > vec_len(m->params)) {
			toks_push(va, ",", 1, false);
		}
		const vec *arg = vec_get_rawptr(args, a);
		for (size_t k = 0, n = vec_len(arg); k < n; k++) {
			toks_push(va, T(arg, k)->s, strlen(T(arg, k)->s), T(arg, k)->ident);
		}
	}
	vec *empty = toks_new();
	vec *out = toks_new();
	bool paste = false;
	const vec *b = m->body;
	for (size_t k = 0, n = vec_len(b); k < n; k++) {
		if (is_p(b, k, "#") && is_p(b, k + 1, "#")) {
			paste = vec_len(out) > 0;
			k++;
			continue;
		}
		const vec *rep = T(b, k)->ident ? arg_of(m, args, T(b, k)->s, va, empty) : NULL;
		if (rep) {
			for (size_t r = 0, rn = vec_len(rep); r < rn; r++) {
				emit(out, T(rep, r), &paste);
			}
			paste = false;
		} else {
			emit(out, T(b, k), &paste);
		}
	}
	toks_free(va);
	toks_free(empty);
	return out;
}


// Expands all macro calls in v (rescanning their expansions)
static vec *expand(xstate *x, const vec *v, vec *dis, int depth)
{
	vec *out = toks_new();
	if (depth > MAX_DEPTH) {
		x->failed = true;
		snprintf(x->err, sizeof(x->err), "macro expansion too deep");
		return out;
	}
	for (size_t i = 0, n = vec_len(v); i < n && !x->failed; i++) {
		size_t close;
		vec *args = NULL;
		const macro *m = call_at(x->t, v, i, dis, &close, &args);
		if (!m) {
			toks_push(out, T(v, i)->s, strlen(T(v, i)->s), T(v, i)->ident);
			continue;
		}
		vec *body = substitute(m, args);
		args_free(args);
		vec_push_rawptr(dis, m->decl->name);
		vec *exp = expand(x, body, dis, depth + 1);
		vec_pop_rawptr(dis, vec_len(dis) - 1);
		for (size_t k = 0, en = vec_len(exp); k < en; k++) {
			toks_push(out, T(exp, k)->s, strlen(T(exp, k)->s), T(exp, k)->ident);
		}
		toks_free(exp);
		toks_free(body);
		if (vec_len(out) > MAX_TOKENS) {
			x->failed = true;
			snprintf(x->err, sizeof(x->err), "macro expansion too large");
		}
		i = close;
	}
	return out;
}


// Whether v consists only of macro calls (possibly separated by ';' or ',')
static bool only_calls(const cdmacrotab *t, const vec *v)
{
	for (size_t i = 0, n = vec_len(v); i < n; i++) {
		if (is_p(v, i, ";") || is_p(v, i, ",") || strncmp(T(v, i)->s, "/**<", 4) == 0) {
			continue;
		}
		size_t close;
		vec *args = NULL;
		const macro *m = call_at(t, v, i, NULL, &close, &args);
		args_free(args);
		if (!m) {
			return false;
		}
		i = close;
	}
	return true;
}


/*
 * Generators
 */

static vec *match_text(const char *text)
{
	vec *cts = cdlex_all(text, strlen(text));
	vec *decls = cddecl_match(text, cts);
	DESTROY_FLAT(cts, vec);
	// keep declarations only
	vec *ret = vec_new(sizeof(cddecl));
	for (size_t i = 0, n = vec_len(decls); i < n; i++) {
		cddecl *d = vec_get_mut(decls, i);
		if (d->kind == CDD_FILE || d->kind == CDD_MACROCALL || d->kind == CDD_MACRO
		        || d->kind == CDD_UNKNOWN || d->name[0] == '\0') {
			cddecl_finalise(d, NULL);
		} else {
			vec_push(ret, d);
		}
	}
	DESTROY_FLAT(decls, vec);
	return ret;
}


static void analyse(const cdmacrotab *t, macro *m)
{
	if (m->analysed) {
		return;
	}
	m->analysed = true;
	// the generator's parameters as arguments: placeholders
	vec *args = vec_new(sizeof(vec *));
	for (size_t p = 0, n = vec_len(m->params); p < n; p++) {
		vec *a = toks_new();
		const char *name = vec_get_rawptr(m->params, p);
		toks_push(a, name, strlen(name), true);
		vec_push_rawptr(args, a);
	}
	vec *body = substitute(m, args);
	args_free(args);
	if (vec_len(body) == 0) {
		m->kind = CDG_NONE;
	} else if (only_calls(t, body)) {
		m->kind = CDG_COMPOSITE;
	} else {
		xstate x = {.t = t, .failed = false};
		vec *dis = vec_new(sizeof(char *));
		vec_push_rawptr(dis, m->decl->name);
		vec *exp = expand(&x, body, dis, 1);
		DESTROY_FLAT(dis, vec);
		char *text = render(exp);
		toks_free(exp);
		m->patterns = match_text(text);
		FREE(text);
		m->kind = vec_len(m->patterns) > 0 ? CDG_LEAF : CDG_NONE;
	}
	toks_free(body);
}


cdgen_kind cdmacro_generator(const cdmacrotab *self, const cddecl *decl,
                             const vec **patterns)
{
	if (patterns) {
		*patterns = NULL;
	}
	if (decl->kind != CDD_MACRO) {
		return CDG_NONE;
	}
	const macro *cm = find_macro(self, decl->name);
	if (!cm || cm->decl != decl) {
		return CDG_NONE;
	}
	macro *m = (macro *)cm;
	analyse(self, m);
	if (patterns) {
		*patterns = m->patterns;
	}
	return m->kind;
}


/*
 * Families
 */

typedef struct {
	const macro *m;
	char *args;          // e.g. "int"
	char *text;          // the expanded declarations
} leaf;


static char *args_text(const vec *args)
{
	strbuf *sb = strbuf_new();
	for (size_t a = 0, n = vec_len(args); a < n; a++) {
		char *s = render(vec_get_rawptr(args, a));
		if (s[0]) {
			strbuf_append(sb, strbuf_len(sb) ? ", " : "");
			strbuf_append(sb, s);
		}
		FREE(s);
	}
	return strbuf_detach(sb);
}


// Expands the calls in v down to leaf generator calls, collected in leaves
static void collect(xstate *x, const vec *v, vec *dis, int depth, vec *leaves)
{
	if (depth > MAX_DEPTH) {
		x->failed = true;
		snprintf(x->err, sizeof(x->err), "macro expansion too deep");
		return;
	}
	for (size_t i = 0, n = vec_len(v); i < n && !x->failed; i++) {
		size_t close;
		vec *args = NULL;
		const macro *m = call_at(x->t, v, i, dis, &close, &args);
		if (!m) {
			continue;
		}
		vec *body = substitute(m, args);
		vec_push_rawptr(dis, m->decl->name);
		if (only_calls(x->t, body)) {
			collect(x, body, dis, depth + 1, leaves);
		} else {
			vec *exp = expand(x, body, dis, depth + 1);
			leaf l = {.m = m, .args = args_text(args), .text = render(exp)};
			vec_push(leaves, &l);
			toks_free(exp);
		}
		vec_pop_rawptr(dis, vec_len(dis) - 1);
		toks_free(body);
		args_free(args);
		if (vec_len(leaves) > MAX_TOKENS) {
			x->failed = true;
			snprintf(x->err, sizeof(x->err), "macro expansion too large");
		}
		i = close;
	}
}


static void warn(vec *warns, size_t line, const char *fmt, ...)
{
	char buf[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	cdmacrowarn w = {.line = line, .msg = cstr_clone(buf)};
	vec_push(warns, &w);
}


static cdfamily *find_family(vec *fams, const cddecl *gen, size_t index)
{
	for (size_t i = 0, n = vec_len(fams); i < n; i++) {
		cdfamily *f = vec_get_mut(fams, i);
		if (f->gen == gen && f->index == index) {
			return f;
		}
	}
	return NULL;
}


static void add_via(cdfamily *fam, const char *via)
{
	for (size_t i = 0, n = vec_len(fam->via); i < n; i++) {
		if (strcmp(vec_get_rawptr(fam->via, i), via) == 0) {
			return;
		}
	}
	vec_push_rawptr(fam->via, cstr_clone(via));
}


vec *cdmacro_families(const cdmacrotab *self, const cdfile *f, vec *warns)
{
	vec *fams = vec_new(sizeof(cdfamily));
	for (size_t i = 0, n = vec_len(f->decls); i < n; i++) {
		const cddecl *call = vec_get(f->decls, i);
		if (call->kind != CDD_MACROCALL) {
			continue;
		}
		if (!find_macro(self, call->name)) {
			warn(warns, call->line, "cannot expand %s: %s is not a function-like macro of "
			     "the documented headers", call->sig, call->name);
			continue;
		}
		xstate x = {.t = self, .failed = false};
		vec *v = lex(call->sig);
		vec *dis = vec_new(sizeof(char *));
		vec *leaves = vec_new(sizeof(leaf));
		collect(&x, v, dis, 0, leaves);
		if (x.failed) {
			warn(warns, call->line, "cannot expand %s: %s", call->sig, x.err);
		}
		for (size_t l = 0, ln = vec_len(leaves); l < ln; l++) {
			leaf *lf = vec_get_mut(leaves, l);
			macro *gm = (macro *)lf->m;
			analyse(self, gm);
			vec *decls = match_text(lf->text);
			for (size_t k = 0, kn = vec_len(decls); k < kn; k++) {
				cdfamily *fam = find_family(fams, gm->decl, k);
				if (!fam) {
					cdfamily nf = {
						.gen = gm->decl, .gen_file = gm->file, .index = k,
						.pattern = (gm->patterns && k < vec_len(gm->patterns))
						           ? vec_get(gm->patterns, k) : NULL,
						.instances = vec_new(sizeof(cdinstance)),
						.via = vec_new(sizeof(char *)), .line = call->line,
						.doc_decl = NULL
					};
					// the doc above the generator documents its single declaration
					if (gm->decl->doc && gm->patterns && vec_len(gm->patterns) == 1) {
						nf.doc_decl = gm->decl;
					} else if (call->doc) {
						nf.doc_decl = call; // (used only if a single instance)
					}
					vec_push(fams, &nf);
					fam = vec_get_mut(fams, vec_len(fams) - 1);
				}
				cdinstance in = {.args = cstr_clone(lf->args), .decl = NEW(cddecl)};
				*in.decl = *(cddecl *)vec_get(decls, k); // moved
				vec_push(fam->instances, &in);
				add_via(fam, call->sig);
			}
			DESTROY_FLAT(decls, vec); // (the declarations were moved)
			FREE(lf->args);
			FREE(lf->text);
		}
		DESTROY_FLAT(leaves, vec);
		DESTROY_FLAT(dis, vec);
		toks_free(v);
	}
	// a doc above an invocation documents a family only if it has one instance
	for (size_t i = 0, n = vec_len(fams); i < n; i++) {
		cdfamily *fam = vec_get_mut(fams, i);
		if (fam->doc_decl && fam->doc_decl->kind == CDD_MACROCALL
		        && vec_len(fam->instances) != 1) {
			fam->doc_decl = NULL;
		}
	}
	return fams;
}


void cdfamily_vec_free(vec *families)
{
	if (!families) {
		return;
	}
	for (size_t i = 0, n = vec_len(families); i < n; i++) {
		cdfamily *fam = vec_get_mut(families, i);
		for (size_t k = 0, kn = vec_len(fam->instances); k < kn; k++) {
			cdinstance *in = vec_get_mut(fam->instances, k);
			FREE(in->args);
			cddecl_finalise(in->decl, NULL);
			FREE(in->decl);
		}
		DESTROY_FLAT(fam->instances, vec);
		DESTROY(fam->via, finaliser_cons(FNR(vec), finaliser_new_ptr()));
	}
	DESTROY_FLAT(families, vec);
}


void cdmacrowarn_vec_free(vec *warns)
{
	if (!warns) {
		return;
	}
	for (size_t i = 0, n = vec_len(warns); i < n; i++) {
		FREE(((cdmacrowarn *)vec_get_mut(warns, i))->msg);
	}
	DESTROY_FLAT(warns, vec);
}


const char *cdfamily_name(const cdfamily *fam)
{
	if (vec_len(fam->instances) == 1 || !fam->pattern) {
		return ((const cdinstance *)vec_get(fam->instances, 0))->decl->name;
	}
	return fam->pattern->name;
}
