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

#include "bitbyte.h"
#include "coretype.h"
#include "errlog.h"
#include "new.h"
#include "cstrutil.h"
#include "strbuf.h"
#include "stack.h"
#include "mathutil.h"


const static float  GROW_BY = 1.62f;  // (!) 1 < GROW_BY <= 2

/*
 * Short string optimization layout:
 *
 * The standard string buffer has three components:
 * - A pointer to a heap allocated buffer (char* pointer)
 * - The string buffer capacity           (uint64_t)
 * - The string buffer size               (uint64_t)
 *
 * On a typical 64-bit architecture each of these fields has 8bytes with
 * no internal or terminal padding required. So the whole structure
 * occupies 24 bytes.
 *
 * +----------------+----------------+----------------+
 * |  buffer*       |  capacity      | length         |
 * +----------------+----------------+----------------+
 *
 * For short strings this is very inefficient and cost unecessary allocation.
 * The Short-string optimisation idea is to use most of the struct's bytes
 * to store the string itself. We use all the bytes, except the last two:
 * - The second-to-last byte will have a null-terminating '\0' char
 * - The last byte is used as follows:
 *   - the most significant bit is set to 1 to indicate that the string
 *     buffer contains a short string
 *   - the remaining 7 bits are used to store the small string length
 *
 *               +----------------+----------------+----------------+
 * long_strfuf:  |  buffer*       |  capacity      | length         |
 *               +----------------+----------------+----------------+
 *               +----------------------------------------------+-+-+
 * short_strbuf: |                                              |0|S|
 *               +----------------------------------------------+-+-+
 *                                                               | |
 *                                   '\0' string terminator <---´  |
 *                                                                 |
 *                                        +-+-+-+-+-+-+-+-+        |
 *                        Last byte:      |1| 7bit length | <-----´
 *                                        +-+-+-+-+-+-+-+-+
 *                                         |
 *                                         v
 *                                 shor string 1-bit flag
 *
 * On x86-64bit architecture, the maximum short string size is therefore
 * 22, not counting the final '\0'. Thus any size fits in the last
 * 7 bits, which could account for lengths in the 0..127 range.
 *
 * Notice that the the most significant bit of the last byte being used
 * as a short string flag means that, once the string outgrows the
 * short string capacity, this last byte must be set to zero and
 * should no longer be used for long_strbuf.length. This has two
 * implications:
 * 1. The maximum capacity/length for long strings is capped at 2^56 - 2
 *    (0..2^56-1 minus one for the last '\0')
 * 2. On little-endian architecture, this last byte is the most significant
 *    byte of long_strbuf.lenght, so no problem because it won't be used
 *    anyways since we do not allow the capacity to go past the above cap.
 *    However, on a big-endian machine, this last byte would be the
 *    least significant byte of long_strbuf.length, and it would have to
 *    be kept 0. To work around this problem, in this case we actually
 *    store (length << 8) in long_strbuf.length.
 */

typedef struct {
	char  *buf;
	uint64_t cap; // string capacity. physical capacity is 1 + this because of ending ('\0')
	uint64_t len; // string contents length, excluding the null-terminating char(s)
} long_strbuf;

#define ISSHORTMASK   ((byte_t)(0x80))
#define SHORTLENMASK  ((byte_t)(0x7F))
#define SHORT_CAP (sizeof(long_strbuf)  - 2)
#define MAX_CAP ((1ULL << 56) - 2ULL)

typedef struct _strbuf {
	union {
		long_strbuf lsb;
		char ssb[SHORT_CAP + 2];
	} data;
} strbuf;

static_assert(offsetof(long_strbuf, len) + sizeof(uint64_t) == sizeof(long_strbuf),
              "strbuf SSO: len must occupy the last bytes of long_strbuf");
static_assert(sizeof(((strbuf *)0)->data.ssb) == sizeof(long_strbuf),
              "strbuf SSO: short buffer must exactly overlay long_strbuf");
static_assert(SHORT_CAP <= SHORTLENMASK,
              "strbuf SSO: short length must fit in 7 bits");

static inline bool _is_short(strbuf *self)
{
	return (self->data.ssb[SHORT_CAP + 1] & ISSHORTMASK);
}

static inline size_t _short_len(strbuf *self)
{
	return (size_t)(self->data.ssb[SHORT_CAP + 1] & SHORTLENMASK);
}

