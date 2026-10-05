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
#include <dirent.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cdconfig.h"
#include "cstrutil.h"
#include "new.h"
#include "strbuf.h"


char *cdpath_join(const char *base, const char *rel)
{
	if (rel[0] == '/') {
		return cstr_clone(rel);
	}
	while (rel[0] == '.' && rel[1] == '/') {
		rel += 2;
		while (*rel == '/') rel++;
	}
	if (strcmp(rel, ".") == 0) {
		rel = "";
	}
	strbuf *sb = strbuf_new();
	if (base[0] && strcmp(base, ".") != 0 && strcmp(base, "./") != 0) {
		strbuf_append(sb, base);
		if (rel[0] && base[strlen(base) - 1] != '/') {
			strbuf_append_char(sb, '/');
		}
	}
	strbuf_append(sb, rel);
	if (strbuf_len(sb) == 0) {
		strbuf_append_char(sb, '.');
	}
	// no trailing '/'
	while (strbuf_len(sb) > 1 && strbuf_get(sb, strbuf_len(sb) - 1) == '/') {
		strbuf_cut(sb, strbuf_len(sb) - 1, 1, NULL);
	}
	return strbuf_detach(sb);
}


static char *trimmed(const char *s, size_t len)
{
	while (len > 0 && isspace((unsigned char)*s)) {
		s++;
		len--;
	}
	while (len > 0 && isspace((unsigned char)s[len - 1])) {
		len--;
	}
	return cstr_clone_len(s, len);
}


static void split_words(vec *dest, const char *s)
{
	while (*s) {
		while (isspace((unsigned char)*s)) s++;
		size_t n = 0;
		while (s[n] && !isspace((unsigned char)s[n])) n++;
		if (n > 0) {
			vec_push_rawptr(dest, cstr_clone_len(s, n));
		}
		s += n;
	}
}


static void free_str_vec(vec *v)
{
	DESTROY(v, finaliser_cons(FNR(vec), finaliser_new_ptr()));
}


// Sets *field to value, or NULL if empty (takes ownership of value)
static void set_str(char **field, char *value)
{
	FREE(*field);
	if (value[0]) {
		*field = value;
	} else {
		*field = NULL;
		FREE(value);
	}
}


cdconfig *cdconfig_load(const char *path, char *err, size_t errsz)
{
	FILE *f = fopen(path, "r");
	if (!f) {
		snprintf(err, errsz, "cannot read %s", path);
		return NULL;
	}
	// paths are relative to the directory of the configuration file
	const char *slash = strrchr(path, '/');
	char *dir = slash ? cstr_clone_len(path, slash - path) : cstr_clone(".");
	if (dir[0] == '\0') {
		FREE(dir);
		dir = cstr_clone("/");
	}

	cdconfig *cfg = NEW(cdconfig);
	cfg->project_name = NULL;
	cfg->version = NULL;
	cfg->output_type = cstr_clone("markdown");
	cfg->output_dir = NULL;
	cfg->base_dir = cstr_clone(dir);
	cfg->modules = vec_new(sizeof(char *));
	cfg->exclude_dirs = vec_new(sizeof(char *));
	cfg->exclude_files = vec_new(sizeof(char *));

	bool ok = true;
	char line[4096];
	for (size_t lineno = 1; ok && fgets(line, sizeof(line), f); lineno++) {
		char *comment = strstr(line, "//");
		if (comment) {
			*comment = '\0';
		}
		char *eq = strchr(line, '=');
		char *key = trimmed(line, eq ? (size_t)(eq - line) : strlen(line));
		if (!eq) {
			if (key[0]) {
				snprintf(err, errsz, "%s:%zu: expected KEY = value", path, lineno);
				ok = false;
			}
			FREE(key);
			continue;
		}
		char *value = trimmed(eq + 1, strlen(eq + 1));
		if (strcmp(key, "PROJECT_NAME") == 0) {
			set_str(&cfg->project_name, value);
		} else if (strcmp(key, "VERSION") == 0) {
			set_str(&cfg->version, value);
		} else if (strcmp(key, "OUTPUT_TYPE") == 0) {
			set_str(&cfg->output_type, value);
			if (!cfg->output_type) {
				cfg->output_type = cstr_clone("markdown");
			}
		} else if (strcmp(key, "OUTPUT_DIRECTORY") == 0) {
			FREE(cfg->output_dir);
			cfg->output_dir = value[0] ? cdpath_join(dir, value) : NULL;
			FREE(value);
		} else if (strcmp(key, "BASE_DIR") == 0) {
			FREE(cfg->base_dir);
			cfg->base_dir = cdpath_join(dir, value);
			FREE(value);
		} else if (strcmp(key, "MODULES") == 0) {
			split_words(cfg->modules, value);
			FREE(value);
		} else if (strcmp(key, "EXCLUDE_DIRS") == 0) {
			split_words(cfg->exclude_dirs, value);
			FREE(value);
		} else if (strcmp(key, "EXCLUDE_FILES") == 0) {
			split_words(cfg->exclude_files, value);
			FREE(value);
		} else {
			snprintf(err, errsz, "%s:%zu: unknown key %s", path, lineno, key);
			FREE(value);
			ok = false;
		}
		FREE(key);
	}
	fclose(f);
	FREE(dir);
	if (ok && strcmp(cfg->output_type, "markdown") != 0) {
		snprintf(err, errsz, "%s: unsupported OUTPUT_TYPE %s (only markdown)", path,
		         cfg->output_type);
		ok = false;
	}
	if (!ok) {
		cdconfig_free(cfg);
		return NULL;
	}
	return cfg;
}


