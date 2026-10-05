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
 * @file cdmd.c
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "cddecl.h"
#include "cddoc.h"
#include "cdmacro.h"
#include "cdmd.h"
#include "cdversion.h"
#include "cstrutil.h"
#include "new.h"
#include "strbuf.h"


static inline bool is_ident_char(char c)
{
	return isalnum((unsigned char)c) || c == '_';
}


char *cdmd_page_name(const cdfile *f)
{
	strbuf *sb = strbuf_new();
	size_t n = strlen(f->name);
	if (n > 2 && strcmp(f->name + n - 2, ".h") == 0) {
		n -= 2;
	}
	strbuf_nappend(sb, f->name, n);
	strbuf_append(sb, ".md");
	return strbuf_detach(sb);
}


// GitLab/GitHub heading anchor: lowercase, keeping letters, digits, '-'
// and '_' (spaces would become '-', but names have none)
static void append_anchor(strbuf *sb, const char *heading)
{
	for (const char *c = heading; *c; c++) {
		if (is_ident_char(*c) || *c == '-') {
			strbuf_append_char(sb, (char)tolower((unsigned char)*c));
		}
	}
}


// Appends a link to s, with the given text
static void append_link(strbuf *out, const char *text, size_t tlen, const cdsym *s,
                        const cdfile *from)
{
	strbuf_append_char(out, '[');
	strbuf_nappend(out, text, tlen);
	strbuf_append(out, "](");
	if (s->file != from) {
		char *page = cdmd_page_name(s->file);
		strbuf_append(out, page);
		FREE(page);
	}
	if (s->kind == CDD_FILE) {
		if (s->file == from) {
			strbuf_append_char(out, '#');
			append_anchor(out, s->file->name);
		}
	} else if (s->family) {
		strbuf_append_char(out, '#');
		append_anchor(out, cdfamily_name(s->family));
	} else {
		strbuf_append_char(out, '#');
		append_anchor(out, (s->kind == CDD_MEMBER && s->parent) ? s->parent->name : s->name);
	}
	strbuf_append_char(out, ')');
}


static bool hidden_generator(const cddecl *d, const cdsymtab *tab);


// Appends name as a link if it resolves, else as code
static void append_ref(strbuf *out, const char *name, size_t len, const cdfile *f,
                       const cdsymtab *tab)
{
	char *key = cstr_clone_len(name, len);
	const cdsym *s = tab ? cdsymtab_resolve(tab, key, f, NULL) : NULL;
	if (s && s->decl && hidden_generator(s->decl, tab)) {
		s = NULL; // not on any page
	}
	if (s) {
		append_link(out, name, len, s, f);
	} else {
		strbuf_append_char(out, '`');
		strbuf_nappend(out, name, len);
		strbuf_append_char(out, '`');
	}
	FREE(key);
}


