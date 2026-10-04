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
#include <strings.h>

#include "cddoc.h"
#include "cstrutil.h"
#include "new.h"
#include "strbuf.h"


#define TABWIDTH 4


/*
 * Stripping the comment syntax
 */

static inline bool is_blank_str(const char *s)
{
	for (; *s; s++) {
		if (!isspace((unsigned char)*s)) {
			return false;
		}
	}
	return true;
}


// Copies s[0:len], expanding tabs in the leading whitespace and
// dropping trailing whitespace.
static char *clean_line(const char *s, size_t len)
{
	strbuf *sb = strbuf_new();
	size_t i = 0, col = 0;
	for (; i < len && (s[i] == ' ' || s[i] == '\t'); i++) {
		size_t w = (s[i] == '\t') ? TABWIDTH - (col % TABWIDTH) : 1;
		for (size_t k = 0; k < w; k++) {
			strbuf_append_char(sb, ' ');
		}
		col += w;
	}
	while (len > i && isspace((unsigned char)s[len - 1])) {
		len--;
	}
	strbuf_nappend(sb, s + i, len - i);
	return strbuf_detach(sb);
}


static size_t indent_of(const char *s)
{
	size_t n = 0;
	while (s[n] == ' ') {
		n++;
	}
	return n;
}


vec *cddoc_strip(const char *raw, size_t len)
{
	// delimiters: "/**" or "/**<" ... "*/", also "**/"
	size_t b = 0, e = len;
	if (len >= 3 && strncmp(raw, "/**", 3) == 0) {
		b = 3;
		if (b < e && raw[b] == '<') {
			b++;
		}
	}
	if (e >= b + 2 && raw[e - 2] == '*' && raw[e - 1] == '/') {
		e -= 2;
		while (e > b && raw[e - 1] == '*') {
			e--;
		}
	}

	// split into lines
	vec *spans = vec_new(2 * sizeof(size_t)); // (start, end) pairs
	for (size_t s = b; s <= e;) {
		size_t t = s;
		while (t < e && raw[t] != '\n') {
			t++;
		}
		size_t span[2] = {s, (t > s && raw[t - 1] == '\r') ? t - 1 : t};
		vec_push(spans, span);
		s = t + 1;
	}

	// the '*' column is removed only if every continuation line has it,
	// as a single '*' followed by whitespace or the end of the line
	// (so that e.g. a line starting with "**bold**" does not count)
	size_t n = vec_len(spans);
	bool star_col = n > 1;
	for (size_t i = 1; i < n && star_col; i++) {
		const size_t *sp = vec_get(spans, i);
		size_t k = sp[0];
		while (k < sp[1] && (raw[k] == ' ' || raw[k] == '\t')) {
			k++;
		}
		if (k < sp[1] && (raw[k] != '*'
		                  || (k + 1 < sp[1] && raw[k + 1] != ' ' && raw[k + 1] != '\t'))) {
			star_col = false;
		}
	}

	vec *lines = vec_new(sizeof(char *));
	for (size_t i = 0; i < n; i++) {
		const size_t *sp = vec_get(spans, i);
		size_t k = sp[0];
		if (i > 0 && star_col) {
			while (k < sp[1] && (raw[k] == ' ' || raw[k] == '\t')) {
				k++;
			}
			if (k < sp[1] && raw[k] == '*') {
				k++;
			}
		}
		char *line = clean_line(raw + k, sp[1] - k);
		vec_push_rawptr(lines, line);
	}
	DESTROY_FLAT(spans, vec);

	// the first line follows the opening delimiter: drop its indentation;
	// dedent the others by their common indentation
	size_t min_ind = SIZE_MAX;
	for (size_t i = 1; i < n; i++) {
		char *l = vec_get_rawptr(lines, i);
		if (!is_blank_str(l)) {
			size_t ind = indent_of(l);
			min_ind = (ind < min_ind) ? ind : min_ind;
		}
	}
	for (size_t i = 0; i < n; i++) {
		char *l = vec_get_rawptr(lines, i);
		size_t cut = (i == 0) ? indent_of(l) : (is_blank_str(l) ? strlen(l) : min_ind);
		if (cut > 0) {
			memmove(l, l + cut, strlen(l) - cut + 1);
		}
	}

	// drop leading and trailing blank lines
	// (FREE evaluates its argument twice: pop into a variable first)
	while (vec_len(lines) > 0 && is_blank_str(vec_last_rawptr(lines))) {
		char *l = vec_pop_rawptr(lines, vec_len(lines) - 1);
		FREE(l);
	}
	while (vec_len(lines) > 0 && is_blank_str(vec_first_rawptr(lines))) {
		char *l = vec_pop_rawptr(lines, 0);
		FREE(l);
	}
	return lines;
}