static inline void _set_short_len(strbuf *self, size_t len)
{
	self->data.ssb[SHORT_CAP + 1] = (byte_t)len | ISSHORTMASK;
}

static void _set_len(strbuf *self, size_t len)
{
	if (_is_short(self)) {
		assert(_short_len(self) <= SHORT_CAP);
		assert(len <= SHORT_CAP);
		_set_short_len(self, len);
	} else {
		assert(len <= self->data.lsb.cap);
#if ENDIANNESS==LITTLE
		self->data.lsb.len = len;
#elif ENDIANNESS==BIG
		self->data.lsb.len = ((uint64_t)len << 8);
#endif
	}
}

static inline char* _buffer(strbuf *self)
{
	return _is_short(self) ? self->data.ssb : self->data.lsb.buf;
}

static void _resize_to(strbuf *self, size_t new_cap)
{
	ERROR_ASSERT(new_cap <= MAX_CAP, "Maximum strbuf capacity (%zu) exceeded.", (size_t)MAX_CAP);
	size_t cur_cap = strbuf_capacity(self);
	if (cur_cap >= new_cap) return;
	for (; cur_cap < new_cap; cur_cap *= GROW_BY);
	cur_cap = MIN(MAX_CAP, cur_cap);
	if (cur_cap <= SHORT_CAP) {
		assert (_is_short(self));
		// still using SSO buffer. do nothing
	} else if (_is_short(self)) {
		// move from SSO buffer to heap-allocated string
		char *str = cstr_new(cur_cap);
		size_t len  = _short_len(self);
		cstr_ncpy(str, self->data.ssb, _short_len(self));
		self->data.lsb.buf = str;
		self->data.lsb.cap = cur_cap;
		self->data.lsb.len = 0; // (!) clear short str flag before resetting length
		_set_len(self, len);
	} else {
		// already heap-allocated
		assert (!_is_short(self));
		self->data.lsb.buf = cstr_resize(self->data.lsb.buf, cur_cap);
		self->data.lsb.cap = cur_cap;
	}
}


strbuf *strbuf_new()
{
	return strbuf_new_with_capacity(SHORT_CAP);
}


strbuf *strbuf_new_with_capacity(size_t init_capacity)
{
	strbuf *ret;
	ret = NEW(strbuf);
	if (init_capacity <= SHORT_CAP) {
		ret->data.lsb = (long_strbuf) {
			.buf = 0, .cap = 0, .len = 0
		};
		ret->data.ssb[SHORT_CAP] = '\0';
		ret->data.ssb[SHORT_CAP + 1] = ISSHORTMASK;
	} else {
		ret->data.lsb.buf = cstr_new(init_capacity);
		ret->data.lsb.cap = init_capacity;
		ret->data.lsb.len = 0;
	}
	return ret;
}


strbuf *strbuf_new_from_str(const char *src, size_t len)
{
	strbuf *ret = strbuf_new_with_capacity(len);
	strbuf_nappend(ret, src, len);
	return ret;
}


void strbuf_finalise(void *ptr, const finaliser *fnr)
{
	strbuf *self = (strbuf *)ptr;
	if (!_is_short(self)) {
		FREE(self->data.lsb.buf);
	}
}


void strbuf_free(strbuf *self)
{
	if (!_is_short(self)) {
		FREE(self->data.lsb.buf);
	}
	FREE(self);
}



size_t strbuf_len(strbuf *self)
{
	if (_is_short(self)) {
		return _short_len(self);
	} else {
#if ENDIANNESS==LITTLE
		return self->data.lsb.len;
#elif ENDIANNESS==BIG
		return self->data.lsb.len >> 8;
#endif
	}
}

size_t strbuf_capacity(strbuf *self)
{
	return _is_short(self) ? SHORT_CAP : self->data.lsb.cap;
}


void strbuf_fit(strbuf *self)
{
	if (_is_short(self)) return;
	size_t cur_len = strbuf_len(self);
	if (cur_len <= SHORT_CAP) {
		// reduce to short string
		char *buf = _buffer(self);
		self->data.lsb = (long_strbuf) {
			0
		};
		cstr_ncpy(self->data.ssb, buf, cur_len);
		_set_short_len(self, cur_len);
		FREE(buf);
	} else {
		assert (!_is_short(self));
		self->data.lsb.buf = cstr_crop_len(self->data.lsb.buf, cur_len);
		self->data.lsb.cap = cur_len;
		_set_len(self, cur_len);
	}
}