// Converts the references of a line of text (not code)
static void append_inline(strbuf *out, const char *line, size_t len, const cdfile *f,
                          const cdsymtab *tab)
{
	bool in_span = false;
	for (size_t i = 0; i < len;) {
		char c = line[i];
		if (c == '`') {
			in_span = !in_span;
			strbuf_append_char(out, c);
			i++;
			continue;
		}
		bool word_start = i == 0 || (!is_ident_char(line[i - 1]) && line[i - 1] != '&');
		if (!in_span && c == '@' && i + 2 < len && line[i + 1] == 'p'
		        && isspace((unsigned char)line[i + 2]) && (i == 0 || isspace((unsigned char)line[i - 1]))) {
			// @p name -> `name`
			size_t b = i + 2;
			while (b < len && (line[b] == ' ' || line[b] == '\t')) b++;
			size_t e = b;
			while (e < len && is_ident_char(line[e])) e++;
			if (e > b) {
				strbuf_append_char(out, '`');
				strbuf_nappend(out, line + b, e - b);
				strbuf_append_char(out, '`');
				i = e;
				continue;
			}
		} else if (!in_span && c == ':' && i + 2 < len && line[i + 1] == ':' && word_start
		           && (isalpha((unsigned char)line[i + 2]) || line[i + 2] == '_')) {
			// legacy Doxygen ::name (DC11 warns about it)
			size_t b = i + 2, e = b;
			while (e < len && (is_ident_char(line[e]) || (line[e] == '.' && e + 1 < len
			                   && is_ident_char(line[e + 1])))) e++;
			append_ref(out, line + b, e - b, f, tab);
			i = e;
			continue;
		} else if (!in_span && c == '#' && word_start && i + 1 < len
		           && (isalpha((unsigned char)line[i + 1]) || line[i + 1] == '_')) {
			// #name, #type.member
			size_t b = i + 1, e = b;
			while (e < len && is_ident_char(line[e])) e++;
			if (e + 1 < len && line[e] == '.' && (isalpha((unsigned char)line[e + 1]) || line[e + 1] == '_')) {
				e++;
				while (e < len && is_ident_char(line[e])) e++;
			}
			append_ref(out, line + b, e - b, f, tab);
			i = e;
			continue;
		} else if (!in_span && is_ident_char(c) && word_start) {
			// name.h -> link to its page, if known
			size_t e = i;
			while (e < len && is_ident_char(line[e])) e++;
			if (e + 1 < len && line[e] == '.' && line[e + 1] == 'h'
			        && (e + 2 == len || !is_ident_char(line[e + 2]))) {
				char *key = cstr_clone_len(line + i, e + 2 - i);
				const cdsym *s = tab ? cdsymtab_resolve(tab, key, f, NULL) : NULL;
				FREE(key);
				if (s && s->kind == CDD_FILE) {
					append_link(out, line + i, e + 2 - i, s, f);
					i = e + 2;
					continue;
				}
			}
			strbuf_nappend(out, line + i, e - i);
			i = e;
			continue;
		}
		if (c == '<' && i + 1 < len
		        && (isalpha((unsigned char)line[i + 1]) || line[i + 1] == '/')) {
			// common HTML tags (not allowed in comments, DC12) become
			// Markdown; anything else that looks like a tag is escaped
			static const char *TAGS[][2] = {
				{"b", "**"}, {"strong", "**"}, {"i", "*"}, {"em", "*"},
				{"tt", "`"}, {"code", "`"}
			};
			size_t k = i + 1 + (line[i + 1] == '/');
			size_t e = k;
			while (e < len && isalpha((unsigned char)line[e])) e++;
			bool done = false;
			for (size_t t = 0; e < len && line[e] == '>' && t < sizeof(TAGS) / sizeof(TAGS[0]); t++) {
				if (strlen(TAGS[t][0]) == e - k && strncasecmp(TAGS[t][0], line + k, e - k) == 0) {
					strbuf_append(out, TAGS[t][1]);
					if (TAGS[t][1][0] == '`') {
						in_span = !in_span;
					}
					i = e + 1;
					done = true;
				}
			}
			if (done) {
				continue;
			}
			// other real HTML tags pass (e.g. tables); text like <id> or
			// j<pos is escaped
			static const char *HTML[] = {
				"table", "thead", "tbody", "tr", "td", "th", "br", "p", "ul", "ol", "li",
				"pre", "div", "span", "sup", "sub", "a", "hr", "u", "s", "img"
			};
			bool html = false;
			for (size_t t = 0; t < sizeof(HTML) / sizeof(HTML[0]) && !html; t++) {
				html = strlen(HTML[t]) == e - k && strncasecmp(HTML[t], line + k, e - k) == 0
				       && e < len && (line[e] == '>' || line[e] == ' ' || line[e] == '/');
			}
			if (!in_span && !html) {
				strbuf_append_char(out, '\\');
			}
		}
		strbuf_append_char(out, c);
		i++;
	}
}


// Appends a Markdown text: references are converted outside code, and
// headings are demoted by shift levels. Continuation lines are prefixed
// with indent (for list items and quotes).
static void append_text(strbuf *out, const char *text, const cdfile *f, const cdsymtab *tab,
                        int shift, const char *indent)
{
	bool fence = false;
	const char *line = text;
	bool first = true;
	while (*line) {
		size_t len = strcspn(line, "\n");
		if (!first) {
			strbuf_append_char(out, '\n');
			if (len > 0 || indent[0] == '>') {
				strbuf_append(out, indent);
			}
		}
		first = false;
		const char *t = line;
		while (t < line + len && (*t == ' ' || *t == '\t')) t++;
		if (strncmp(t, "```", 3) == 0 || strncmp(t, "~~~", 3) == 0) {
			fence = !fence;
			strbuf_nappend(out, line, len);
		} else if (fence) {
			strbuf_nappend(out, line, len);
		} else {
			size_t h = 0;
			while (h < len && line[h] == '#') h++;
			if (h > 0 && h < len && line[h] == ' ') {
				for (int k = 0; k < shift; k++) {
					strbuf_append_char(out, '#');
				}
			}
			append_inline(out, line, len, f, tab);
		}
		line += len + (line[len] == '\n');
	}
}


