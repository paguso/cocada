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
 * @file cdsym.c
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 */

#include <stdint.h>
#include <string.h>

#include "cddoc.h"
#include "cdsym.h"
#include "cstrutil.h"
#include "hash.h"
#include "hashmap.h"
#include "new.h"


#define NONE SIZE_MAX

typedef struct {
	cdsym sym;
	size_t next; // next symbol with the same name, or NONE
} entry;


typedef struct {
	const cdfile *file;
	vec *families;  // vec of cdfamily
	vec *warns;     // vec of cdmacrowarn
} file_gen;


struct _cdsymtab {
	vec *entries;   // vec of entry
	hashmap *index; // name (char *) -> position of its first entry (size_t)
	cdmacrotab *macros;
	vec *gens;      // vec of file_gen, one per file
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


// Takes ownership of name
static void add(cdsymtab *t, char *name, cddecl_kind kind, const cdfile *file,
                const cddecl *decl, const cddecl *parent, const cdfamily *family,
                bool hidden)
{
	entry e = {
		.sym = {
			.name = name, .kind = kind, .file = file, .decl = decl, .parent = parent,
			.family = family, .hidden = hidden
		},
		.next = NONE
	};
	size_t pos = vec_len(t->entries);
	vec_push(t->entries, &e);
	if (hashmap_contains(t->index, &name)) {
		// append to the chain of declarations with this name
		size_t k = hashmap_get_size_t(t->index, &name);
		entry *last = vec_get_mut(t->entries, k);
		while (last->next != NONE) {
			last = vec_get_mut(t->entries, last->next);
		}
		last->next = pos;
	} else {
		hashmap_ins_size_t(t->index, &name, pos);
	}
}


static char *qualified(const char *type, const char *member)
{
	size_t lt = strlen(type), lm = strlen(member);
	char *ret = cstr_new(lt + lm + 1);
	memcpy(ret, type, lt);
	ret[lt] = '.';
	memcpy(ret + lt + 1, member, lm + 1);
	return ret;
}


static void add_file(cdsymtab *t, const cdfile *f)
{
	const cddecl *file_doc = NULL;
	for (size_t i = 0, n = vec_len(f->decls); i < n && !file_doc; i++) {
		const cddecl *d = vec_get(f->decls, i);
		if (d->kind == CDD_FILE) {
			file_doc = d;
		}
	}
	add(t, cstr_clone(f->name), CDD_FILE, f, file_doc, NULL, NULL, false);

	for (size_t i = 0, n = vec_len(f->decls); i < n; i++) {
		const cddecl *d = vec_get(f->decls, i);
		if (d->kind == CDD_FILE || d->kind == CDD_MACROCALL || d->name[0] == '\0') {
			continue;
		}
		bool hidden = cddoc_hidden(d->doc);
		add(t, cstr_clone(d->name), d->kind, f, d, NULL, NULL, hidden);
		bool is_enum = d->sig && (strstr(d->sig, "enum ") == d->sig
		                          || strncmp(d->sig, "typedef enum", 12) == 0);
		for (size_t j = 0, m = d->members ? vec_len(d->members) : 0; j < m; j++) {
			const cddecl *mb = vec_get(d->members, j);
			if (mb->name[0] == '\0') {
				continue;
			}
			add(t, qualified(d->name, mb->name), CDD_MEMBER, f, mb, d, NULL, hidden);
			if (is_enum) {
				add(t, cstr_clone(mb->name), CDD_MEMBER, f, mb, d, NULL, hidden);
			}
		}
	}
}


cdsymtab *cdsymtab_new(const vec *files)
{
	cdsymtab *ret = NEW(cdsymtab);
	ret->entries = vec_new(sizeof(entry));
	ret->index = hashmap_new(sizeof(char *), sizeof(size_t), hash_str, eq_str);
	for (size_t i = 0, n = vec_len(files); i < n; i++) {
		add_file(ret, vec_get_rawptr(files, i));
	}
	// macro-generated declarations
	ret->macros = cdmacrotab_new(files);
	ret->gens = vec_new(sizeof(file_gen));
	for (size_t i = 0, n = vec_len(files); i < n; i++) {
		file_gen g = {.file = vec_get_rawptr(files, i), .warns = vec_new(sizeof(cdmacrowarn))};
		g.families = cdmacro_families(ret->macros, g.file, g.warns);
		vec_push(ret->gens, &g);
		for (size_t k = 0, kn = vec_len(g.families); k < kn; k++) {
			const cdfamily *fam = vec_get(g.families, k);
			if (fam->pattern && vec_len(fam->instances) > 1) {
				add(ret, cstr_clone(fam->pattern->name), fam->pattern->kind, g.file,
				    fam->pattern, NULL, fam, fam->hidden);
			}
			for (size_t j = 0, jn = vec_len(fam->instances); j < jn; j++) {
				const cdinstance *in = vec_get(fam->instances, j);
				add(ret, cstr_clone(in->decl->name), in->decl->kind, g.file, in->decl, NULL, fam,
				    fam->hidden);
			}
		}
	}
	return ret;
}


void cdsymtab_free(cdsymtab *self)
{
	if (!self) {
		return;
	}
	DESTROY_FLAT(self->index, hashmap);
	for (size_t i = 0, n = vec_len(self->gens); i < n; i++) {
		file_gen *g = vec_get_mut(self->gens, i);
		cdfamily_vec_free(g->families);
		cdmacrowarn_vec_free(g->warns);
	}
	DESTROY_FLAT(self->gens, vec);
	cdmacrotab_free(self->macros);
	for (size_t i = 0, n = vec_len(self->entries); i < n; i++) {
		entry *e = vec_get_mut(self->entries, i);
		FREE(e->sym.name);
	}
	DESTROY_FLAT(self->entries, vec);
	FREE(self);
}


const cdsym *cdsymtab_resolve(const cdsymtab *self, const char *name,
                              const cdfile *from, size_t *ncands)
{
	if (ncands) {
		*ncands = 0;
	}
	if (!hashmap_contains(self->index, &name)) {
		return NULL;
	}
	size_t k = hashmap_get_size_t(self->index, &name);
	const entry *first = vec_get(self->entries, k);
	const cdsym *ret = &first->sym;
	bool local = false;
	for (const entry *e = first; ; e = vec_get(self->entries, e->next)) {
		if (ncands) {
			// count files, not declarations: alternative declarations in
			// one file (e.g. in #if branches) are not ambiguous
			bool new_file = true;
			for (const entry *p = first; p != e; p = vec_get(self->entries, p->next)) {
				new_file &= p->sym.file != e->sym.file;
			}
			*ncands += new_file;
		}
		if (!local && from && e->sym.file == from) {
			ret = &e->sym;
			local = true;
		}
		if (e->next == NONE) {
			break;
		}
	}
	return ret;
}


static const file_gen *gen_of(const cdsymtab *self, const cdfile *f)
{
	for (size_t i = 0, n = vec_len(self->gens); i < n; i++) {
		const file_gen *g = vec_get(self->gens, i);
		if (g->file == f) {
			return g;
		}
	}
	return NULL;
}


const vec *cdsymtab_families(const cdsymtab *self, const cdfile *f)
{
	const file_gen *g = gen_of(self, f);
	return g ? g->families : NULL;
}


const vec *cdsymtab_macro_warnings(const cdsymtab *self, const cdfile *f)
{
	const file_gen *g = gen_of(self, f);
	return g ? g->warns : NULL;
}


const cdmacrotab *cdsymtab_macros(const cdsymtab *self)
{
	return self->macros;
}


size_t cdsymtab_size(const cdsymtab *self)
{
	return vec_len(self->entries);
}


const cdsym *cdsymtab_get(const cdsymtab *self, size_t i)
{
	return &((const entry *)vec_get(self->entries, i))->sym;
}
