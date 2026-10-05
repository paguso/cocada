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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cdconfig.h"
#include "cddecl.h"
#include "cddoc.h"
#include "cdfile.h"
#include "cdlexer.h"
#include "cdlint.h"
#include "cdmd.h"
#include "cdsym.h"
#include "cdversion.h"
#include "cli.h"
#include "cstrutil.h"
#include "new.h"
#include "strbuf.h"


cliparser *create_cli_parser()
{
	cliparser *clip = cliparser_new("cocadoc", "COCADA source code documentation. "
	                                "Without files, documents the headers listed by the "
	                                "configuration file (cocadoc.config)");
	cliparser_add_option(clip, cliopt_new_sc_defaults('v', "version",
	                     "Prints the version of cocadoc"));
	cliparser_add_option(clip, cliopt_new('c', "config",
	                                      "Configuration file (default: ./cocadoc.config)",
	                                      OPT_OPTIONAL, OPT_SINGLE, ARG_FILE, 1, 1, NULL, NULL));
	cliparser_add_option(clip, cliopt_new_defaults('t', "tokens",
	                     "Dump the token stream of each input file (debug)"));
	cliparser_add_option(clip, cliopt_new_defaults('d', "decls",
	                     "Dump the documented declarations of each input file (debug)"));
	cliparser_add_option(clip, cliopt_new('o', "output",
	                                      "Write Markdown documentation pages to this directory",
	                                      OPT_OPTIONAL, OPT_SINGLE, ARG_DIR, 1, 1, NULL, NULL));
	cliparser_add_option(clip, cliopt_new_defaults('l', "lint",
	                     "Check the documentation comments against the COCADA style"));
	cliparser_add_option(clip, cliopt_new_defaults('s', "symbols",
	                     "Dump the symbol table of all input files (debug)"));
	cliparser_add_option(clip, cliopt_new_defaults('D', "docs",
	                     "Dump the parsed documentation of each declaration (debug)"));
	cliparser_add_pos_arg(clip, cliarg_new_multi_optional("files",
	                      "C headers (instead of those listed by the configuration)",
	                      ARG_FILE));
	return clip;
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


// Prints a multi-line field indented, one output line per text line
static void print_field(const char *label, const char *text)
{
	printf("\t\t%-10s", label);
	for (const char *s = text; *s; s++) {
		putchar(*s);
		if (*s == '\n') {
			printf("\t\t%-10s", "");
		}
	}
	putchar('\n');
}


static void print_str_vec(const char *label, const vec *v)
{
	for (size_t i = 0, n = vec_len(v); i < n; i++) {
		print_field(label, vec_get_rawptr(v, i));
	}
}


static void print_doc(const char *path, const cddecl *d, bool member)
{
	if (!d->doc) {
		return;
	}
	cddoc *doc = cddoc_parse(d->doc, strlen(d->doc));
	if (member) {
		printf("\t. %s\n", d->name);
	} else {
		printf("%s:%zu\t%s %s\n", path, d->line, cddecl_kind_name(d->kind), d->name);
	}
	print_field("brief:", doc->brief);
	for (size_t i = 0, n = vec_len(doc->params); i < n; i++) {
		const cdparam *p = vec_get(doc->params, i);
		char label[64];
		snprintf(label, sizeof(label), "param %s%s:", p->move ? "@move " : "", p->name);
		printf("\t\t%s\n", label);
		print_field("", p->desc);
	}
	if (doc->ret) {
		print_field(doc->ret_move ? "return @move:" : "return:", doc->ret);
	}
	print_str_vec("see:", doc->see);
	print_str_vec("warning:", doc->warnings);
	print_str_vec("note:", doc->notes);
	if (doc->deprecated) {
		print_field("deprecated:", doc->deprecated);
	}
	print_str_vec("author:", doc->authors);
	if (doc->details[0]) {
		print_field("details:", doc->details);
	}
	for (size_t i = 0, n = vec_len(doc->diags); i < n; i++) {
		const cddiag *dg = vec_get(doc->diags, i);
		printf("\t\t[%s] line %zu: %s\n", dg->rule, d->doc_line + dg->line, dg->msg);
	}
	cddoc_free(doc);
}


static void dump_docs(const char *path, const char *src, size_t len)
{
	vec *toks = cdlex_all(src, len);
	vec *decls = cddecl_match(src, toks);
	for (size_t i = 0, n = vec_len(decls); i < n; i++) {
		const cddecl *d = vec_get(decls, i);
		print_doc(path, d, false);
		for (size_t j = 0, m = d->members ? vec_len(d->members) : 0; j < m; j++) {
			print_doc(path, vec_get(d->members, j), true);
		}
	}
	cddecl_vec_free(decls);
	DESTROY_FLAT(toks, vec);
}


#define NRULES 15

// Prints the style warnings of a file. Counts them per rule in counts.
static size_t lint_file(const cdfile *f, const cdsymtab *tab, size_t counts[NRULES])
{
	const char *path = f->path;
	vec *warns = cdlint(f, tab);
	size_t n = vec_len(warns);
	for (size_t i = 0; i < n; i++) {
		const cdwarn *w = vec_get(warns, i);
		printf("%s:%zu: warning: [%s] %s: %s\n", path, w->line, w->rule, w->name, w->msg);
		int r = atoi(w->rule + 2);
		if (r > 0 && r < NRULES) {
			counts[r]++;
		}
	}
	cdwarn_vec_free(warns);
	return n;
}


static void dump_symbols(const cdsymtab *tab)
{
	for (size_t i = 0, n = cdsymtab_size(tab); i < n; i++) {
		const cdsym *s = cdsymtab_get(tab, i);
		size_t ncands;
		cdsymtab_resolve(tab, s->name, NULL, &ncands);
		printf("%-40s %-9s %s:%zu%s%s\n", s->name, cddecl_kind_name(s->kind), s->file->path,
		       s->decl ? s->decl->line : 1, (s->decl && s->decl->doc) ? "" : "  (undocumented)",
		       ncands > 1 ? "  (ambiguous)" : "");
	}
}


// Creates a directory and its parents, if needed. Returns false on error.
static bool make_dirs(const char *path)
{
	char *p = cstr_clone(path);
	bool ok = true;
	for (char *c = p + 1; ok; c++) {
		if (*c == '/' || *c == '\0') {
			char saved = *c;
			*c = '\0';
			ok = mkdir(p, 0755) == 0 || errno == EEXIST;
			*c = saved;
			if (saved == '\0') {
				break;
			}
		}
	}
	FREE(p);
	return ok;
}


static bool write_file(const char *dir, const char *name, const char *text)
{
	strbuf *path = strbuf_new();
	strbuf_append(path, dir);
	strbuf_append_char(path, '/');
	strbuf_append(path, name);
	FILE *f = fopen(strbuf_as_str(path), "w");
	bool ok = f != NULL;
	if (f) {
		ok = fputs(text, f) >= 0 && fputc('\n', f) != EOF;
		ok = (fclose(f) == 0) && ok;
	}
	if (!ok) {
		fprintf(stderr, "cocadoc: cannot write %s\n", strbuf_as_str(path));
	}
	strbuf_free(path);
	return ok;
}


// Writes the pages of all files and the index to dir
static bool write_docs(const char *dir, const vec *files, const cdsymtab *tab,
                       const char *title)
{
	if (!make_dirs(dir)) {
		fprintf(stderr, "cocadoc: cannot create directory %s\n", dir);
		return false;
	}
	bool ok = true;
	for (size_t i = 0, n = vec_len(files); i < n; i++) {
		const cdfile *f = vec_get_rawptr(files, i);
		char *name = cdmd_page_name(f);
		char *page = cdmd_page(f, tab);
		ok = write_file(dir, name, page) && ok;
		FREE(page);
		FREE(name);
	}
	char *index = cdmd_index(files, title);
	ok = write_file(dir, "index.md", index) && ok;
	FREE(index);
	fprintf(stderr, "cocadoc: wrote %zu pages and index.md to %s\n", vec_len(files), dir);
	return ok;
}


static const char *opt_value(const cliparser *clip, char shortname)
{
	if (!cliparser_opt_used_from_shortname(clip, shortname)) {
		return NULL;
	}
	return vec_get_rawptr(cliparser_opt_val_from_shortname(clip, shortname), 0);
}


static bool file_exists(const char *path)
{
	struct stat st;
	return stat(path, &st) == 0;
}


// "PROJECT VERSION API Reference", without the parts that are not set
static char *index_title(const cdconfig *cfg)
{
	strbuf *sb = strbuf_new();
	if (cfg && cfg->project_name) {
		strbuf_append(sb, cfg->project_name);
		strbuf_append_char(sb, ' ');
	}
	if (cfg && cfg->version) {
		strbuf_append(sb, cfg->version);
		strbuf_append_char(sb, ' ');
	}
	strbuf_append(sb, "API Reference");
	return strbuf_detach(sb);
}


int main(int argc, char **argv)
{
	cliparser *clip = create_cli_parser();
	cliparser_parse(clip, argc, argv, true);
	const cliopt *sc = cliparser_active_sc_option(clip);
	if (sc) {
		if (cliopt_shortname(sc) == 'v') {
			printf("cocadoc %s\n", cocadoc_version_str());
		}
		DESTROY_FLAT(clip, cliparser);
		return EXIT_SUCCESS; // (-h has printed the help)
	}

	bool tokens = cliparser_opt_used_from_shortname(clip, 't');
	bool decls = cliparser_opt_used_from_shortname(clip, 'd');
	bool docs = cliparser_opt_used_from_shortname(clip, 'D');
	bool lint = cliparser_opt_used_from_shortname(clip, 'l');
	bool symbols = cliparser_opt_used_from_shortname(clip, 's');
	bool debug = tokens || decls || docs || lint || symbols;
	size_t nwarns = 0, counts[NRULES] = {0};
	const vec *args = cliparser_arg_val_from_pos(clip, 0);
	bool explicit_files = args && vec_len(args) > 0;
	int ret = EXIT_SUCCESS;

	// configuration: -c FILE, or ./cocadoc.config if there is one
	const char *cfg_path = opt_value(clip, 'c');
	if (!cfg_path && file_exists("cocadoc.config")) {
		cfg_path = "cocadoc.config";
	}
	cdconfig *cfg = NULL;
	if (cfg_path) {
		char err[512];
		cfg = cdconfig_load(cfg_path, err, sizeof(err));
		if (!cfg) {
			fprintf(stderr, "cocadoc: %s\n", err);
			DESTROY_FLAT(clip, cliparser);
			return EXIT_FAILURE;
		}
	} else if (!explicit_files) {
		fprintf(stderr, "cocadoc: no input files and no cocadoc.config "
		        "(see cocadoc -h)\n");
		DESTROY_FLAT(clip, cliparser);
		return EXIT_FAILURE;
	}

	// input: the files given, or those listed by the configuration
	vec *paths = vec_new(sizeof(char *));
	if (explicit_files) {
		for (size_t i = 0, n = vec_len(args); i < n; i++) {
			vec_push_rawptr(paths, cstr_clone(vec_get_rawptr(args, i)));
		}
	} else {
		vec *found = cdconfig_find_headers(cfg);
		vec_cat(paths, found);
		DESTROY_FLAT(found, vec);
		if (vec_len(paths) == 0) {
			fprintf(stderr, "cocadoc: no headers found in the modules of %s\n", cfg_path);
			ret = EXIT_FAILURE;
		}
	}

	// load all files: references may point to any of them
	vec *loaded = vec_new(sizeof(cdfile *));
	for (size_t i = 0, n = vec_len(paths); i < n; i++) {
		const char *path = vec_get_rawptr(paths, i);
		cdfile *f = cdfile_load(path);
		if (!f) {
			fprintf(stderr, "cocadoc: cannot read %s\n", path);
			ret = EXIT_FAILURE;
			continue;
		}
		vec_push_rawptr(loaded, f);
	}

	for (size_t i = 0, n = vec_len(loaded); i < n; i++) {
		const cdfile *f = vec_get_rawptr(loaded, i);
		if (tokens) {
			dump_tokens(f->path, f->src, f->len);
		}
		if (decls) {
			dump_decls(f->path, f->src, f->len);
		}
		if (docs) {
			dump_docs(f->path, f->src, f->len);
		}
	}

	// pages: with -o, or by default when documenting the configuration
	const char *outdir = opt_value(clip, 'o');
	if (!outdir && !explicit_files && !debug) {
		outdir = cfg->output_dir;
		if (!outdir) {
			fprintf(stderr, "cocadoc: OUTPUT_DIRECTORY is not set in %s\n", cfg_path);
			ret = EXIT_FAILURE;
		}
	}
	cdsymtab *tab = (lint || symbols || outdir) ? cdsymtab_new(loaded) : NULL;
	if (outdir && vec_len(loaded) > 0) {
		char *title = index_title(cfg);
		if (!write_docs(outdir, loaded, tab, title)) {
			ret = EXIT_FAILURE;
		}
		FREE(title);
	}
	if (symbols) {
		dump_symbols(tab);
	}
	if (lint) {
		for (size_t i = 0, n = vec_len(loaded); i < n; i++) {
			nwarns += lint_file(vec_get_rawptr(loaded, i), tab, counts);
		}
	}
	cdsymtab_free(tab);

	if (lint) {
		fprintf(stderr, "%zu warning%s", nwarns, nwarns == 1 ? "" : "s");
		const char *sep = " (";
		for (int r = 1; r < NRULES; r++) {
			if (counts[r]) {
				fprintf(stderr, "%sDC%d: %zu", sep, r, counts[r]);
				sep = ", ";
			}
		}
		fprintf(stderr, "%s\n", nwarns ? ")" : "");
	}

	for (size_t i = 0, n = vec_len(loaded); i < n; i++) {
		cdfile_free(vec_get_rawptr(loaded, i));
	}
	DESTROY_FLAT(loaded, vec);
	DESTROY(paths, finaliser_cons(FNR(vec), finaliser_new_ptr()));
	cdconfig_free(cfg);
	DESTROY_FLAT(clip, cliparser);
	return ret;
}
