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

#include <stddef.h>
#include <string.h>

#include "new.h"
#include "cstrutil.h"
#include "strbuf.h"
#include "mathutil.h"
#include "iter.h"


const static size_t DEFAULT_CAPACITY = 8;


typedef struct _strbuf {
	char *str;
	size_t len;
	size_t capacity;
}
strbuf;


strbuf *strbuf_new()
{
	return strbuf_new_with_capacity(DEFAULT_CAPACITY);
}


strbuf *strbuf_new_with_capacity(size_t init_capacity)
{
	strbuf *ret;
	ret = NEW(strbuf);
	ret->capacity = init_capacity;
	ret->len = 0;
	ret->str = cstr_new(ret->capacity);
	return ret;
}


strbuf *strbuf_new_from_str(const char *src)
{
	strbuf *ret;
	ret = NEW(strbuf);
	ret->len = ret->capacity = strlen(src);
	ret->str = cstr_new(ret->capacity);
	strcpy(ret->str, src);
	return ret;
}

void strbuf_free(strbuf *sb)
{
	if (sb==NULL) return;
	FREE(sb->str);
	FREE(sb);
}

size_t strbuf_len(strbuf *sb)
{
	return sb->len;
}

size_t strbuf_capacity(strbuf *sb)
{
	return sb->capacity;
}

char strbuf_get(strbuf *sb, size_t pos)
{
	return sb->str[pos];
}


void strbuf_clear(strbuf *sb)
{
	memset(sb->str, '\0', sb->capacity);
	sb->len = 0;
}


void strbuf_set(strbuf *sb, size_t pos, char c)
{
	sb->str[pos] = c;
}


void _double(strbuf *sb)
{
	sb->capacity = MAX(1, 2*sb->capacity);
	sb->str = realloc(sb->str, (sb->capacity+1)*sizeof(char));
	cstr_fill(sb->str, sb->len, sb->capacity, '\0');
}


void strbuf_append(strbuf *sb, const char *suff)
{
	size_t slen = strlen(suff);
	while (sb->len + slen >= sb->capacity) {
		_double(sb);
	}
	strcpy(sb->str+sb->len, suff);
	sb->len += slen;
	sb->str[sb->len] = '\0';

}


void strbuf_join(strbuf *sb, size_t n, const char**arr, const char *sep)
{
	for (size_t i=0; i<n; i++) {
		if (i) {
			strbuf_append(sb, sep);
		}
		strbuf_append(sb, arr[i]);
	}
}


void strbuf_join_iter(strbuf *sb, iter *it, const char *sep)
{
	for (size_t i=0; iter_has_next(it); i++) {
		if (i) {
			strbuf_append(sb, sep);
		}
		strbuf_append(sb, *((const char **)iter_next(it)));
	}
}


void strbuf_append_char(strbuf *sb, char c)
{
	if (sb->len == sb->capacity) {
		_double(sb);
	}
	sb->str[sb->len] = c;
	sb->len++;
	sb->str[sb->len] = '\0';
}


const char *strbuf_as_str(strbuf *sb)
{
	return sb->str;
}


char *strbuf_detach(strbuf *sb)
{
	char *str = sb->str;
	str = realloc(str, (sb->len+1));
	FREE(sb);
	return str;
}