/*
 * Declarations
 */

// The code shown for a declaration
static void append_code(strbuf *out, const cddecl *d)
{
	strbuf_append(out, "```c\n");
	const char *body = strstr(d->sig, "{...}");
	if (body && d->members && vec_len(d->members) > 0) {
		bool is_enum = strstr(d->sig, "enum") != NULL;
		strbuf_nappend(out, d->sig, body - d->sig);
		strbuf_append(out, "{\n");
		for (size_t i = 0, n = vec_len(d->members); i < n; i++) {
			const cddecl *m = vec_get(d->members, i);
			strbuf_append_char(out, '\t');
			strbuf_append(out, m->sig);
			strbuf_append(out, is_enum ? (i + 1 < n ? "," : "") : ";");
			strbuf_append_char(out, '\n');
		}
		strbuf_append_char(out, '}');
		strbuf_append(out, body + 5);
		strbuf_append(out, ";\n");
	} else {
		strbuf_append(out, d->sig);
		if (d->kind != CDD_MACRO && d->kind != CDD_MACROCALL) {
			strbuf_append_char(out, ';');
		}
		strbuf_append_char(out, '\n');
	}
	strbuf_append(out, "```\n\n");
}


static const cdparam *find_param(const cddoc *doc, const char *name)
{
	for (size_t i = 0, n = vec_len(doc->params); i < n; i++) {
		const cdparam *p = vec_get(doc->params, i);
		if (strcmp(p->name, name) == 0) {
			return p;
		}
	}
	return NULL;
}


static void append_param(strbuf *out, const char *name, const cdparam *p, const cdfile *f,
                         const cdsymtab *tab)
{
	strbuf_append(out, "- `");
	strbuf_append(out, name);
	strbuf_append(out, "`");
	if (p && p->move) {
		strbuf_append(out, " **(moved)**");
	}
	strbuf_append(out, ": ");
	if (p && p->desc[0]) {
		append_text(out, p->desc, f, tab, 0, "  ");
	} else {
		strbuf_append(out, "*undocumented*");
	}
	strbuf_append_char(out, '\n');
}


static void append_see(strbuf *out, const char *see, const cdfile *f, const cdsymtab *tab,
                       bool *first)
{
	const char *s = see;
	while (*s) {
		size_t n = strcspn(s, ",");
		size_t b = 0, e = n;
		while (b < e && isspace((unsigned char)s[b])) b++;
		while (e > b && (isspace((unsigned char)s[e - 1]) || s[e - 1] == '.')) e--;
		if (b < e && s[b] == '#') b++;
		if (e > b) {
			strbuf_append(out, *first ? "" : ", ");
			*first = false;
			append_ref(out, s + b, e - b, f, tab);
		}
		s += n + (s[n] == ',');
	}
}


static void append_brief(strbuf *out, const cddoc *doc, const cdfile *f, const cdsymtab *tab)
{
	if (doc && doc->brief[0]) {
		append_text(out, doc->brief, f, tab, 0, "");
	} else {
		strbuf_append(out, "*Undocumented.*");
	}
}