/*
 * Block commands
 */

typedef enum {
	SEC_DETAILS = 0,
	SEC_BRIEF,
	SEC_PARAM,
	SEC_RETURN,
	SEC_SEE,
	SEC_WARNING,
	SEC_NOTE,
	SEC_DEPRECATED,
	SEC_AUTHOR,
	SEC_PAR,
	SEC_FILE,
	SEC_NONE // not a block command
} sec_kind;


typedef struct {
	const char *name;
	sec_kind kind;
} cmd_def;


static const cmd_def BLOCK_CMDS[] = {
	{"brief", SEC_BRIEF}, {"short", SEC_BRIEF},
	{"param", SEC_PARAM},
	{"return", SEC_RETURN}, {"returns", SEC_RETURN}, {"result", SEC_RETURN},
	{"see", SEC_SEE}, {"sa", SEC_SEE},
	{"warning", SEC_WARNING}, {"warn", SEC_WARNING}, {"attention", SEC_WARNING},
	{"note", SEC_NOTE}, {"remark", SEC_NOTE}, {"remarks", SEC_NOTE},
	{"deprecated", SEC_DEPRECATED},
	{"author", SEC_AUTHOR}, {"authors", SEC_AUTHOR},
	{"par", SEC_PAR},
	{"file", SEC_FILE},
};


// Commands that are known but are not block commands
static const char *OTHER_CMDS[] = {
	"p", "a", "c", "e", "b", "em", "ref", "link", "endlink", "n",
	"code", "endcode", "verbatim", "endverbatim",
};


static sec_kind block_cmd(const char *w, size_t len)
{
	for (size_t i = 0; i < sizeof(BLOCK_CMDS) / sizeof(BLOCK_CMDS[0]); i++) {
		if (strlen(BLOCK_CMDS[i].name) == len
		        && strncmp(BLOCK_CMDS[i].name, w, len) == 0) {
			return BLOCK_CMDS[i].kind;
		}
	}
	return SEC_NONE;
}


static bool other_cmd(const char *w, size_t len)
{
	for (size_t i = 0; i < sizeof(OTHER_CMDS) / sizeof(OTHER_CMDS[0]); i++) {
		if (strlen(OTHER_CMDS[i]) == len && strncmp(OTHER_CMDS[i], w, len) == 0) {
			return true;
		}
	}
	return false;
}


/*
 * Parser state
 */

typedef enum {
	FENCE_NONE = 0,
	FENCE_BACKTICK,  // ```
	FENCE_TILDE,     // ~~~
	FENCE_CODE,      // @code ... @endcode
	FENCE_VERBATIM   // @verbatim ... @endverbatim
} fence_kind;


typedef struct {
	sec_kind kind;
	strbuf *text;
} section;


typedef struct {
	strbuf *details;
	vec *secs;         // vec of section, in order
	strbuf *cur;       // where text currently goes
	sec_kind cur_kind;
	fence_kind fence;
	vec *diags;
} pstate;


