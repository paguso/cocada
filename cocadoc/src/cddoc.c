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
 * @file cddoc.c
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
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


// Splits the comment into lines without the comment syntax. *first is
// set to the number of leading blank lines dropped, so that line i of the
// result is line (*first + i) of the comment.
static vec *strip_lines(const char *raw, size_t len, size_t *first)
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
	*first = 0;
	while (vec_len(lines) > 0 && is_blank_str(vec_first_rawptr(lines))) {
		char *l = vec_pop_rawptr(lines, 0);
		FREE(l);
		(*first)++;
	}
	return lines;
}


vec *cddoc_strip(const char *raw, size_t len)
{
	size_t first;
	return strip_lines(raw, len, &first);
}


/*
 * Commands
 */

typedef enum {
	SEC_BRIEF = 0,
	SEC_DETAILS,
	SEC_PARAM,
	SEC_RETURN,
	SEC_WARNING,
	SEC_NOTE,
	SEC_DEPRECATED,
	SEC_SEE,
	SEC_AUTHOR,
	SEC_FILE,
	SEC_PAR,
	SEC_AI,
	SEC_HIDE,
	SEC_NONE // not a block command
} sec_kind;


// Position of each section in the prescribed order (DC7), or -1 if any
static int sec_rank(sec_kind k)
{
	return (k <= SEC_SEE) ? (int)k : -1;
}


static const char *sec_label(sec_kind k)
{
	static const char *LABELS[] = {"@brief", "details", "@param", "@return",
	                               "@warning", "@note", "@deprecated", "@see"
	                              };
	return (k <= SEC_SEE) ? LABELS[k] : "?";
}


typedef struct {
	const char *name;
	sec_kind kind;
	const char *use; // NULL if allowed, else what to use instead
} cmd_def;


static const cmd_def BLOCK_CMDS[] = {
	{"brief", SEC_BRIEF, NULL},
	{"param", SEC_PARAM, NULL},
	{"return", SEC_RETURN, NULL},
	{"see", SEC_SEE, NULL},
	{"warning", SEC_WARNING, NULL},
	{"note", SEC_NOTE, NULL},
	{"deprecated", SEC_DEPRECATED, NULL},
	{"author", SEC_AUTHOR, NULL},
	{"file", SEC_FILE, NULL},
	{"ai", SEC_AI, NULL},
	{"hide", SEC_HIDE, NULL},
	// accepted, but not allowed by the style
	{"short", SEC_BRIEF, "@brief"},
	{"returns", SEC_RETURN, "@return"},
	{"result", SEC_RETURN, "@return"},
	{"sa", SEC_SEE, "@see"},
	{"warn", SEC_WARNING, "@warning"},
	{"attention", SEC_WARNING, "@warning"},
	{"remark", SEC_NOTE, "@note"},
	{"remarks", SEC_NOTE, "@note"},
	{"authors", SEC_AUTHOR, "@author"},
	{"par", SEC_PAR, "a Markdown heading or **bold** text"},
};


// Inline Doxygen commands that are not allowed (Markdown is used instead)
static const char *INLINE_CMDS[] = {
	"a", "c", "e", "b", "em", "ref", "link", "endlink", "n",
};


static const cmd_def *find_block_cmd(const char *w, size_t len)
{
	for (size_t i = 0; i < sizeof(BLOCK_CMDS) / sizeof(BLOCK_CMDS[0]); i++) {
		if (strlen(BLOCK_CMDS[i].name) == len
		        && strncmp(BLOCK_CMDS[i].name, w, len) == 0) {
			return &BLOCK_CMDS[i];
		}
	}
	return NULL;
}


static bool word_in(const char *w, size_t len, const char **list, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		if (strlen(list[i]) == len && strncmp(list[i], w, len) == 0) {
			return true;
		}
	}
	return false;
}


static bool starts_with_word(const char *s, const char *w)
{
	size_t n = strlen(w);
	return strncmp(s, w, n) == 0 && !isalnum((unsigned char)s[n]) && s[n] != '_';
}


static inline bool is_ident_char(char c)
{
	return isalnum((unsigned char)c) || c == '_';
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
	size_t line;
} section;