// The documentation of a declaration (or file comment, if d is NULL),
// except its brief
static void append_body(strbuf *out, const cddecl *d, const cddoc *doc, const cdfile *f,
                        const cdsymtab *tab, int shift)
{
	if (!doc) {
		return;
	}
	if (doc->details[0]) {
		append_text(out, doc->details, f, tab, shift, "");
		strbuf_append(out, "\n\n");
	}

	// parameters, in declaration order
	bool named = false;
	vec *params = (d && !cddecl_is_generator(d)) ? cddecl_params(d, &named) : NULL;
	if (params && named) {
		if (vec_len(params) > 0) {
			strbuf_append(out, "**Parameters**\n\n");
			for (size_t i = 0, n = vec_len(params); i < n; i++) {
				const char *name = vec_get_rawptr(params, i);
				append_param(out, name, find_param(doc, name), f, tab);
			}
			strbuf_append_char(out, '\n');
		}
	} else if (vec_len(doc->params) > 0) {
		strbuf_append(out, "**Parameters**\n\n");
		for (size_t i = 0, n = vec_len(doc->params); i < n; i++) {
			const cdparam *p = vec_get(doc->params, i);
			append_param(out, p->name, p, f, tab);
		}
		strbuf_append_char(out, '\n');
	}
	if (params) {
		DESTROY(params, finaliser_cons(FNR(vec), finaliser_new_ptr()));
	}

	// return value
	bool returns = d && d->kind == CDD_FUNC && cddecl_returns_value(d);
	if (doc->ret || returns) {
		strbuf_append(out, "**Returns");
		strbuf_append(out, doc->ret_move ? "** (moved to the caller): " : ":** ");
		if (doc->ret && doc->ret[0]) {
			append_text(out, doc->ret, f, tab, 0, "");
		} else {
			strbuf_append(out, "*undocumented*");
		}
		strbuf_append(out, "\n\n");
	}

	for (size_t i = 0, n = vec_len(doc->warnings); i < n; i++) {
		strbuf_append(out, "> **Warning:** ");
		append_text(out, vec_get_rawptr(doc->warnings, i), f, tab, 0, "> ");
		strbuf_append(out, "\n\n");
	}
	for (size_t i = 0, n = vec_len(doc->notes); i < n; i++) {
		strbuf_append(out, "> **Note:** ");
		append_text(out, vec_get_rawptr(doc->notes, i), f, tab, 0, "> ");
		strbuf_append(out, "\n\n");
	}
	if (doc->deprecated) {
		strbuf_append(out, "**Deprecated.** ");
		append_text(out, doc->deprecated, f, tab, 0, "");
		strbuf_append(out, "\n\n");
	}
	if (vec_len(doc->see) > 0) {
		strbuf_append(out, "**See also:** ");
		bool first = true;
		for (size_t i = 0, n = vec_len(doc->see); i < n; i++) {
			append_see(out, vec_get_rawptr(doc->see, i), f, tab, &first);
		}
		strbuf_append(out, "\n\n");
	}
}


static void append_members(strbuf *out, const cddecl *d, const cdfile *f, const cdsymtab *tab)
{
	if (!d->members || vec_len(d->members) == 0) {
		return;
	}
	strbuf_append(out, "**Members**\n\n");
	for (size_t i = 0, n = vec_len(d->members); i < n; i++) {
		const cddecl *m = vec_get(d->members, i);
		strbuf_append(out, "- `");
		strbuf_append(out, m->name);
		strbuf_append(out, "`: ");
		cddoc *doc = m->doc ? cddoc_parse(m->doc, strlen(m->doc)) : NULL;
		if (doc && doc->brief[0]) {
			append_text(out, doc->brief, f, tab, 0, "  ");
		} else {
			strbuf_append(out, "*undocumented*");
		}
		cddoc_free(doc);
		strbuf_append_char(out, '\n');
	}
	strbuf_append_char(out, '\n');
}


typedef enum {
	PART_TYPES = 0,
	PART_GEN_TYPES,
	PART_FUNCTIONS,
	PART_GEN_FUNCTIONS,
	PART_MACROS,
	PART_NONE
} part;


static part part_of(const cddecl *d)
{
	switch (d->kind) {
	case CDD_TYPEDEF:
	case CDD_STRUCT:
	case CDD_UNION:
	case CDD_ENUM:
	case CDD_VAR:
		return PART_TYPES;
	case CDD_FUNC:
		return PART_FUNCTIONS;
	case CDD_MACRO:
		return PART_MACROS;
	default:
		return PART_NONE; // file comments, macro invocations (DC14)
	}
}


// A generator macro used only to generate families: by other macros, and
// never invoked directly (DC14). Such macros are not listed on the pages.
static bool hidden_generator(const cddecl *d, const cdsymtab *tab)
{
	return tab && d->kind == CDD_MACRO && cddecl_is_generator(d)
	       && cdmacro_used_by_macros(cdsymtab_macros(tab), d->name)
	       && !cdmacro_invoked(cdsymtab_macros(tab), d->name);
}