static void diag(pstate *st, const char *fmt, ...)
{
	char buf[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	vec_push_rawptr(st->diags, cstr_clone(buf));
}


static inline char last_char(strbuf *sb)
{
	size_t l = strbuf_len(sb);
	return l ? strbuf_get(sb, l - 1) : '\0';
}


// Appends text as a new line of sb
static void append_line(strbuf *sb, const char *text, size_t len)
{
	if (strbuf_len(sb) > 0 && last_char(sb) != '\n') {
		strbuf_append_char(sb, '\n');
	}
	strbuf_nappend(sb, text, len);
}


// Appends a line of a code block, which is kept even if blank
static void append_code_line(strbuf *sb, const char *text)
{
	if (strbuf_len(sb) > 0) {
		strbuf_append_char(sb, '\n');
	}
	strbuf_append(sb, text);
}


// Ends the current paragraph of sb with a blank line
static void para_break(strbuf *sb)
{
	size_t l = strbuf_len(sb);
	if (l == 0) {
		return;
	}
	if (last_char(sb) != '\n') {
		strbuf_append_char(sb, '\n');
	}
	if (strbuf_len(sb) < 2 || strbuf_get(sb, strbuf_len(sb) - 2) != '\n') {
		strbuf_append_char(sb, '\n');
	}
}


static void back_to_details(pstate *st)
{
	st->cur = st->details;
	st->cur_kind = SEC_DETAILS;
}


static void open_section(pstate *st, sec_kind kind)
{
	if (kind == SEC_PAR) {
		// a titled paragraph of the details
		back_to_details(st);
		para_break(st->details);
		st->cur_kind = SEC_PAR;
		return;
	}
	section s = {.kind = kind, .text = strbuf_new()};
	vec_push(st->secs, &s);
	st->cur = s.text;
	st->cur_kind = kind;
}


// Appends text (a line fragment) to the current section
static void add_text(pstate *st, const char *text, size_t len, bool first)
{
	if (st->cur_kind == SEC_PAR && first) {
		// @par title: rest of the line, as a bold paragraph
		while (len > 0 && isspace((unsigned char)text[0])) {
			text++;
			len--;
		}
		while (len > 0 && isspace((unsigned char)text[len - 1])) {
			len--;
		}
		if (len > 0 && islower((unsigned char)text[0])) {
			diag(st, "@par makes a titled paragraph (here titled \"%.*s\"); "
			     "did you mean @param or @p?", (int)(len > 60 ? 60 : len), text);
		}
		if (len > 0) {
			strbuf_append(st->details, "**");
			strbuf_nappend(st->details, text, len);
			strbuf_append(st->details, "**");
			para_break(st->details);
		}
		st->cur_kind = SEC_DETAILS;
		return;
	}
	if (st->cur_kind != SEC_DETAILS) {
		// block command text is reflowed: drop indentation
		while (len > 0 && isspace((unsigned char)text[0])) {
			text++;
			len--;
		}
	}
	while (len > 0 && isspace((unsigned char)text[len - 1])) {
		len--;
	}
	if (len == 0) {
		return;
	}
	append_line(st->cur, text, len);
}


// Processes a line that is not inside a code block
static void parse_line(pstate *st, const char *line)
{
	size_t len = strlen(line);
	size_t seg = 0;          // start of the current text segment
	bool seg_first = false;  // segment is the first after a command
	bool in_span = false;    // inside a `code span`
	for (size_t i = 0; i < len; i++) {
		char c = line[i];
		if (c == '`') {
			in_span = !in_span;
			continue;
		}
		if (in_span || (c != '@' && c != '\\')
		        || (i > 0 && !isspace((unsigned char)line[i - 1]))) {
			continue;
		}
		size_t w = i + 1, wend = w;
		while (wend < len && isalpha((unsigned char)line[wend])) {
			wend++;
		}
		if (wend == w) {
			continue;
		}
		sec_kind kind = block_cmd(line + w, wend - w);
		if (kind == SEC_NONE) {
			if (c == '@' && !other_cmd(line + w, wend - w)) {
				diag(st, "unknown command @%.*s (did you mean @p %.*s?)",
				     (int)(wend - w), line + w, (int)(wend - w), line + w);
			}
			continue;
		}
		add_text(st, line + seg, i - seg, seg_first);
		open_section(st, kind);
		seg = wend;
		seg_first = true;
		i = wend - 1;
	}
	add_text(st, line + seg, len - seg, seg_first);
}


static bool starts_with_word(const char *s, const char *w)
{
	size_t n = strlen(w);
	return strncmp(s, w, n) == 0 && !isalnum((unsigned char)s[n]);
}


static void parse_lines(pstate *st, const vec *lines)
{
	for (size_t i = 0, n = vec_len(lines); i < n; i++) {
		const char *line = vec_get_rawptr(lines, i);
		const char *t = line + indent_of(line);

		if (st->fence != FENCE_NONE) {
			bool closes =
			    (st->fence == FENCE_BACKTICK && strncmp(t, "```", 3) == 0)
			    || (st->fence == FENCE_TILDE && strncmp(t, "~~~", 3) == 0)
			    || (st->fence == FENCE_CODE && starts_with_word(t, "@endcode"))
			    || (st->fence == FENCE_VERBATIM && starts_with_word(t, "@endverbatim"));
			if (closes) {
				append_line(st->cur, (*t == '@') ? "```" : t, (*t == '@') ? 3 : strlen(t));
				st->fence = FENCE_NONE;
			} else {
				append_code_line(st->cur, line);
			}
			continue;
		}

		if (strncmp(t, "```", 3) == 0 || strncmp(t, "~~~", 3) == 0) {
			st->fence = (*t == '`') ? FENCE_BACKTICK : FENCE_TILDE;
			append_line(st->cur, t, strlen(t));
		} else if (starts_with_word(t, "@code")) {
			st->fence = FENCE_CODE;
			const char *lang = "c";
			size_t llen = 1;
			if (strncmp(t + 5, "{.", 2) == 0) { // @code{.py}
				lang = t + 7;
				llen = strcspn(lang, "}");
			}
			append_line(st->cur, "```", 3);
			strbuf_nappend(st->cur, lang, llen);
		} else if (starts_with_word(t, "@verbatim")) {
			st->fence = FENCE_VERBATIM;
			append_line(st->cur, "```", 3);
		} else if (*t == '\0') {
			// a blank line ends a block command
			back_to_details(st);
			para_break(st->details);
		} else {
			parse_line(st, line);
		}
	}
	if (st->fence != FENCE_NONE) {
		diag(st, "unterminated code block");
		append_line(st->cur, "```", 3);
	}
}


/*
 * Building the result
 */

static char *trimmed_copy(const char *s)
{
	size_t b = 0, e = strlen(s);
	while (b < e && isspace((unsigned char)s[b])) {
		b++;
	}
	while (e > b && isspace((unsigned char)s[e - 1])) {
		e--;
	}
	return cstr_clone_len(s + b, e - b);
}


// Joins the lines of s with spaces (for one-paragraph fields like the brief)
static void unwrap(strbuf *dest, const char *s)
{
	for (; *s; s++) {
		strbuf_append_char(dest, (*s == '\n') ? ' ' : *s);
	}
}


static bool is_abbrev_end(const char *text, size_t dot)
{
	static const char *ABBREVS[] = {"e.g", "i.e", "etc", "a.k.a", "vs", "cf"};
	for (size_t i = 0; i < sizeof(ABBREVS) / sizeof(ABBREVS[0]); i++) {
		size_t n = strlen(ABBREVS[i]);
		if (dot >= n && strncasecmp(text + dot - n, ABBREVS[i], n) == 0
		        && (dot == n || !isalnum((unsigned char)text[dot - n - 1]))) {
			return true;
		}
	}
	return false;
}


// Whether a paragraph is plain text, rather than a heading, list, code
// block, table, quote or HTML block
static bool starts_plain_text(const char *d)
{
	size_t digits = strspn(d, "0123456789");
	if (strchr("#|<>", d[0]) || strncmp(d, "```", 3) == 0 || strncmp(d, "~~~", 3) == 0) {
		return false;
	}
	if (strchr("-*+", d[0]) && d[1] == ' ') {
		return false; // bullet list
	}
	if (digits > 0 && (d[digits] == '.' || d[digits] == ')') && d[digits + 1] == ' ') {
		return false; // numbered list
	}
	return true;
}


// JAVADOC_AUTOBRIEF: the first sentence of the first paragraph of the
// details becomes the brief, and is removed from the details.
static void auto_brief(char **brief, char **details)
{
	const char *d = *details;
	const char *pe = strstr(d, "\n\n");
	size_t par_end = pe ? (size_t)(pe - d) : strlen(d);
	if (par_end == 0 || !starts_plain_text(d)) {
		return;
	}
	size_t end = par_end; // exclusive
	bool in_span = false;
	for (size_t i = 0; i < par_end; i++) {
		if (d[i] == '`') {
			in_span = !in_span;
		} else if (d[i] == '.' && !in_span && (i + 1 == par_end || isspace((unsigned char)d[i + 1]))
		           && !is_abbrev_end(d, i)) {
			end = i + 1;
			break;
		}
	}
	strbuf *sb = strbuf_new();
	char *head = cstr_clone_len(d, end);
	unwrap(sb, head);
	FREE(head);
	char *b = strbuf_detach(sb);
	FREE(*brief);
	*brief = trimmed_copy(b);
	FREE(b);
	char *rest = trimmed_copy(d + end);
	FREE(*details);
	*details = rest;
}


static cdownership parse_ownership(char **desc)
{
	// (**move**), (*no transfer*), (move), ...
	const char *s = *desc;
	if (*s != '(') {
		return CDO_UNSPECIFIED;
	}
	const char *p = s + 1;
	while (*p == ' ') p++;
	size_t stars = strspn(p, "*");
	if (stars > 2) {
		return CDO_UNSPECIFIED;
	}
	const char *w = p + stars;
	size_t wlen = strcspn(w, "*)");
	const char *q = w + wlen;
	if (strspn(q, "*") != stars) {
		return CDO_UNSPECIFIED;
	}
	q += stars;
	while (*q == ' ') q++;
	if (*q != ')') {
		return CDO_UNSPECIFIED;
	}
	cdownership own = CDO_UNSPECIFIED;
	if (wlen == 11 && strncasecmp(w, "no transfer", 11) == 0) {
		own = CDO_NO_TRANSFER;
	} else if ((wlen == 8 && strncasecmp(w, "transfer", 8) == 0)
	           || (wlen == 13 && strncasecmp(w, "full transfer", 13) == 0)) {
		own = CDO_TRANSFER;
	} else if (wlen == 4 && strncasecmp(w, "move", 4) == 0) {
		own = CDO_MOVE;
	}
	if (own != CDO_UNSPECIFIED) {
		char *rest = trimmed_copy(q + 1);
		FREE(*desc);
		*desc = rest;
	}
	return own;
}


static cdparam parse_param(pstate *st, const char *text)
{
	cdparam prm = {.name = NULL, .own = CDO_UNSPECIFIED, .desc = NULL};
	const char *s = text;
	if (*s == '[') { // direction, e.g. [in,out]
		s += strcspn(s, "]");
		if (*s) s++;
		while (isspace((unsigned char)*s)) s++;
	}
	size_t nlen = strcspn(s, " \t\n");
	while (nlen > 0 && (s[nlen - 1] == ',' || s[nlen - 1] == ':')) {
		nlen--;
	}
	prm.name = cstr_clone_len(s, nlen);
	if (nlen == 0) {
		diag(st, "@param without a name");
	}
	s += strcspn(s, " \t\n");
	prm.desc = trimmed_copy(s);
	prm.own = parse_ownership(&prm.desc);
	return prm;
}


static void cdparam_finalise(void *ptr, const finaliser *fnr)
{
	cdparam *p = (cdparam *)ptr;
	FREE(p->name);
	FREE(p->desc);
}


static vec *new_str_vec()
{
	return vec_new(sizeof(char *));
}


static void free_str_vec(vec *v)
{
	DESTROY(v, finaliser_cons(FNR(vec), finaliser_new_ptr()));
}


cddoc *cddoc_parse(const char *raw, size_t len)
{
	vec *lines = cddoc_strip(raw, len);

	pstate st = {
		.details = strbuf_new(),
		.secs = vec_new(sizeof(section)),
		.fence = FENCE_NONE,
		.diags = new_str_vec()
	};
	back_to_details(&st);
	parse_lines(&st, lines);
	free_str_vec(lines);

	cddoc *doc = NEW(cddoc);
	doc->params = vec_new(sizeof(cdparam));
	doc->ret = NULL;
	doc->see = new_str_vec();
	doc->warnings = new_str_vec();
	doc->notes = new_str_vec();
	doc->deprecated = NULL;
	doc->authors = new_str_vec();

	strbuf *brief = strbuf_new();
	strbuf *ret = NULL;
	for (size_t i = 0, n = vec_len(st.secs); i < n; i++) {
		section *s = vec_get_mut(st.secs, i);
		char *text = trimmed_copy(strbuf_as_str(s->text));
		strbuf_free(s->text);
		switch (s->kind) {
		case SEC_BRIEF:
			if (strbuf_len(brief) > 0) {
				strbuf_append_char(brief, ' ');
			}
			unwrap(brief, text);
			FREE(text);
			break;
		case SEC_PARAM: {
			cdparam p = parse_param(&st, text);
			vec_push(doc->params, &p);
			FREE(text);
			break;
		}
		case SEC_RETURN:
			if (!ret) {
				ret = strbuf_new();
			} else {
				strbuf_append(ret, "\n\n");
			}
			strbuf_append(ret, text);
			FREE(text);
			break;
		case SEC_SEE:
			vec_push_rawptr(doc->see, text);
			break;
		case SEC_WARNING:
			vec_push_rawptr(doc->warnings, text);
			break;
		case SEC_NOTE:
			vec_push_rawptr(doc->notes, text);
			break;
		case SEC_DEPRECATED:
			FREE(doc->deprecated);
			doc->deprecated = text;
			break;
		case SEC_AUTHOR:
			vec_push_rawptr(doc->authors, text);
			break;
		default: // @file: the declaration matcher takes care of it
			FREE(text);
			break;
		}
	}
	DESTROY_FLAT(st.secs, vec);

	doc->brief = trimmed_copy(strbuf_as_str(brief));
	strbuf_free(brief);
	doc->ret = ret ? strbuf_detach(ret) : NULL;
	doc->details = trimmed_copy(strbuf_as_str(st.details));
	strbuf_free(st.details);
	if (doc->brief[0] == '\0') {
		auto_brief(&doc->brief, &doc->details);
	}
	if (doc->brief[0] == '\0') {
		diag(&st, "no brief description");
	}
	doc->diags = st.diags;
	return doc;
}


void cddoc_free(cddoc *self)
{
	if (!self) {
		return;
	}
	FREE(self->brief);
	FREE(self->details);
	DESTROY(self->params, finaliser_cons(FNR(vec), finaliser_new(cdparam_finalise)));
	FREE(self->ret);
	free_str_vec(self->see);
	free_str_vec(self->warnings);
	free_str_vec(self->notes);
	FREE(self->deprecated);
	free_str_vec(self->authors);
	free_str_vec(self->diags);
	FREE(self);
}


const char *cdownership_name(cdownership own)
{
	switch (own) {
	case CDO_UNSPECIFIED:
		return "unspecified";
	case CDO_NO_TRANSFER:
		return "no transfer";
	case CDO_TRANSFER:
		return "transfer";
	case CDO_MOVE:
		return "move";
	}
	return "?";
}
