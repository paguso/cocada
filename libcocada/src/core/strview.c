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

#include <assert.h>
#include <string.h>

#include "cstrutil.h"
#include "errlog.h"
#include "new.h"
#include "strview.h"


void strview_init(strview *self, const char *src)
{
	strview_init_from_range(self, src, 0, strlen(src));
}


strview strview_build(const char *src)
{
	strview ret;
	strview_init_from_range(&ret, src, 0, strlen(src));
	return ret;
}


strview *strview_new(const char* src)
{
	strview *ret = NEW(strview);
	strview_init_from_range(ret, src, 0, strlen(src));
	return ret;
}


void strview_init_from_range(strview *self, const char *src, size_t from, size_t to)
{
	assert (from <= to);
	self->str = src + from;
	self->len = to - from;
}


strview strview_build_from_range(const char *src, size_t from, size_t to)
{
	strview ret;
	strview_init_from_range(&ret, src, from, to);
	return ret;
}


strview* strview_new_from_range(const char *src, size_t from, size_t to)
{
	strview *ret = NEW(strview);
	strview_init_from_range(ret, src, from, to);
	return ret;
}


char *strview_to_new_cstr(strview *self)
{
	char *ret = cstr_new(self->len);
	strview_cpy_to_cstr(self, ret);
	return ret;
}


void strview_cpy_to_cstr(strview *self, char *dest)
{
	strncpy(dest, self->str, self->len);
}


size_t strview_len(strview *self)
{
	return self->len;
}


bool strview_eq_cstr(strview *self, const char *other)
{
	return strview_eqn_cstr(self, other, strlen(other));
}


bool strview_eqn_cstr(strview *self, const char *other, size_t n)
{
	if (self->len != n) return false;
	for (size_t i = 0 ; i < n; i++) {
		if (self->str[i] != other[i]) {
			return false;
		}
	}
	return true;
}


strview strview_substr(strview *self, size_t from, size_t to)
{
	assert(from <= to);
	assert(to - from <= self->len);
	strview ret;
	strview_init_from_range(&ret, self->str, from, to);
	return ret;
}


// Aho-corasick-like FSM
typedef struct {
	int n;
	int *delta[256];
} fsm;


static void fsm_free(fsm *f)
{
	for (size_t i = 0; i < 256; free(f->delta[i++]));
	free(f);
}


static fsm *build_fsm(const char *pat, int len)
{
	assert( 0 <= len );
	fsm *ret = NEW(fsm);
	ret->n = len + 1;
	int fail = 0;
	int m = 256; // 1-byte char assumed
	for (int i = 0; i < m; i++) {
		ret->delta[i] = calloc(ret->n, sizeof(int));
	}
	for (int j = 0 ; j < len; j++) {
		for (int i = 0; i < m; i++) {
			ret->delta[i][j] = ret->delta[i][fail];
		}
		ret->delta[(size_t)pat[j]][j] = j + 1;
		fail = ret->delta[(size_t)pat[j]][fail];
	}
	for (int i = 0; i < m; i++) {
		ret->delta[i][ret->n - 1] = ret->delta[i][0];
	}
	return ret;
}


size_t strview_find_n(strview *self, const char *pat, size_t n,
                      size_t *dest)
{
	size_t self_len = strview_len(self);
	size_t patlen = strlen(pat);
	if (n == 0 ||  patlen > self_len) {
		return 0;
	}
	fsm *matcher = build_fsm(pat, patlen);
	size_t ret = 0;
	if (patlen == 0 && n > 0) {
		dest[ret++] = 0;
	}
	const char *self_str = self->str;
	for (size_t i = 0, state = 0; ret < n && i < self_len; i++) {
		state = matcher->delta[(size_t)self_str[i]][state];
		if (state == patlen) {
			assert (i + 1 >= patlen );
			dest[ret++] = i + 1 - patlen;
		}
	}
	fsm_free(matcher);
	return ret;
}


typedef enum {
	SPLITTER = 0
} strview_iter_type;


typedef struct {
	char *sep;
	size_t sep_len;
	size_t pos;
} split_data_t;


typedef struct {
	bool (*has_next) (strview_iter *it);
	strview (*next) (strview_iter *it);
} strview_iter_vt;


struct strview_iter {
	strview_iter_vt *vt;
	strview *src;
	strview_iter_type type;
	union {
		split_data_t split_data;
	} aux;
};


void strview_iter_free(strview_iter *self)
{
	if (!self) return;
	switch (self->type) {
	case SPLITTER:
		FREE(self->aux.split_data.sep);
		break;
	default:
		break;
	}
	FREE(self);
}


bool strview_iter_has_next(strview_iter *it)
{
	return it->vt->has_next(it);
}


strview strview_iter_next(strview_iter *it)
{
	return it->vt->next(it);
}



static bool _strview_split_has_next(strview_iter *it)
{
	size_t pos = it->aux.split_data.pos;
	size_t len = strview_len(it->src);
	return pos <= len;
}

static strview _strview_split_next(strview_iter *it)
{
	size_t pos = it->aux.split_data.pos;
	const char *str = it->src->str;
	const char *sep = it->aux.split_data.sep;
	size_t sep_len = it->aux.split_data.sep_len;
	size_t len = strview_len(it->src);
	assert(pos <= len);
	// find next occurrence of separator
	size_t sep_pos = pos;
	while (sep_pos < len) {
		size_t j;
		for (j = 0; j < sep_len && sep_pos + j < len && sep[j] == str[sep_pos + j]; j++);
		if (j == sep_len)
			break;
		sep_pos++;
	}
	if (sep_pos < len) { // found separator at sep_pos
		strview ret = {.str = str + pos, .len = sep_pos - pos };
		it->aux.split_data.pos = sep_pos + sep_len;
		return ret;
	} else { // separator not found
		strview ret = {.str = str + pos, .len = len - pos };
		it->aux.split_data.pos = len + 1;
		return ret;
	}
}


static strview_iter_vt _strview_split_vt = {_strview_split_has_next, _strview_split_next};


strview_iter *strview_split(strview *self, const char *sep, size_t sep_len)
{
	ERROR_ASSERT(sep_len > 0, "strview split separator must be non-empty.");
	strview_iter *ret = NEW(strview_iter);
	ret->type = SPLITTER;
	ret->src = self;
	ret->aux.split_data = (split_data_t) {
		.sep = cstr_clone_len(sep, sep_len), .sep_len = sep_len, .pos = 0
	};
	ret->vt = &_strview_split_vt;
	return ret;
}


