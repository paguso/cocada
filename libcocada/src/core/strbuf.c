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
#include <stddef.h>
#include <string.h>

#include "new.h"
#include "cstrutil.h"
#include "strbuf.h"
#include "stack.h"
#include "mathutil.h"


const static size_t DEFAULT_CAP = 4;
const static float  GROW_BY = 1.62f;  // (!) 1 < GROW_BY <= 2

typedef struct _strbuf {
	char *str;
	size_t len;
	size_t capacity; // string capacity. physical capacity is 1 + this because of ending ('\0')
}
strbuf;


static void _resize_to(strbuf *sb, size_t min_cap)
{
	min_cap = MAX(min_cap, sb->len); // losing data not allowed
	size_t cap;
	for(cap = MAX(DEFAULT_CAP, sb->capacity); cap < min_cap; cap *= GROW_BY);
	sb->str = realloc(sb->str, (cap + 1) * sizeof(char));
	cstr_fill(sb->str, sb->len, cap, '\0');
	sb->capacity = cap;
}

/*
static void _double(strbuf *sb)
{
	sb->capacity = MAX(1, 2*sb->capacity);
	sb->str = realloc(sb->str, (sb->capacity+1)*sizeof(char));
	cstr_fill(sb->str, sb->len, sb->capacity, '\0');
}
*/

strbuf *strbuf_new()
{
	return strbuf_new_with_capacity(DEFAULT_CAP);
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


strbuf *strbuf_new_from_str(const char *src, size_t len)
{
	strbuf *ret;
	ret = NEW(strbuf);
	ret->len = ret->capacity = len;
	ret->str = cstr_new(ret->capacity);
	strncpy(ret->str, src, len);
	return ret;
}


void strbuf_finalise(void *ptr, const finaliser *fnr)
{
	FREE(((strbuf *)ptr)->str);
}


void strbuf_free(strbuf *sb)
{
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


void strbuf_nappend(strbuf *sb, const char *src, size_t len)
{
	_resize_to(sb, sb->len + len);
	strcpy(sb->str + sb->len, src);
	sb->len += len;
	sb->str[sb->len] = '\0';

}


void strbuf_append(strbuf *sb, const char *src)
{
	strbuf_nappend(sb, src, strlen(src));	
}


void strbuf_ncat(strbuf *dest, const strbuf *src, size_t n)
{
	strbuf_nappend(dest, (const char *)src->str, MIN(n, src->len));
}


void strbuf_cat(strbuf *dest, const strbuf *src)
{
	strbuf_nappend(dest, (const char *)src->str, src->len);
}


void strbuf_append_char(strbuf *sb, char c)
{
	_resize_to(sb, sb->len + 1);
	sb->str[sb->len] = c;
	sb->len++;
	sb->str[sb->len] = '\0';
}


void strbuf_join(strbuf *sb, size_t n, const char **arr, const char *sep)
{
	size_t seplen = strlen(sep);
	for (size_t i=0; i<n; i++) {
		if (i) {
			strbuf_nappend(sb, sep, seplen);
		}
		strbuf_nappend(sb, arr[i], strlen(arr[i]));
	}
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


void strbuf_ins(strbuf *sb, size_t pos, const char *str, size_t len)
{
	assert(pos <= sb->len);
	_resize_to(sb, sb->len + len);
	memmove(sb->str + ((pos + len) * sizeof(char)), sb->str + (pos * sizeof(char)), (sb->len - pos) * sizeof(char));
	memcpy(sb->str + (pos * sizeof(char)), str, len * sizeof(char));
	sb->len += len;
}


void strbuf_cut(strbuf *sb, size_t from, size_t len, char *dest)
{
	assert(from + len <= sb->len);
	if (dest != NULL) {
		strncpy(dest, sb->str + (from * sizeof(char)), len);
		dest[len] = '\0';
	}
	memmove(sb->str + (from * sizeof(char)), sb->str + ((from + len) * sizeof(char)), (sb->len - (from + len)) * sizeof(char));
	sb->len -= len;
	memset(sb->str + (sb->len * sizeof(char)), '\0', len);
}


void strbuf_paste(strbuf *sb, size_t from, const char *src, size_t len)
{
	assert (from <= sb->len);
	if (from + len <= sb->len) {
		memcpy(sb->str + (from * sizeof(char)), src, len * sizeof(char));
	} else {
		size_t hang = len - (sb->len - from);
		memcpy(sb->str + (from * sizeof(char)), src, (len - hang) * sizeof(char));
		strbuf_nappend(sb, src + (len - hang) * sizeof(char), hang);
	}
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


void strbuf_replace_n(strbuf *sb, const char *old, const char *new, size_t n)
{
	size_t patlen = strlen(old);
	size_t repllen = strlen(new);
	fsm *matcher = build_fsm(old, patlen);
	stack *occ = stack_new(sizeof(size_t));
	size_t occ_count = 0;
	if (patlen == 0 && occ_count < n ) {
		stack_push_size_t(occ, 0);
		occ_count++;
	}
	for (size_t i = 0, state = 0; occ_count < n && i < sb->len; i++) {
		state = matcher->delta[(size_t)sb->str[i]][state];
		if (state == patlen) {
			assert (i + 1 >= patlen );
			stack_push_size_t(occ, i + 1 - patlen );
			occ_count++;
		}
	}
	while (!stack_empty(occ)) {
		size_t pos = stack_pop_size_t(occ);
		if (repllen > patlen) {
			strbuf_paste(sb, pos, new, patlen);
			strbuf_ins(sb, pos + patlen, new + patlen, repllen - patlen);
		} else if (repllen < patlen) {
			strbuf_cut(sb, pos, patlen - repllen, NULL);
			strbuf_paste(sb, pos, new, repllen);
		} else { // equals
			strbuf_paste(sb, pos, new, patlen);
		}
	}
	DESTROY_FLAT(occ, stack);
	fsm_free(matcher);
}


void strbuf_replace(strbuf *sb, const char *old, const char *new)
{
	strbuf_replace_n(sb, old, new, 1);
}


void strbuf_replace_all(strbuf *sb, const char *old, const char *new)
{
	strbuf_replace_n(sb, old, new, SIZE_MAX);
}


