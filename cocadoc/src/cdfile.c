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
#include <stdlib.h>
#include <string.h>

#include "cddecl.h"
#include "cdfile.h"
#include "cdlexer.h"
#include "cstrutil.h"
#include "new.h"


// Takes ownership of path and src
static cdfile *cdfile_new(char *path, char *src, size_t len)
{
	cdfile *ret = NEW(cdfile);
	ret->path = path;
	char *slash = strrchr(path, '/');
	ret->name = slash ? slash + 1 : path;
	ret->src = src;
	ret->len = len;
	ret->toks = cdlex_all(src, len);
	ret->decls = cddecl_match(src, ret->toks);
	return ret;
}


cdfile *cdfile_load(const char *path)
{
	FILE *f = fopen(path, "rb");
	if (!f) {
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (sz < 0) {
		fclose(f);
		return NULL;
	}
	char *src = malloc(sz + 1);
	size_t len = fread(src, 1, sz, f);
	src[len] = '\0';
	fclose(f);
	return cdfile_new(cstr_clone(path), src, len);
}


cdfile *cdfile_new_from_str(const char *path, const char *src, size_t len)
{
	return cdfile_new(cstr_clone(path), cstr_clone_len(src, len), len);
}


void cdfile_free(cdfile *self)
{
	if (!self) {
		return;
	}
	cddecl_vec_free(self->decls);
	DESTROY_FLAT(self->toks, vec);
	FREE(self->src);
	FREE(self->path);
	FREE(self);
}
