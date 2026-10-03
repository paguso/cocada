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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "cddecl.h"
#include "cdlexer.h"
#include "cli.h"
#include "new.h"


cliparser *create_cli_parser()
{
	cliparser *clip = cliparser_new("cocadoc", "COCADA source code documentation");
	cliparser_add_option(clip, cliopt_new_defaults('t', "tokens",
	                     "Dump the token stream of each input file (debug)"));
	cliparser_add_option(clip, cliopt_new_defaults('d', "decls",
	                     "Dump the documented declarations of each input file (debug)"));
	cliparser_add_pos_arg(clip, cliarg_new_multi("files", "C source/header files",
	                      ARG_FILE));
	return clip;
}


// Whether a valueless switch option was used in the call
static bool switch_on(const cliparser *clip, char shortname)
{
	const vec *v = cliparser_opt_val_from_shortname(clip, shortname);
	return v && vec_len(v) > 0 && vec_get_bool(v, 0);
}


// Reads a whole file into a heap allocated buffer. Returns NULL on error.
static char *slurp(const char *path, size_t *len)
{
	FILE *f = fopen(path, "rb");
	if (!f) {
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = malloc(sz + 1);
	*len = fread(buf, 1, sz, f);
	buf[*len] = '\0';
	fclose(f);
	return buf;
}


static void dump_tokens(const char *path, const char *src, size_t len)
{
	vec *toks = cdlex_all(src, len);
	for (size_t i = 0, n = vec_len(toks); i < n; i++) {
		const cdtoken *tk = vec_get(toks, i);
		printf("%s:%zu\t%-8s\t", path, tk->line, cdtoken_type_name(tk->type));
		// print on a single line
		for (size_t j = tk->pos; j < tk->pos + tk->len; j++) {
			putchar(src[j] == '\n' ? ' ' : src[j]);
		}
		putchar('\n');
	}
	DESTROY_FLAT(toks, vec);
}


// First line of the brief text of a doc comment, for compact dumps
static void print_doc_head(const char *doc)
{
	if (!doc) {
		printf("-");
		return;
	}
	const char *s = doc + 3; // skip "/**"
	if (*s == '<') {
		s++;
	}
	while (*s && (isspace((unsigned char)*s) || *s == '*')) {
		s++;
	}
	if (strncmp(s, "@brief", 6) == 0) {
		s += 6;
		while (*s == ' ' || *s == '\t') {
			s++;
		}
	}
	int n = 0;
	while (s[n] && s[n] != '\n' && !(s[n] == '*' && s[n + 1] == '/') && n < 60) {
		n++;
	}
	printf("%.*s", n, s);
}


static void dump_decls(const char *path, const char *src, size_t len)
{
	vec *toks = cdlex_all(src, len);
	vec *decls = cddecl_match(src, toks);
	for (size_t i = 0, n = vec_len(decls); i < n; i++) {
		const cddecl *d = vec_get(decls, i);
		printf("%s:%zu\t%-9s %s\n\t\tsig: %s\n\t\tdoc: ", path, d->line,
		       cddecl_kind_name(d->kind), d->name, d->sig);
		print_doc_head(d->doc);
		printf("\n");
		for (size_t j = 0, m = d->members ? vec_len(d->members) : 0; j < m; j++) {
			const cddecl *mb = vec_get(d->members, j);
			printf("\t\t. %-20s | %-30s | ", mb->name, mb->sig);
			print_doc_head(mb->doc);
			printf("\n");
		}
	}
	cddecl_vec_free(decls);
	DESTROY_FLAT(toks, vec);
}


int main(int argc, char **argv)
{
	cliparser *clip = create_cli_parser();
	cliparser_parse(clip, argc, argv, true);

	bool tokens = switch_on(clip, 't');
	bool decls = switch_on(clip, 'd');
	const vec *files = cliparser_arg_val_from_pos(clip, 0);

	int ret = EXIT_SUCCESS;
	for (size_t i = 0, n = files ? vec_len(files) : 0; i < n; i++) {
		const char *path = vec_get_rawptr(files, i);
		size_t len;
		char *src = slurp(path, &len);
		if (!src) {
			fprintf(stderr, "cocadoc: cannot read %s\n", path);
			ret = EXIT_FAILURE;
			continue;
		}
		if (tokens) {
			dump_tokens(path, src, len);
		}
		if (decls) {
			dump_decls(path, src, len);
		}
		FREE(src);
	}

	DESTROY_FLAT(clip, cliparser);
	return ret;
}