// Whether d is shown: named, not private-and-undocumented, first of its
// name, and not a hidden generator
static bool shown(const cddecl *d, const vec *decls, size_t pos, const cdsymtab *tab)
{
	if (d->name[0] == '\0' || (d->name[0] == '_' && !d->doc)) {
		return false;
	}
	if (hidden_generator(d, tab)) {
		return false;
	}
	for (size_t i = 0; i < pos; i++) {
		const cddecl *e = vec_get(decls, i);
		if (e->kind != CDD_FILE && strcmp(e->name, d->name) == 0) {
			return false;
		}
	}
	return true;
}


void cdmd_module_of(const char *path, char **module, char **rel)
{
	const char *src = NULL;
	if (strncmp(path, "src/", 4) == 0) {
		src = path;
	} else {
		const char *p = strstr(path, "/src/");
		src = p ? p + 1 : NULL;
	}
	if (src) {
		// the module is the last directory before src/
		const char *end = (src > path) ? src - 1 : src;
		const char *b = end;
		while (b > path && b[-1] != '/') b--;
		*module = (end > b) ? cstr_clone_len(b, end - b) : cstr_clone(".");
		*rel = cstr_clone(src + 4);
	} else {
		const char *slash = strrchr(path, '/');
		*module = slash ? cstr_clone_len(path, slash - path) : cstr_clone(".");
		*rel = cstr_clone(slash ? slash + 1 : path);
	}
}


static const char *PART_TITLES[] = {
	"Types and constants", "Macro-generated types and constants",
	"Functions", "Macro-generated functions", "Macros"
};


// An entry of a page: a declaration, or a family of generated ones
typedef struct {
	const cddecl *decl;      // the declaration shown
	const cdfamily *fam;     // or NULL
} item;


// The declaration shown for a family: its single instance, or its pattern
static const cddecl *family_decl(const cdfamily *fam)
{
	if (vec_len(fam->instances) == 1 || !fam->pattern) {
		return ((const cdinstance *)vec_get(fam->instances, 0))->decl;
	}
	return fam->pattern;
}


static const char *item_name(const item *it)
{
	return it->fam ? cdfamily_name(it->fam) : it->decl->name;
}


static cddoc *item_doc(const item *it)
{
	const cddecl *dd = it->fam ? it->fam->doc_decl : it->decl;
	return (dd && dd->doc) ? cddoc_parse(dd->doc, strlen(dd->doc)) : NULL;
}


// The items of a part of the page of f, in source order
static vec *items_of(const cdfile *f, const cdsymtab *tab, part p)
{
	vec *ret = vec_new(sizeof(item));
	if (p == PART_GEN_TYPES || p == PART_GEN_FUNCTIONS) {
		const vec *fams = tab ? cdsymtab_families(tab, f) : NULL;
		for (size_t i = 0, n = fams ? vec_len(fams) : 0; i < n; i++) {
			const cdfamily *fam = vec_get(fams, i);
			const cddecl *d = family_decl(fam);
			part fp = (d->kind == CDD_FUNC) ? PART_GEN_FUNCTIONS : PART_GEN_TYPES;
			if (fp == p) {
				item it = {.decl = d, .fam = fam};
				vec_push(ret, &it);
			}
		}
		return ret;
	}
	for (size_t i = 0, n = vec_len(f->decls); i < n; i++) {
		const cddecl *d = vec_get(f->decls, i);
		if (part_of(d) == p && shown(d, f->decls, i, tab)) {
			item it = {.decl = d, .fam = NULL};
			vec_push(ret, &it);
		}
	}
	return ret;
}


static void append_generated(strbuf *out, const cdfamily *fam)
{
	strbuf_append(out, "**Generated** by ");
	for (size_t i = 0, n = vec_len(fam->via); i < n; i++) {
		strbuf_append(out, i ? ", `" : "`");
		strbuf_append(out, vec_get_rawptr(fam->via, i));
		strbuf_append(out, "`");
	}
	if (vec_len(fam->instances) > 1) {
		strbuf_append(out, ": ");
		for (size_t i = 0, n = vec_len(fam->instances); i < n; i++) {
			strbuf_append(out, i ? ", `" : "`");
			strbuf_append(out, ((const cdinstance *)vec_get(fam->instances, i))->decl->name);
			strbuf_append(out, "`");
		}
	}
	strbuf_append(out, ".\n\n");
}