void cdconfig_free(cdconfig *self)
{
	if (!self) {
		return;
	}
	FREE(self->project_name);
	FREE(self->version);
	FREE(self->output_type);
	FREE(self->output_dir);
	FREE(self->base_dir);
	free_str_vec(self->modules);
	free_str_vec(self->exclude_dirs);
	free_str_vec(self->exclude_files);
	FREE(self);
}


static bool in_list(const vec *v, const char *s)
{
	for (size_t i = 0, n = vec_len(v); i < n; i++) {
		if (strcmp(vec_get_rawptr(v, i), s) == 0) {
			return true;
		}
	}
	return false;
}


static bool matches_any(const vec *patterns, const char *name)
{
	for (size_t i = 0, n = vec_len(patterns); i < n; i++) {
		if (fnmatch(vec_get_rawptr(patterns, i), name, 0) == 0) {
			return true;
		}
	}
	return false;
}


static void scan(const cdconfig *cfg, const char *dir, vec *out)
{
	DIR *d = opendir(dir);
	if (!d) {
		return;
	}
	for (struct dirent *e = readdir(d); e; e = readdir(d)) {
		if (e->d_name[0] == '.') {
			continue;
		}
		char *path = cdpath_join(dir, e->d_name);
		struct stat st;
		if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
			if (!in_list(cfg->exclude_dirs, e->d_name)) {
				scan(cfg, path, out);
			}
		} else {
			size_t n = strlen(e->d_name);
			if (n > 2 && strcmp(e->d_name + n - 2, ".h") == 0
			        && !matches_any(cfg->exclude_files, e->d_name)) {
				vec_push_rawptr(out, path);
				path = NULL;
			}
		}
		FREE(path);
	}
	closedir(d);
}


static int cmp_str(const void *a, const void *b)
{
	return strcmp(*(const char **)a, *(const char **)b);
}


vec *cdconfig_find_headers(const cdconfig *self)
{
	vec *ret = vec_new(sizeof(char *));
	for (size_t i = 0, n = vec_len(self->modules); i < n; i++) {
		char *mod = cdpath_join(self->base_dir, vec_get_rawptr(self->modules, i));
		char *src = cdpath_join(mod, "src");
		vec *found = vec_new(sizeof(char *));
		scan(self, src, found);
		vec_qsort(found, cmp_str);
		for (size_t j = 0, m = vec_len(found); j < m; j++) {
			vec_push_rawptr(ret, vec_get_rawptr(found, j));
		}
		DESTROY_FLAT(found, vec);
		FREE(src);
		FREE(mod);
	}
	return ret;
}
