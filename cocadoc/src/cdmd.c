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
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "cddecl.h"
#include "cddoc.h"
#include "cdmd.h"
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
	} else {
		strbuf_append_char(out, '#');
		append_anchor(out, (s->kind == CDD_MEMBER && s->parent) ? s->parent->name : s->name);
	}
	strbuf_append_char(out, ')');
}


// Appends name as a link if it resolves, else as code
static void append_ref(strbuf *out, const char *name, size_t len, const cdfile *f,
                       const cdsymtab *tab)
{
	char *key = cstr_clone_len(name, len);
	const cdsym *s = tab ? cdsymtab_resolve(tab, key, f, NULL) : NULL;
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


// The documentation of a declaration (or file comment, if d is NULL)
static void append_doc(strbuf *out, const cddecl *d, const cddoc *doc, const cdfile *f,
                       const cdsymtab *tab, int shift)
{
	if (doc && doc->brief[0]) {
		append_text(out, doc->brief, f, tab, 0, "");
		strbuf_append(out, "\n\n");
	} else {
		strbuf_append(out, "*Undocumented.*\n\n");
	}
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
	PART_FUNCTIONS,
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


// Whether d is shown: named, not private-and-undocumented, first of its name
static bool shown(const cddecl *d, const vec *decls, size_t pos)
{
	if (d->name[0] == '\0' || (d->name[0] == '_' && !d->doc)) {
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


char *cdmd_page(const cdfile *f, const cdsymtab *tab)
{
	strbuf *out = strbuf_new();
	strbuf_append(out, "# ");
	strbuf_append(out, f->name);
	strbuf_append(out, "\n\n");

	// 1. module documentation
	const cddecl *file_doc = NULL;
	for (size_t i = 0, n = vec_len(f->decls); i < n && !file_doc; i++) {
		const cddecl *d = vec_get(f->decls, i);
		if (d->kind == CDD_FILE) {
			file_doc = d;
		}
	}
	cddoc *doc = file_doc ? cddoc_parse(file_doc->doc, strlen(file_doc->doc)) : NULL;
	append_doc(out, NULL, doc, f, tab, 1);
	if (doc && vec_len(doc->authors) > 0) {
		strbuf_append(out, "**Author");
		strbuf_append(out, vec_len(doc->authors) > 1 ? "s:** " : ":** ");
		for (size_t i = 0, n = vec_len(doc->authors); i < n; i++) {
			strbuf_append(out, i ? ", " : "");
			strbuf_append(out, vec_get_rawptr(doc->authors, i));
		}
		strbuf_append(out, "\n\n");
	}
	cddoc_free(doc);

	// 2-4. types and constants, functions, macros
	static const char *TITLES[] = {"Types and constants", "Functions", "Macros"};
	for (part p = PART_TYPES; p < PART_NONE; p++) {
		bool titled = false;
		for (size_t i = 0, n = vec_len(f->decls); i < n; i++) {
			const cddecl *d = vec_get(f->decls, i);
			if (part_of(d) != p || !shown(d, f->decls, i)) {
				continue;
			}
			if (!titled) {
				strbuf_append(out, "## ");
				strbuf_append(out, TITLES[p]);
				strbuf_append(out, "\n\n");
				titled = true;
			}
			strbuf_append(out, "### ");
			strbuf_append(out, d->name);
			strbuf_append(out, "\n\n");
			append_code(out, d);
			cddoc *ddoc = d->doc ? cddoc_parse(d->doc, strlen(d->doc)) : NULL;
			append_doc(out, d, ddoc, f, tab, 3);
			cddoc_free(ddoc);
			append_members(out, d, f, tab);
		}
	}

	// no trailing blank lines
	while (strbuf_len(out) > 1 && strbuf_get(out, strbuf_len(out) - 1) == '\n'
	        && strbuf_get(out, strbuf_len(out) - 2) == '\n') {
		strbuf_cut(out, strbuf_len(out) - 1, 1, NULL);
	}
	return strbuf_detach(out);
}


/*
 * Index
 */

static int cmp_path(const void *a, const void *b)
{
	const cdfile *x = *(const cdfile **)a, *y = *(const cdfile **)b;
	return strcmp(x->path, y->path);
}


static size_t dir_len(const char *path)
{
	const char *slash = strrchr(path, '/');
	return slash ? (size_t)(slash - path) : 0;
}


char *cdmd_index(const vec *files)
{
	size_t n = vec_len(files);
	const cdfile **sorted = malloc(n * sizeof(cdfile *));
	for (size_t i = 0; i < n; i++) {
		sorted[i] = vec_get_rawptr(files, i);
	}
	qsort(sorted, n, sizeof(cdfile *), cmp_path);

	strbuf *out = strbuf_new();
	strbuf_append(out, "# API reference\n");
	for (size_t i = 0; i < n; i++) {
		const cdfile *f = sorted[i];
		size_t dl = dir_len(f->path);
		if (i == 0 || dl != dir_len(sorted[i - 1]->path)
		        || strncmp(f->path, sorted[i - 1]->path, dl) != 0) {
			strbuf_append(out, "\n## ");
			if (dl) {
				strbuf_nappend(out, f->path, dl);
			} else {
				strbuf_append(out, ".");
			}
			strbuf_append(out, "\n\n");
		}
		char *page = cdmd_page_name(f);
		strbuf_append(out, "- [");
		strbuf_append(out, f->name);
		strbuf_append(out, "](");
		strbuf_append(out, page);
		strbuf_append(out, ")");
		FREE(page);
		for (size_t j = 0, m = vec_len(f->decls); j < m; j++) {
			const cddecl *d = vec_get(f->decls, j);
			if (d->kind == CDD_FILE) {
				cddoc *doc = cddoc_parse(d->doc, strlen(d->doc));
				if (doc->brief[0]) {
					strbuf_append(out, ": ");
					append_text(out, doc->brief, f, NULL, 0, "  ");
				}
				cddoc_free(doc);
				break;
			}
		}
		strbuf_append_char(out, '\n');
	}
	FREE(sorted);
	return strbuf_detach(out);
}