// For a generator macro shown on the page (e.g. DECL_TRAIT): what it declares
static void append_declares(strbuf *out, const cddecl *d, const cdsymtab *tab)
{
	const vec *pats = NULL;
	if (!tab || d->kind != CDD_MACRO
	        || cdmacro_generator(cdsymtab_macros(tab), d, &pats) != CDG_LEAF || !pats) {
		return;
	}
	strbuf_append(out, "**Declares:** ");
	for (size_t i = 0, n = vec_len(pats); i < n; i++) {
		const cddecl *p = vec_get(pats, i);
		strbuf_append(out, i ? ", `" : "`");
		strbuf_append(out, p->sig);
		strbuf_append(out, "`");
	}
	strbuf_append(out, "\n\n");
}


static const cddecl *file_comment(const cdfile *f)
{
	for (size_t i = 0, n = vec_len(f->decls); i < n; i++) {
		const cddecl *d = vec_get(f->decls, i);
		if (d->kind == CDD_FILE) {
			return d;
		}
	}
	return NULL;
}


static void append_contents(strbuf *out, const cdfile *f, const cdsymtab *tab)
{
	strbuf_append(out, "## Contents\n\n");
	bool any = false;
	for (part p = PART_TYPES; p < PART_NONE; p++) {
		vec *items = items_of(f, tab, p);
		if (vec_len(items) > 0) {
			strbuf_append(out, any ? "\n**" : "**");
			strbuf_append(out, PART_TITLES[p]);
			strbuf_append(out, "**\n\n");
			any = true;
		}
		for (size_t i = 0, n = vec_len(items); i < n; i++) {
			const item *it = vec_get(items, i);
			strbuf_append(out, "- [");
			strbuf_append(out, item_name(it));
			strbuf_append(out, "](#");
			append_anchor(out, item_name(it));
			strbuf_append(out, "): ");
			cddoc *doc = item_doc(it);
			if (doc && doc->brief[0]) {
				append_text(out, doc->brief, f, tab, 0, "  ");
			} else {
				strbuf_append(out, "*undocumented*");
			}
			cddoc_free(doc);
			strbuf_append_char(out, '\n');
		}
		DESTROY_FLAT(items, vec);
	}
	strbuf_append(out, any ? "\n" : "*Empty.*\n\n");
}


