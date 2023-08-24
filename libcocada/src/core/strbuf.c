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
#include "memdbg.h"


const static size_t DEFAULT_CAP = 4;
const static float  GROW_BY = 1.62f;  // (!) 1 < GROW_BY <= 2

typedef struct _strbuf {
	char *str;
	size_t len; // string contents length, excluding the null-terminating char(s)
	size_t capacity; // string capacity. physical capacity is 1 + this because of ending ('\0')
}
strbuf;


static void _resize_to(strbuf *self, size_t min_cap)
{
	min_cap = MAX(min_cap, self->len); // losing data not allowed
	size_t cap;
	for (cap = MAX(DEFAULT_CAP, self->capacity); cap < min_cap; cap *= GROW_BY);
	self->str = realloc(self->str, (cap + 1) * sizeof(char));
	cstr_fill(self->str, self->len, cap, '\0');
	self->capacity = cap;
}

/*
static void _double(strbuf *self)
{
	self->capacity = MAX(1, 2*self->capacity);
	self->str = realloc(self->str, (self->capacity+1)*sizeof(char));
	cstr_fill(self->str, self->len, self->capacity, '\0');
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


strbuf *strbuf_new_from_str(const char *other, size_t len)
{
	strbuf *ret;
	ret = NEW(strbuf);
	ret->len = ret->capacity = len;
	ret->str = cstr_new(ret->capacity);
	strncpy(ret->str, other, len);
	return ret;
}


void strbuf_finalise(void *ptr, const finaliser *fnr)
{
	FREE(((strbuf *)ptr)->str);
}


void strbuf_free(strbuf *self)
{
	FREE(self->str);
	FREE(self);
}


size_t strbuf_len(strbuf *self)
{
	return self->len;
}


size_t strbuf_capacity(strbuf *self)
{
	return self->capacity;
}


void strbuf_fit(strbuf *self)
{
	_resize_to(self, self->len);
}


bool strbuf_eq(strbuf *self, strbuf *other)
{
	return (strcmp(self->str, other->str) == 0);
}



char strbuf_get(strbuf *self, size_t pos)
{
	return self->str[pos];
}


void strbuf_clear(strbuf *self)
{
	memset(self->str, '\0', self->capacity);
	self->len = 0;
}


void strbuf_set(strbuf *self, size_t pos, char c)
{
	self->str[pos] = c;
}


void strbuf_nappend(strbuf *self, const char *other, size_t len)
{
	_resize_to(self, self->len + len);
	strcpy(self->str + self->len, other);
	self->len += len;
	self->str[self->len] = '\0';

}


void strbuf_append(strbuf *self, const char *other)
{
	strbuf_nappend(self, other, strlen(other));
}


void strbuf_ncat(strbuf *dest, const strbuf *other, size_t n)
{
	strbuf_nappend(dest, (const char *)other->str, MIN(n, other->len));
}


void strbuf_cat(strbuf *dest, const strbuf *other)
{
	strbuf_nappend(dest, (const char *)other->str, other->len);
}


void strbuf_append_char(strbuf *self, char c)
{
	_resize_to(self, self->len + 1);
	self->str[self->len] = c;
	self->len++;
	self->str[self->len] = '\0';
}


void strbuf_join(strbuf *self, size_t n, const char **arr, const char *sep)
{
	size_t seplen = strlen(sep);
	for (size_t i=0; i<n; i++) {
		if (i) {
			strbuf_nappend(self, sep, seplen);
		}
		strbuf_nappend(self, arr[i], strlen(arr[i]));
	}
}


const char *strbuf_as_str(strbuf *self)
{
	return self->str;
}


char *strbuf_detach(strbuf *self)
{
	char *str = self->str;
	str = realloc(str, (self->len+1));
	FREE(self);
	return str;
}


void strbuf_ins(strbuf *self, size_t pos, const char *str, size_t len)
{
	assert(pos <= self->len);
	_resize_to(self, self->len + len);
	memmove(self->str + ((pos + len) * sizeof(char)),
	        self->str + (pos * sizeof(char)),
	        (self->len - pos) * sizeof(char));
	memcpy(self->str + (pos * sizeof(char)), str, len * sizeof(char));
	self->len += len;
}


void strbuf_cut(strbuf *self, size_t from, size_t len, char *dest)
{
	assert(from + len <= self->len);
	if (dest != NULL) {
		strncpy(dest, self->str + (from * sizeof(char)), len);
		dest[len] = '\0';
	}
	memmove(self->str + (from * sizeof(char)),
	        self->str + ((from + len) * sizeof(char)),
	        (self->len - (from + len)) * sizeof(char));
	self->len -= len;
	memset(self->str + (self->len * sizeof(char)), '\0', len);
}


void strbuf_paste(strbuf *self, size_t from, const char *other, size_t len)
{
	assert (from <= self->len);
	if (from + len <= self->len) {
		memcpy(self->str + (from * sizeof(char)), other, len * sizeof(char));
	}
	else {
		size_t hang = len - (self->len - from);
		memcpy(self->str + (from * sizeof(char)), other, (len - hang) * sizeof(char));
		strbuf_nappend(self, other + (len - hang) * sizeof(char), hang);
	}
}


void strbuf_clip(strbuf *self, size_t from, size_t to)
{
	assert(from <= to && to <= self->len);
	memmove(self->str, self->str + from, to - from);
	self->len = to - from;
	self->str[self->len] = '\0';
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


size_t strbuf_find_n(strbuf *self, const char *old, size_t n, size_t from_pos,
                     size_t *dest)
{
	size_t patlen = strlen(old);
	if (n == 0 || from_pos + patlen > self->len) {
		return 0;
	}
	fsm *matcher = build_fsm(old, patlen);
	size_t ret = 0;
	if (patlen == 0 && n > 0) {
		dest[ret++] = from_pos;
	}
	for (size_t i = from_pos, state = 0; ret < n && i < self->len; i++) {
		state = matcher->delta[(size_t)self->str[i]][state];
		if (state == patlen) {
			assert (i + 1 >= patlen );
			dest[ret++] = i + 1 - patlen;
		}
	}
	fsm_free(matcher);
	return ret;
}


size_t strbuf_replace_n(strbuf *self, const char *old, const char *new,
                        size_t n, size_t from)
{
	if (from > self->len) return 0;
	size_t patlen = strlen(old);
	size_t repllen = strlen(new);
	fsm *matcher = build_fsm(old, patlen);
	stack *occ = stack_new(sizeof(size_t));
	uint occ_count = 0;
	if (patlen == 0 && occ_count < n ) {
		stack_push_size_t(occ, 0);
		occ_count++;
	}
	for (size_t i = from, state = 0; occ_count < n && i < self->len; i++) {
		state = matcher->delta[(size_t)self->str[i]][state];
		if (state == patlen) {
			assert (i + 1 >= patlen );
			stack_push_size_t(occ, i + 1 - patlen );
			occ_count++;
		}
	}
	while (!stack_empty(occ)) {
		size_t pos = stack_pop_size_t(occ);
		if (repllen > patlen) {
			strbuf_paste(self, pos, new, patlen);
			strbuf_ins(self, pos + patlen, new + patlen, repllen - patlen);
		}
		else if (repllen < patlen) {
			strbuf_cut(self, pos, patlen - repllen, NULL);
			strbuf_paste(self, pos, new, repllen);
		}
		else {   // equals
			strbuf_paste(self, pos, new, patlen);
		}
	}
	DESTROY_FLAT(occ, stack);
	fsm_free(matcher);
	return occ_count;
}


size_t strbuf_replace(strbuf *self, const char *old, const char *new,
                      size_t from)
{
	return strbuf_replace_n(self, old, new, 1, from);
}


size_t strbuf_replace_all(strbuf *self, const char *old, const char *new,
                          size_t from)
{
	return strbuf_replace_n(self, old, new, SIZE_MAX, from);
}


int sbprintf(strbuf *self, const char *fmt, ...)
{
	size_t fmt_len = strlen(fmt);
	size_t avail = self->capacity - self->len;
	if (avail < fmt_len) {
		_resize_to(self, self->len + (2 * fmt_len));
	}
	avail = self->capacity - self->len;
	char *dest = self->str + self->len;
	va_list valist;
	va_start(valist, fmt);
	int written = vsnprintf(dest, avail, fmt, valist);
	va_end(valist);
	if (written >= avail) {
		_resize_to(self, self->len + written + 1);
		dest = self->str + self->len;
		avail = self->capacity - self->len;
		va_start(valist, fmt);
		written = vsnprintf(dest, avail, fmt, valist);
		va_end(valist);
		assert(written < avail);
	}
	self->len += written;
	return written;
}