bool strbuf_eq(strbuf *self, strbuf *other)
{
	return (strcmp(_buffer(self), _buffer(other)) == 0);
}



char strbuf_get(strbuf *self, size_t pos)
{
	return _buffer(self)[pos];
}


void strbuf_clear(strbuf *self)
{
	memset(_buffer(self), '\0', strbuf_capacity(self));
	_set_len(self, 0);
}


void strbuf_set(strbuf *self, size_t pos, char c)
{
	_buffer(self)[pos] = c;
}


void strbuf_nappend(strbuf *self, const char *other, size_t len)
{
	size_t self_len = strbuf_len(self);
	_resize_to(self, self_len + len);
	char *str = _buffer(self);
	strncpy(str + self_len, other, len);
	self_len += len;
	str[self_len] = '\0';
	_set_len(self, self_len);

}


void strbuf_append(strbuf *self, const char *other)
{
	strbuf_nappend(self, other, strlen(other));
}


void strbuf_ncat(strbuf *dest, const strbuf *other, size_t n)
{
	strbuf_nappend(dest, strbuf_as_str((strbuf*)other), MIN(n, strbuf_len((strbuf*)other)));
}


void strbuf_cat(strbuf *dest, const strbuf *other)
{
	strbuf_nappend(dest, strbuf_as_str((strbuf*)other), strbuf_len((strbuf*)other));
}


void strbuf_append_char(strbuf *self, char c)
{
	size_t self_len = strbuf_len(self);
	_resize_to(self, self_len + 1);
	char *str = _buffer(self);
	str[self_len] = c;
	self_len++;
	str[self_len] = '\0';
	_set_len(self, self_len);
}


void strbuf_join(strbuf *self, size_t n, const char **arr, const char *sep)
{
	size_t seplen = strlen(sep);
	for (size_t i = 0; i < n; i++) {
		if (i) {
			strbuf_nappend(self, sep, seplen);
		}
		strbuf_nappend(self, arr[i], strlen(arr[i]));
	}
}


const char *strbuf_as_str(strbuf *self)
{
	return _buffer(self);
}


char *strbuf_detach(strbuf *self)
{
	char *str = _is_short(self) ? cstr_clone(_buffer(self)) : _buffer(self);
	str = cstr_crop(str, 0, strbuf_len(self));
	FREE(self);
	return str;
}


void strbuf_ins(strbuf *self, size_t pos, const char *str, size_t len)
{
	size_t self_len = strbuf_len(self);
	assert(pos <= self_len);
	_resize_to(self, self_len + len);
	char *self_str = _buffer(self);
	memmove(self_str + pos + len,
	        self_str + pos,
	        self_len - pos);
	memcpy(self_str + pos, str, len );
	self_len += len;
	_set_len(self, self_len);
}


void strbuf_cut(strbuf *self, size_t from, size_t len, char *dest)
{
	size_t self_len = strbuf_len(self);
	assert(from + len <= self_len);
	char *self_str = _buffer(self);
	if (dest != NULL) {
		strncpy(dest, self_str + from, len);
		dest[len] = '\0';
	}
	memmove(self_str + from,
	        self_str + from + len,
	        self_len - (from + len));
	self_len -= len;
	memset(self_str + self_len, '\0', len);
	_set_len(self, self_len);
}


void strbuf_paste(strbuf *self, size_t from, const char *other, size_t len)
{
	size_t self_len = strbuf_len(self);
	assert (from <= self_len);
	char *self_str = _buffer(self);
	if (from + len <= self_len) {
		memcpy(self_str + from, other, len);
	} else {
		size_t hang = len - (self_len - from);
		memcpy(self_str + from, other, len - hang );
		strbuf_nappend(self, other + (len - hang) * sizeof(char), hang);
	}
}