char *cdmd_page(const cdfile *f, const cdsymtab *tab)
{
	strbuf *out = strbuf_new();
	strbuf_append(out, "# ");
	strbuf_append(out, f->name);
	strbuf_append(out, "\n\n");

	// title, brief, authors, navigation
	const cddecl *file_doc = file_comment(f);
	cddoc *doc = file_doc ? cddoc_parse(file_doc->doc, strlen(file_doc->doc)) : NULL;
	append_brief(out, doc, f, tab);
	strbuf_append(out, "\n\n");
	if (doc && vec_len(doc->authors) > 0) {
		strbuf_append(out, "**Author");
		strbuf_append(out, vec_len(doc->authors) > 1 ? "s:** " : ":** ");
		for (size_t i = 0, n = vec_len(doc->authors); i < n; i++) {
			strbuf_append(out, i ? ", " : "");
			strbuf_append(out, vec_get_rawptr(doc->authors, i));
		}
		strbuf_append(out, "\n\n");
	}
	if (doc && vec_len(doc->ai) > 0) {
		strbuf_append(out, "**AI involvement:** ");
		for (size_t i = 0, n = vec_len(doc->ai); i < n; i++) {
			const cdai *ai = vec_get(doc->ai, i);
			const char *title = cdai_level_title(ai->level);
			strbuf_append(out, i ? "; " : "");
			if (title) {
				strbuf_append(out, "[");
				strbuf_append(out, title);
				strbuf_append(out, "](ai-levels.md#");
				append_anchor(out, ai->level);
				strbuf_append(out, ")");
			} else {
				strbuf_append(out, ai->level);
			}
			if (ai->agent[0]) {
				strbuf_append(out, " (");
				strbuf_append(out, ai->agent);
				strbuf_append(out, ")");
			}
		}
		strbuf_append(out, "\n\n");
	}
	char *module, *rel;
	cdmd_module_of(f->path, &module, &rel);
	strbuf_append(out, "[Description](#description) · [Contents](#contents) · "
	              "[Back to module index](index.md");
	strbuf *anchor = strbuf_new();
	append_anchor(anchor, module);
	if (strbuf_len(anchor) > 0) {
		strbuf_append_char(out, '#');
		strbuf_append(out, strbuf_as_str(anchor));
	}
	strbuf_free(anchor);
	strbuf_append(out, ")\n\n");
	FREE(module);
	FREE(rel);

	// 1. description
	strbuf_append(out, "## Description\n\n");
	size_t before = strbuf_len(out);
	append_body(out, NULL, doc, f, tab, 1);
	if (strbuf_len(out) == before) {
		strbuf_append(out, "*No description.*\n\n");
	}
	cddoc_free(doc);

	// contents
	append_contents(out, f, tab);

	// 2. types and constants, 2.1 macro-generated ones, 3. functions,
	// 3.1 macro-generated ones, 4. macros
	for (part p = PART_TYPES; p < PART_NONE; p++) {
		vec *items = items_of(f, tab, p);
		if (vec_len(items) > 0) {
			strbuf_append(out, "## ");
			strbuf_append(out, PART_TITLES[p]);
			strbuf_append(out, "\n\n");
		}
		for (size_t i = 0, n = vec_len(items); i < n; i++) {
			const item *it = vec_get(items, i);
			const cddecl *d = it->decl;
			strbuf_append(out, "### ");
			strbuf_append(out, item_name(it));
			strbuf_append(out, "\n\n");
			append_code(out, d);
			cddoc *ddoc = item_doc(it);
			append_brief(out, ddoc, f, tab);
			strbuf_append(out, "\n\n");
			append_body(out, d, ddoc, f, tab, 3);
			cddoc_free(ddoc);
			append_members(out, d, f, tab);
			if (it->fam) {
				append_generated(out, it->fam);
			} else {
				append_declares(out, d, tab);
			}
			strbuf_append(out, "[Back to contents](#contents)\n\n");
		}
		DESTROY_FLAT(items, vec);
	}

	strbuf_append(out, "---\n\nGenerated by [cocadoc](https://github.com/paguso/cocada) ");
	strbuf_append(out, cocadoc_version_str());
	return strbuf_detach(out);
}


/*
 * Index
 */

typedef struct {
	const cdfile *file;
	char *module;
	char *rel;
} index_entry;


static int cmp_entry(const void *a, const void *b)
{
	const index_entry *x = a, *y = b;
	return strcmp(x->rel, y->rel);
}


// Number of leading directory components shared by two relative paths
static size_t common_dirs(const char *a, const char *b)
{
	size_t n = 0;
	for (;;) {
		const char *sa = strchr(a, '/'), *sb = strchr(b, '/');
		if (!sa || !sb || sa - a != sb - b || strncmp(a, b, sa - a) != 0) {
			return n;
		}
		n++;
		a = sa + 1;
		b = sb + 1;
	}
}


static void append_indent(strbuf *out, size_t depth)
{
	for (size_t i = 0; i < depth; i++) {
		strbuf_append(out, "  ");
	}
}


static void append_module(strbuf *out, const char *module, vec *entries)
{
	strbuf_append(out, "\n## ");
	strbuf_append(out, module);
	strbuf_append(out, "\n\n");
	vec_qsort(entries, cmp_entry);
	const char *prev = "";
	for (size_t i = 0, n = vec_len(entries); i < n; i++) {
		const index_entry *e = vec_get(entries, i);
		// open the directories not shared with the previous file
		size_t depth = common_dirs(prev, e->rel);
		const char *dir = e->rel;
		for (size_t k = 0; k < depth; k++) {
			dir = strchr(dir, '/') + 1;
		}
		for (const char *sl = strchr(dir, '/'); sl; sl = strchr(dir, '/')) {
			append_indent(out, depth);
			strbuf_append(out, "- ");
			strbuf_nappend(out, dir, sl - dir + 1);
			strbuf_append_char(out, '\n');
			depth++;
			dir = sl + 1;
		}
		append_indent(out, depth);
		char *page = cdmd_page_name(e->file);
		strbuf_append(out, "- [");
		strbuf_append(out, e->file->name);
		strbuf_append(out, "](");
		strbuf_append(out, page);
		strbuf_append(out, ")");
		FREE(page);
		const cddecl *fc = file_comment(e->file);
		if (fc) {
			cddoc *doc = cddoc_parse(fc->doc, strlen(fc->doc));
			if (doc->brief[0]) {
				strbuf_append(out, ": ");
				append_text(out, doc->brief, e->file, NULL, 0, "  ");
			}
			cddoc_free(doc);
		}
		strbuf_append_char(out, '\n');
		prev = e->rel;
	}
}