typedef struct {
	strbuf *details;
	vec *secs;         // vec of section, in order
	strbuf *cur;       // where text currently goes
	sec_kind cur_kind;
	fence_kind fence;
	vec *diags;
	vec *refs;
	size_t line;       // current line in the comment
	bool member;       // a member doc /**< ... */
	bool file;         // a file comment (has @file)
	int last_rank;     // for DC7
	sec_kind last_ranked;
	bool order_warned;
	bool member_warned;
} pstate;


static void diag(pstate *st, const char *rule, size_t line, const char *fmt, ...)
{
	char buf[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	cddiag d = {.rule = rule, .line = line, .msg = cstr_clone(buf)};
	vec_push(st->diags, &d);
}


static inline char last_char(strbuf *sb)
{
	size_t l = strbuf_len(sb);
	return l ? strbuf_get(sb, l - 1) : '\0';
}


static void add_ref(pstate *st, cdref_kind kind, const char *target, size_t len,
                    size_t line)
{
	cdref r = {.kind = kind, .target = cstr_clone_len(target, len), .line = line};
	vec_push(st->refs, &r);
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


// DC7: sections in the prescribed order
static void check_order(pstate *st, sec_kind k)
{
	int r = sec_rank(k);
	if (r < 0) {
		return;
	}
	if (r < st->last_rank && !st->order_warned) {
		diag(st, "DC7", st->line, "%s after %s; the order is @brief, details, "
		     "@param, @return, @warning, @note, @deprecated, @see",
		     sec_label(k), sec_label(st->last_ranked));
		st->order_warned = true;
	}
	if (r > st->last_rank) {
		st->last_rank = r;
		st->last_ranked = k;
	}
}


static void open_section(pstate *st, sec_kind kind)
{
	if (st->member && !st->member_warned) {
		diag(st, "DC2", st->line, "member docs have no block commands");
		st->member_warned = true;
	}
	if (kind == SEC_PAR) {
		// a titled paragraph of the details
		back_to_details(st);
		para_break(st->details);
		st->cur_kind = SEC_PAR;
		return;
	}
	check_order(st, kind);
	section s = {.kind = kind, .text = strbuf_new(), .line = st->line};
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
			diag(st, "DC6", st->line, "@par is not allowed; did you mean @param or @p? "
			     "(@par makes a paragraph titled \"%.*s\")", (int)(len > 40 ? 40 : len), text);
		} else {
			diag(st, "DC6", st->line,
			     "@par is not allowed; use a Markdown heading or **bold** text");
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
	if (st->cur_kind == SEC_DETAILS) {
		check_order(st, SEC_DETAILS);
	}
	append_line(st->cur, text, len);
}


static const char *HTML_TAGS[] = {
	"tt", "b", "i", "em", "strong", "code", "br", "p", "ul", "ol", "li", "pre",
	"sup", "sub", "a", "table", "tr", "td", "th", "div", "span", "hr", "u",
};


// Checks a line of text (not code) for DC9, DC11 and DC12 problems
static void scan_line(pstate *st, const char *line)
{
	size_t len = strlen(line);
	const char *t = line + indent_of(line);
	if (!st->file && t[0] == '#') {
		size_t h = strspn(t, "#");
		if (t[h] == ' ') {
			diag(st, "DC12", st->line, "Markdown heading outside a file comment");
		}
	}
	bool in_span = false;
	for (size_t i = 0; i < len; i++) {
		char c = line[i];
		if (c == '`') {
			in_span = !in_span;
			continue;
		}
		if (in_span) {
			continue;
		}
		if (c == ':' && line[i + 1] == ':' && (isalpha((unsigned char)line[i + 2])
		                                       || line[i + 2] == '_')) {
			size_t e = i + 2;
			while (is_ident_char(line[e])) e++;
			size_t b = i; // type::member
			while (b > 0 && is_ident_char(line[b - 1])) b--;
			if (b < i) {
				diag(st, "DC11", st->line, "%.*s: write #%.*s.%.*s", (int)(e - b), line + b,
				     (int)(i - b), line + b, (int)(e - i - 2), line + i + 2);
			} else {
				diag(st, "DC11", st->line, "::%.*s: write #%.*s", (int)(e - i - 2), line + i + 2,
				     (int)(e - i - 2), line + i + 2);
			}
			i = e - 1;
		} else if ((isalpha((unsigned char)c) || c == '_')
		           && (i == 0 || (!is_ident_char(line[i - 1]) && line[i - 1] != '#'
		                          && line[i - 1] != '@' && line[i - 1] != ':'))) {
			size_t e = i;
			while (is_ident_char(line[e])) e++;
			if (line[e] == '(' && line[e + 1] == ')') {
				diag(st, "DC11", st->line, "%.*s(): write #%.*s", (int)(e - i), line + i,
				     (int)(e - i), line + i);
			}
			i = e - 1;
		} else if (c == '<') {
			size_t k = i + 1 + (line[i + 1] == '/');
			size_t e = k;
			while (isalpha((unsigned char)line[e])) e++;
			if (e > k && (line[e] == '>' || line[e] == ' ' || (line[e] == '/' && line[e + 1] == '>'))) {
				for (size_t h = 0; h < sizeof(HTML_TAGS) / sizeof(HTML_TAGS[0]); h++) {
					if (strlen(HTML_TAGS[h]) == e - k && strncasecmp(HTML_TAGS[h], line + k, e - k) == 0) {
						diag(st, "DC12", st->line, "HTML tag <%.*s>: use Markdown",
						     (int)(e - i - 1), line + i + 1);
						break;
					}
				}
			}
		} else if (c == '#' && (isalpha((unsigned char)line[i + 1]) || line[i + 1] == '_')
		           && (i == 0 || (!is_ident_char(line[i - 1]) && line[i - 1] != '&'))) {
			// #name, #type.member
			size_t b = i + 1, e = b;
			while (is_ident_char(line[e])) e++;
			if (line[e] == '.' && (isalpha((unsigned char)line[e + 1]) || line[e + 1] == '_')) {
				size_t f = e + 1;
				while (is_ident_char(line[f])) f++;
				if (f - e - 1 == 1 && line[e + 1] == 'h') {
					diag(st, "DC11", st->line, "#%.*s: write %.*s (headers take no #)",
					     (int)(f - b), line + b, (int)(f - b), line + b);
				}
				e = f;
			}
			add_ref(st, CDR_SYMBOL, line + b, e - b, st->line);
			i = e - 1;
		} else if (c == '@' && line[i + 1] == 'p' && isspace((unsigned char)line[i + 2])
		           && (i == 0 || isspace((unsigned char)line[i - 1]))) {
			// @p name
			size_t b = i + 2;
			while (line[b] == ' ' || line[b] == '\t') b++;
			size_t e = b;
			while (is_ident_char(line[e])) e++;
			if (e > b) {
				add_ref(st, CDR_PARAM, line + b, e - b, st->line);
			}
			i = e > b ? e - 1 : i;
		} else if (c == '@' && starts_with_word(line + i + 1, "move")
		           && (i == 0 || isspace((unsigned char)line[i - 1]))) {
			// must follow "@param" or "@return" ("@param NAME @move" is
			// reported when the @param is parsed)
			size_t p = i;
			while (p > 0 && isspace((unsigned char)line[p - 1])) p--;
			size_t w = p;
			while (w > 0 && !isspace((unsigned char)line[w - 1])) w--;
			size_t w2 = w;
			while (w2 > 0 && isspace((unsigned char)line[w2 - 1])) w2--;
			size_t w1 = w2;
			while (w1 > 0 && !isspace((unsigned char)line[w1 - 1])) w1--;
			bool after_cmd = (p - w == 6 && strncmp(line + w, "@param", 6) == 0)
			                 || (p - w == 7 && strncmp(line + w, "@return", 7) == 0)
			                 || (p - w == 8 && strncmp(line + w, "@returns", 8) == 0);
			bool after_name = w2 - w1 == 6 && strncmp(line + w1, "@param", 6) == 0;
			if (!after_cmd && !after_name) {
				diag(st, "DC9", st->line, "@move must come right after @param or @return");
			}
		}
	}
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
		const cmd_def *cmd = find_block_cmd(line + w, wend - w);
		if (!cmd) {
			if (c == '@' && !(wend - w == 1 && line[w] == 'p')
			        && !(wend - w == 4 && strncmp(line + w, "move", 4) == 0)) {
				if (word_in(line + w, wend - w, INLINE_CMDS, sizeof(INLINE_CMDS) / sizeof(INLINE_CMDS[0]))) {
					diag(st, "DC6", st->line, "@%.*s is not allowed; use Markdown "
					     "(`code`, *emphasis*, **bold**) or #name", (int)(wend - w), line + w);
				} else {
					diag(st, "DC6", st->line, "unknown command @%.*s (did you mean @p %.*s?)",
					     (int)(wend - w), line + w, (int)(wend - w), line + w);
				}
			}
			continue;
		}
		if (c == '\\') {
			diag(st, "DC6", st->line, "\\%s: write @%s", cmd->name, cmd->name);
		} else if (cmd->use && cmd->kind != SEC_PAR) {
			diag(st, "DC6", st->line, "@%s is not allowed; use %s", cmd->name, cmd->use);
		}
		add_text(st, line + seg, i - seg, seg_first);
		open_section(st, cmd->kind);
		seg = wend;
		seg_first = true;
		i = wend - 1;
	}
	add_text(st, line + seg, len - seg, seg_first);
}


static void parse_lines(pstate *st, const vec *lines, size_t first)
{
	for (size_t i = 0, n = vec_len(lines); i < n; i++) {
		const char *line = vec_get_rawptr(lines, i);
		const char *t = line + indent_of(line);
		st->line = first + i;

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
			diag(st, "DC12", st->line, "@code is not allowed; use a ```c fence");
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
			diag(st, "DC12", st->line, "@verbatim is not allowed; use a ``` fence");
			st->fence = FENCE_VERBATIM;
			append_line(st->cur, "```", 3);
		} else if (*t == '\0') {
			// a blank line ends a block command
			back_to_details(st);
			para_break(st->details);
		} else {
			scan_line(st, line);
			parse_line(st, line);
		}
	}
	if (st->fence != FENCE_NONE) {
		diag(st, "DC12", st->line, "unterminated code block");
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
	static const char *ABBREVS[] = {"e.g", "i.e", "etc", "a.k.a", "vs", "cf", "s.t"};
	for (size_t i = 0; i < sizeof(ABBREVS) / sizeof(ABBREVS[0]); i++) {
		size_t n = strlen(ABBREVS[i]);
		if (dot >= n && strncasecmp(text + dot - n, ABBREVS[i], n) == 0
		        && (dot == n || !isalnum((unsigned char)text[dot - n - 1]))) {
			return true;
		}
	}
	return false;
}


// End (exclusive) of the first sentence of s[0:len], or len
static size_t sentence_end(const char *s, size_t len)
{
	bool in_span = false;
	for (size_t i = 0; i < len; i++) {
		if (s[i] == '`') {
			in_span = !in_span;
		} else if (s[i] == '.' && !in_span && (i + 1 == len || isspace((unsigned char)s[i + 1]))
		           && !is_abbrev_end(s, i)) {
			return i + 1;
		}
	}
	return len;
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


// The first sentence of the first paragraph of the details becomes the
// brief, and is removed from the details.
static void auto_brief(char **brief, char **details)
{
	const char *d = *details;
	const char *pe = strstr(d, "\n\n");
	size_t par_end = pe ? (size_t)(pe - d) : strlen(d);
	if (par_end == 0 || !starts_plain_text(d)) {
		return;
	}
	size_t end = sentence_end(d, par_end);
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


typedef enum {
	LEGACY_NONE = 0,
	LEGACY_BORROW, // (**no transfer**), ...
	LEGACY_MOVE    // (**move**), (**transfer**), ...
} legacy_own;


// Recognises and removes a legacy ownership annotation at the start of
// *text, e.g. "(**move**)". The annotation is copied to found.
static legacy_own strip_legacy_ownership(char **text, char *found, size_t found_sz)
{
	const char *s = *text;
	if (*s != '(') {
		return LEGACY_NONE;
	}
	const char *p = s + 1;
	while (*p == ' ') p++;
	size_t stars = strspn(p, "*");
	if (stars > 2) {
		return LEGACY_NONE;
	}
	const char *w = p + stars;
	size_t wlen = strcspn(w, "*)");
	const char *q = w + wlen;
	if (strspn(q, "*") != stars) {
		return LEGACY_NONE;
	}
	q += stars;
	while (*q == ' ') q++;
	if (*q != ')') {
		return LEGACY_NONE;
	}
	legacy_own own = LEGACY_NONE;
	if (wlen == 11 && strncasecmp(w, "no transfer", 11) == 0) {
		own = LEGACY_BORROW;
	} else if ((wlen == 8 && strncasecmp(w, "transfer", 8) == 0)
	           || (wlen == 13 && strncasecmp(w, "full transfer", 13) == 0)
	           || (wlen == 4 && strncasecmp(w, "move", 4) == 0)) {
		own = LEGACY_MOVE;
	}
	if (own != LEGACY_NONE) {
		snprintf(found, found_sz, "%.*s", (int)(q + 1 - s), s);
		char *rest = trimmed_copy(q + 1);
		FREE(*text);
		*text = rest;
	}
	return own;
}


static void check_legacy(pstate *st, size_t line, char **text, bool *move,
                         const char *cmd, const char *name)
{
	char found[64];
	legacy_own own = strip_legacy_ownership(text, found, sizeof(found));
	if (own == LEGACY_MOVE) {
		*move = true;
		diag(st, "DC9", line, "write %s @move%s%s instead of %s", cmd,
		     name ? " " : "", name ? name : "", found);
	} else if (own == LEGACY_BORROW) {
		diag(st, "DC9", line, "%s: not moving is the default; remove it", found);
	}
}


static cdparam parse_param(pstate *st, const char *text, size_t line)
{
	cdparam prm = {.name = NULL, .move = false, .desc = NULL, .line = line};
	const char *s = text;
	if (starts_with_word(s, "@move")) {
		prm.move = true;
		s += 5;
		while (isspace((unsigned char)*s)) s++;
	}
	if (*s == '(') { // legacy annotation before the name: @param (move) x
		char *rest = cstr_clone(s);
		char found[64];
		legacy_own own = strip_legacy_ownership(&rest, found, sizeof(found));
		if (own != LEGACY_NONE) {
			size_t nl = strcspn(rest, " \t\n");
			if (own == LEGACY_MOVE) {
				prm.move = true;
				diag(st, "DC9", line, "write @param @move %.*s instead of %s", (int)nl, rest, found);
			} else {
				diag(st, "DC9", line, "%s: not moving is the default; remove it", found);
			}
			s = text + (strlen(text) - strlen(rest));
		}
		FREE(rest);
	}
	if (*s == '[') { // direction, e.g. [in,out]
		diag(st, "DC8", line, "direction annotations like %.*s are not used",
		     (int)(strcspn(s, "]") + 1), s);
		s += strcspn(s, "]");
		if (*s) s++;
		while (isspace((unsigned char)*s)) s++;
	}
	size_t nlen = strcspn(s, " \t\n");
	size_t clean = nlen;
	while (clean > 0 && (s[clean - 1] == ',' || s[clean - 1] == ':')) {
		clean--;
	}
	if (clean < nlen) {
		diag(st, "DC8", line, "punctuation after the parameter name in \"@param %.*s\"",
		     (int)nlen, s);
	}
	prm.name = cstr_clone_len(s, clean);
	if (clean == 0) {
		diag(st, "DC8", line, "@param without a name");
	}
	s += nlen;
	prm.desc = trimmed_copy(s);
	if (starts_with_word(prm.desc, "@move")) {
		diag(st, "DC9", line, "@move goes before the parameter name: @param @move %s",
		     prm.name);
		prm.move = true;
		char *rest = trimmed_copy(prm.desc + 5);
		FREE(prm.desc);
		prm.desc = rest;
	}
	check_legacy(st, line, &prm.desc, &prm.move, "@param", prm.name);
	return prm;
}


// DC10: @see lists symbol and file names, separated by commas
static void check_see(pstate *st, const char *text, size_t line)
{
	const char *s = text;
	while (*s) {
		size_t n = strcspn(s, ",");
		size_t b = 0, e = n;
		while (b < e && isspace((unsigned char)s[b])) b++;
		while (e > b && isspace((unsigned char)s[e - 1])) e--;
		bool ok = e > b;
		for (size_t i = b; i < e && ok; i++) {
			ok = is_ident_char(s[i]) || s[i] == '.' || (i == b && s[i] == '#');
		}
		if (!ok) {
			diag(st, "DC10", line, "@see lists only symbol and file names, "
			     "separated by commas (found \"%.*s\")", (int)((e - b) > 40 ? 40 : (e - b)), s + b);
			return;
		}
		size_t hb = (s[b] == '#') ? b + 1 : b;
		while (e > hb && s[e - 1] == '.') e--; // end of sentence
		add_ref(st, CDR_SEE, s + hb, e - hb, line);
		s += n + (s[n] == ',');
	}
}


// DC1 (block form) and DC2 (single-line member docs), on the raw comment
static void check_form(pstate *st, const char *raw, size_t len)
{
	const char *nl = memchr(raw, '\n', len);
	size_t last_line = 0;
	for (size_t i = 0; i < len; i++) {
		last_line += (raw[i] == '\n');
	}
	if (st->member) {
		if (nl) {
			diag(st, "DC2", 0, "member docs are a single line");
		}
		if (len >= 3 && strncmp(raw + len - 3, "**/", 3) == 0) {
			diag(st, "DC1", last_line, "close with */, not **/");
		}
		return;
	}
	if (!nl) {
		diag(st, "DC1", 0, "one-line doc comment; put /** and */ on their own lines");
		return;
	}
	for (const char *p = raw + 3; p < nl; p++) {
		if (!isspace((unsigned char)*p)) {
			diag(st, "DC1", 0, "text on the opening /** line");
			break;
		}
	}
	const char *last = raw + len;
	while (last > raw && last[-1] != '\n') last--;
	size_t ll = raw + len - last;
	bool double_star = len >= 3 && strncmp(raw + len - 3, "**/", 3) == 0;
	if (double_star) {
		diag(st, "DC1", last_line, "close with */, not **/");
	}
	// text before the closing */ on its line
	const char *p = last;
	while (p < raw + len && isspace((unsigned char)*p)) p++;
	if (p < raw + len && *p == '*' && p + 1 < raw + len && p[1] != '/') {
		p++; // star column
	}
	while (p < raw + len && isspace((unsigned char)*p)) p++;
	if (ll > 0 && p < raw + len - 2 - double_star) {
		diag(st, "DC1", last_line, "text on the closing */ line");
	}
}


// AI involvement levels (DC15) and their titles
static const char *AI_LEVELS[][2] = {
	{"human", "Human"},
	{"ai-informed", "AI-informed"},
	{"ai-assisted", "AI-assisted"},
	{"ai-generated", "AI-generated, human-directed"},
	{"ai-autonomous", "AI-autonomous"},
};


bool cddoc_hidden(const char *raw)
{
	if (!raw) {
		return false;
	}
	cddoc *doc = cddoc_parse(raw, strlen(raw));
	bool ret = doc->hide;
	cddoc_free(doc);
	return ret;
}


const char *cdai_level_title(const char *level)
{
	for (size_t i = 0; i < sizeof(AI_LEVELS) / sizeof(AI_LEVELS[0]); i++) {
		if (strcmp(AI_LEVELS[i][0], level) == 0) {
			return AI_LEVELS[i][1];
		}
	}
	return NULL;
}


// DC15: @ai level, agent
static cdai parse_ai(pstate *st, const char *text, size_t line)
{
	size_t n = strcspn(text, ",");
	char *lv = cstr_clone_len(text, n);
	cdai ai = {.level = trimmed_copy(lv), .agent = trimmed_copy(text[n] ? text + n + 1 : "")};
	FREE(lv);
	if (!st->file) {
		diag(st, "DC15", line, "@ai is only used in file comments");
	}
	if (!cdai_level_title(ai.level)) {
		diag(st, "DC15", line, "unknown AI level \"%s\" (one of human, ai-informed, "
		     "ai-assisted, ai-generated, ai-autonomous)", ai.level);
	} else if (ai.agent[0] == '\0' && strcmp(ai.level, "human") != 0) {
		diag(st, "DC15", line, "@ai %s without the AI used, e.g. @ai %s, Claude (Anthropic)",
		     ai.level, ai.level);
	}
	return ai;
}


static void cdai_finalise(void *ptr, const finaliser *fnr)
{
	cdai *a = (cdai *)ptr;
	FREE(a->level);
	FREE(a->agent);
}


static void cdparam_finalise(void *ptr, const finaliser *fnr)
{
	cdparam *p = (cdparam *)ptr;
	FREE(p->name);
	FREE(p->desc);
}


static void cddiag_finalise(void *ptr, const finaliser *fnr)
{
	FREE(((cddiag *)ptr)->msg);
}


static void cdref_finalise(void *ptr, const finaliser *fnr)
{
	FREE(((cdref *)ptr)->target);
}


static vec *new_str_vec()
{
	return vec_new(sizeof(char *));
}


static void free_str_vec(vec *v)
{
	DESTROY(v, finaliser_cons(FNR(vec), finaliser_new_ptr()));
}


static bool has_file_cmd(const char *raw, size_t len)
{
	for (size_t i = 0; i + 5 <= len; i++) {
		if (raw[i] == '@' && strncmp(raw + i + 1, "file", 4) == 0
		        && (i + 5 == len || !isalnum((unsigned char)raw[i + 5]))
		        && (i == 0 || isspace((unsigned char)raw[i - 1]))) {
			return true;
		}
	}
	return false;
}


cddoc *cddoc_parse(const char *raw, size_t len)
{
	size_t first;
	vec *lines = strip_lines(raw, len, &first);

	pstate st = {
		.details = strbuf_new(),
		.secs = vec_new(sizeof(section)),
		.fence = FENCE_NONE,
		.diags = vec_new(sizeof(cddiag)),
		.refs = vec_new(sizeof(cdref)),
		.member = len >= 4 && strncmp(raw, "/**<", 4) == 0,
		.file = has_file_cmd(raw, len),
		.last_rank = -1,
		.last_ranked = SEC_BRIEF,
	};
	back_to_details(&st);
	check_form(&st, raw, len);
	parse_lines(&st, lines, first);
	free_str_vec(lines);

	cddoc *doc = NEW(cddoc);
	doc->params = vec_new(sizeof(cdparam));
	doc->ret = NULL;
	doc->ret_move = false;
	doc->see = new_str_vec();
	doc->warnings = new_str_vec();
	doc->notes = new_str_vec();
	doc->deprecated = NULL;
	doc->authors = new_str_vec();
	doc->ai = vec_new(sizeof(cdai));
	doc->hide = false;

	strbuf *brief = strbuf_new();
	bool brief_cmd = false;
	size_t brief_line = 0;
	strbuf *ret = NULL;
	for (size_t i = 0, n = vec_len(st.secs); i < n; i++) {
		section *s = vec_get_mut(st.secs, i);
		char *text = trimmed_copy(strbuf_as_str(s->text));
		strbuf_free(s->text);
		switch (s->kind) {
		case SEC_BRIEF:
			if (text[0] == '\0') {
				diag(&st, "DC5", s->line, "empty @brief");
			}
			if (!brief_cmd) {
				brief_line = s->line;
			}
			brief_cmd = true;
			if (strbuf_len(brief) > 0 && text[0]) {
				strbuf_append_char(brief, ' ');
			}
			unwrap(brief, text);
			FREE(text);
			break;
		case SEC_PARAM: {
			cdparam p = parse_param(&st, text, s->line);
			vec_push(doc->params, &p);
			FREE(text);
			break;
		}
		case SEC_RETURN:
			if (starts_with_word(text, "@move")) {
				doc->ret_move = true;
				char *rest = trimmed_copy(text + 5);
				FREE(text);
				text = rest;
			}
			check_legacy(&st, s->line, &text, &doc->ret_move, "@return", NULL);
			if (text[0] == '\0') {
				diag(&st, "DC8", s->line, "empty @return");
			}
			if (!ret) {
				ret = strbuf_new();
			} else {
				strbuf_append(ret, "\n\n");
			}
			strbuf_append(ret, text);
			FREE(text);
			break;
		case SEC_SEE:
			check_see(&st, text, s->line);
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
		case SEC_AI: {
			cdai ai = parse_ai(&st, text, s->line);
			vec_push(doc->ai, &ai);
			FREE(text);
			break;
		}
		case SEC_HIDE:
			doc->hide = true;
			FREE(text);
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

	// DC5: brief
	if (doc->brief[0] == '\0') {
		auto_brief(&doc->brief, &doc->details);
		if (!st.member) {
			if (doc->brief[0] == '\0') {
				if (!brief_cmd) {
					diag(&st, "DC5", 0, "no brief description");
				}
			} else if (!brief_cmd) {
				diag(&st, "DC5", first, "no @brief (the first sentence is used)");
			}
		}
	} else {
		size_t bl = strlen(doc->brief);
		size_t se = sentence_end(doc->brief, bl);
		if (se < bl) {
			diag(&st, "DC5", brief_line, "the brief has more than one sentence; "
			     "move the rest to the details");
		}
	}
	doc->diags = st.diags;
	doc->refs = st.refs;
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
	DESTROY(self->ai, finaliser_cons(FNR(vec), finaliser_new(cdai_finalise)));
	DESTROY(self->diags, finaliser_cons(FNR(vec), finaliser_new(cddiag_finalise)));
	DESTROY(self->refs, finaliser_cons(FNR(vec), finaliser_new(cdref_finalise)));
	FREE(self);
}