void strbuf_clip(strbuf *self, size_t from, size_t to)
{
	size_t self_len = strbuf_len(self);
	assert(from <= to && to <= self_len);
	char *self_str = _buffer(self);
	memmove(self_str, self_str + from, to - from);
	self_len = to - from;
	self_str[self_len] = '\0';
	_set_len(self, self_len);
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


size_t strbuf_find_n(strbuf *self, const char *pat, size_t n, size_t from_pos,
                     size_t *dest)
{
	size_t self_len = strbuf_len(self);
	size_t patlen = strlen(pat);
	if (n == 0 || from_pos + patlen > self_len) {
		return 0;
	}
	fsm *matcher = build_fsm(pat, patlen);
	size_t ret = 0;
	if (patlen == 0 && n > 0) {
		dest[ret++] = from_pos;
	}
	const char *self_str = _buffer(self);
	for (size_t i = from_pos, state = 0; ret < n && i < self_len; i++) {
		state = matcher->delta[(size_t)self_str[i]][state];
		if (state == patlen) {
			assert (i + 1 >= patlen );
			dest[ret++] = i + 1 - patlen;
		}
	}
	fsm_free(matcher);
	return ret;
}


size_t strbuf_replace_n(strbuf *self, const char *old_str, const char *new_str,
                        size_t n, size_t from)
{
	size_t self_len = strbuf_len(self);
	if (from > self_len) return 0;
	size_t patlen = strlen(old_str);
	size_t repllen = strlen(new_str);
	fsm *matcher = build_fsm(old_str, patlen);
	stack *occ = stack_new(sizeof(size_t));
	uint occ_count = 0;
	if (patlen == 0 && occ_count < n ) {
		stack_push_size_t(occ, 0);
		occ_count++;
	}
	const char *self_str = _buffer(self);
	for (size_t i = from, state = 0; occ_count < n && i < self_len; i++) {
		state = matcher->delta[(size_t)self_str[i]][state];
		if (state == patlen) {
			assert (i + 1 >= patlen );
			stack_push_size_t(occ, i + 1 - patlen );
			occ_count++;
		}
	}
	while (!stack_empty(occ)) {
		size_t pos = stack_pop_size_t(occ);
		if (repllen > patlen) {
			strbuf_paste(self, pos, new_str, patlen);
			strbuf_ins(self, pos + patlen, new_str + patlen, repllen - patlen);
		} else if (repllen < patlen) {
			strbuf_cut(self, pos, patlen - repllen, NULL);
			strbuf_paste(self, pos, new_str, repllen);
		} else { // equals
			strbuf_paste(self, pos, new_str, patlen);
		}
	}
	DESTROY_FLAT(occ, stack);
	fsm_free(matcher);
	return occ_count;
}


size_t strbuf_replace(strbuf *self, const char *old_str, const char *new_str,
                      size_t from)
{
	return strbuf_replace_n(self, old_str, new_str, 1, from);
}


size_t strbuf_replace_all(strbuf *self, const char *old_str,
                          const char *new_str,
                          size_t from)
{
	return strbuf_replace_n(self, old_str, new_str, SIZE_MAX, from);
}


void strbuf_reverse(strbuf *self)
{
	size_t self_len = strbuf_len(self);
	char *self_str = _buffer(self);
	for (size_t i = 0, j = self_len - 1; i < j; i++, j--) {
		char tmp = self_str[i];
		self_str[i] = self_str[j];
		self_str[j] = tmp;
	}
}


int sbprintf(strbuf *self, const char *fmt, ...)
{
	size_t fmt_len = strlen(fmt);
	size_t self_len = strbuf_len(self);
	size_t self_capacity = strbuf_capacity(self);
	size_t avail = self_capacity - self_len;
	if (avail < fmt_len) {
		_resize_to(self, self_len + (2 * fmt_len));
		self_capacity = strbuf_capacity(self);
		avail = self_capacity - self_len;
	}
	char *self_str = _buffer(self);
	char *dest = self_str + self_len;
	va_list valist;
	va_start(valist, fmt);
	int written = vsnprintf(dest, avail, fmt, valist);
	va_end(valist);
	if (written >= avail) {
		_resize_to(self, self_len + (written + 1));
		self_capacity = strbuf_capacity(self);
		avail = self_capacity - self_len;
		self_str = _buffer(self);
		dest = self_str + self_len;
		va_start(valist, fmt);
		written = vsnprintf(dest, avail, fmt, valist);
		va_end(valist);
		assert(written < avail);
	}
	self_len += written;
	_set_len(self, self_len);
	return written;
}