char *cdmd_index(const vec *files, const char *title)
{
	// modules in order of first appearance
	vec *modules = vec_new(sizeof(char *));
	vec *entries = vec_new(sizeof(index_entry));
	for (size_t i = 0, n = vec_len(files); i < n; i++) {
		index_entry e = {.file = vec_get_rawptr(files, i)};
		cdmd_module_of(e.file->path, &e.module, &e.rel);
		vec_push(entries, &e);
		bool seen = false;
		for (size_t j = 0, m = vec_len(modules); j < m && !seen; j++) {
			seen = strcmp(vec_get_rawptr(modules, j), e.module) == 0;
		}
		if (!seen) {
			vec_push_rawptr(modules, e.module);
		}
	}

	strbuf *out = strbuf_new();
	strbuf_append(out, "# ");
	strbuf_append(out, title);
	strbuf_append_char(out, '\n');
	for (size_t j = 0, m = vec_len(modules); j < m; j++) {
		const char *module = vec_get_rawptr(modules, j);
		vec *mine = vec_new(sizeof(index_entry));
		for (size_t i = 0, n = vec_len(entries); i < n; i++) {
			const index_entry *e = vec_get(entries, i);
			if (strcmp(e->module, module) == 0) {
				vec_push(mine, e);
			}
		}
		append_module(out, module, mine);
		DESTROY_FLAT(mine, vec);
	}
	strbuf_append(out, "\nThe AI involvement in writing each header is explained in "
	              "[AI involvement levels](ai-levels.md).\n");
	strbuf_append(out, "\n---\n\nGenerated by [cocadoc](https://github.com/paguso/cocada) ");
	strbuf_append(out, cocadoc_version_str());

	for (size_t i = 0, n = vec_len(entries); i < n; i++) {
		index_entry *e = vec_get_mut(entries, i);
		FREE(e->module);
		FREE(e->rel);
	}
	DESTROY_FLAT(entries, vec);
	DESTROY_FLAT(modules, vec);
	return strbuf_detach(out);
}


/*
 * AI involvement levels
 */

char *cdmd_ai_levels_page()
{
	static const char *LEVELS[][2] = {
		{"human", "Written by humans. AI was not used, or only like a search engine."},
		{"ai-informed", "Written by humans. AI explained, reviewed or suggested approaches, "
			"but wrote none of the code."},
		{"ai-assisted", "Mostly written by humans. AI wrote parts (fixes, functions, tests) "
			"that a human reviewed and integrated."},
		{"ai-generated", "Mostly or entirely written by AI, directed by a human: the human set "
			"the requirements and design decisions, and reviewed and approved the result."},
		{"ai-autonomous", "Written by AI with little or no human direction or review."},
	};
	strbuf *out = strbuf_new();
	strbuf_append(out, "# AI involvement levels\n\n"
	              "Each header declares, in its file comment, how much AI was involved in "
	              "writing it, as `@ai level, agent` (one line per AI used). The level is about "
	              "who wrote the code and who directed and checked it, not about the share of "
	              "lines, which cannot be measured reliably. When parts of a file are at "
	              "different levels, the level describes the bulk of it. In every case, the human who commits the file is "
	              "responsible for it.\n\n"
	              "Headers that declare no AI involvement are `human`, and their pages show no "
	              "note about it.\n\n"
	              "[Back to the index](index.md)\n");
	for (size_t i = 0; i < sizeof(LEVELS) / sizeof(LEVELS[0]); i++) {
		strbuf_append(out, "\n## ");
		strbuf_append(out, LEVELS[i][0]);
		strbuf_append(out, "\n\n**");
		strbuf_append(out, cdai_level_title(LEVELS[i][0]));
		strbuf_append(out, ".** ");
		strbuf_append(out, LEVELS[i][1]);
		strbuf_append_char(out, '\n');
	}
	strbuf_append(out, "\n---\n\nGenerated by [cocadoc](https://github.com/paguso/cocada) ");
	strbuf_append(out, cocadoc_version_str());
	return strbuf_detach(out);
}
