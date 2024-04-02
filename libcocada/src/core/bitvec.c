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
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "bitarr.h"
#include "bitbyte.h"
#include "bitvec.h"
#include "cstrutil.h"
#include "format.h"
#include "mathutil.h"
#include "new.h"


static const float GROW_BY = 1.61803398875f;
static const size_t MIN_CAP = BYTESIZE; // Must be a multiple of BYTESIZE

#define NBYTES(NBITS) ((size_t)DIVCEIL(NBITS, BYTESIZE))

struct _BitVec {
	byte_t *bits;
	size_t  len;
	size_t  cap;
};


size_t bitvec_memsize(BitVec *bv)
{
	return sizeof(struct _BitVec) + NBYTES(bv->cap);
}


BitVec *bitvec_new()
{
	return bitvec_new_with_capacity(1);
}


BitVec *bitvec_new_with_len(size_t len)
{
	BitVec *ret = bitvec_new_with_capacity(len);
	ret->len = len;
	return ret;
}



BitVec *bitvec_new_with_capacity(size_t capacity)
{
	BitVec *bv = NEW(BitVec);
	bv->len = 0;
	bv->cap = MAX(MIN_CAP, NBYTES(capacity) * BYTESIZE);
	size_t byte_cap = bv->cap / BYTESIZE;
	bv->bits = malloc(byte_cap);
	memset(bv->bits, 0, byte_cap); //(!) unused positions must be 0
	return bv;
}


BitVec *bitvec_new_from_bitarr(const byte_t *src, size_t len)
{
	BitVec *bv = bitvec_new_with_capacity(len);
	memcpy(bv->bits, src, NBYTES(len));
	bv->len = len;
	return bv;
}


void bitvec_finalise(void *ptr, const Finaliser *fnr)
{
	FREE(((BitVec *)ptr)->bits);
}


void bitvec_free(BitVec *bv)
{
	FREE(bv->bits);
	FREE(bv);
}


BitVec *bitvec_clone(const BitVec *src)
{
	return bitvec_cropped_clone(src, src->len);
}


BitVec *bitvec_cropped_clone(const BitVec *src, size_t nbits)
{
	BitVec *bv = bitvec_new_with_capacity(nbits);
	size_t nbytes = NBYTES(nbits);
	memcpy(bv->bits, src->bits, nbytes);
	bv->len = nbits;
	for (size_t i = bv->len; i < nbytes * BYTESIZE; i++)
		bitvec_set_bit(bv, i, 0);
	return bv;
}


void bitvec_fit(BitVec *bv)
{
	size_t byte_cap = MAX(MIN_CAP / BYTESIZE, NBYTES(bv->len));
	bv->cap = byte_cap * BYTESIZE;
	bv->bits = realloc(bv->bits, byte_cap);
}


const byte_t *bitvec_as_bytes(const BitVec *bv)
{
	return bv->bits;
}


byte_t *bitvec_detach(BitVec *bv)
{
	byte_t *ret = (byte_t *)bv->bits;
	FREE(bv);
	return ret;
}


size_t bitvec_len(const BitVec *bv)
{
	return bv->len;
}


inline bool bitvec_get_bit(const BitVec *bv, size_t pos)
{
	return bitarr_get_bit(bv->bits, pos);
}


static inline size_t _bitvec_count1(const BitVec *bv, size_t from, size_t to)
{
	if (from >= to) return 0;
	assert(from < to && to <= bv->len);

	size_t byte_pos = from / BYTESIZE;
	size_t last_byte = to / BYTESIZE;

	// if range is within one byte
	if (byte_pos == last_byte) {
		return byte_bitcount1( bv->bits[byte_pos]
		                       & LSBMASK(BYTESIZE - (from % BYTESIZE))
		                       & MSBMASK(to % BYTESIZE) );
	}

	//count bits from first byte
	size_t ret = 0;
	ret += byte_bitcount1(bv->bits[byte_pos] & LSBMASK(BYTESIZE -
	                      (from % BYTESIZE)));
	byte_pos++;

	while (byte_pos + ULLONG_BYTES < last_byte) {
		ret += ullong_bitcount1(*((ullong *)(bv->bits + byte_pos)));
		byte_pos += ULLONG_BYTES;
	}
	while (byte_pos + ULONG_BYTES < last_byte) {
		ret += ulong_bitcount1(*((ulong *)(bv->bits + byte_pos)));
		byte_pos += ULONG_BYTES;
	}
	while (byte_pos + UINT_BYTES < last_byte) {
		ret += uint_bitcount1(*((uint *)(bv->bits + byte_pos)));
		byte_pos += UINT_BYTES;
	}
	while (byte_pos + USHRT_BYTES < last_byte) {
		ret += ushort_bitcount1(*((ushort *)(bv->bits + byte_pos)));
		byte_pos += USHRT_BYTES;
	}
	while (byte_pos < last_byte) {
		ret += byte_bitcount1(bv->bits[byte_pos]);
		byte_pos++;
	}

	// last byte
	ret += byte_bitcount1(bv->bits[last_byte] & MSBMASK(to % BYTESIZE));

	return ret;
}


static inline size_t _bitvec_count0(const BitVec *bv, size_t from, size_t to)
{
	return (to - from) - _bitvec_count1(bv, from, to);
}


typedef size_t (*_bv_cnt_func)(const BitVec *, size_t from, size_t to);


static _bv_cnt_func _bitvec_count_func[2] = {_bitvec_count0, _bitvec_count1};


size_t bitvec_count(const BitVec *bv, bool bit)
{
	return _bitvec_count_func[bit](bv, 0, bv->len);
}


size_t bitvec_count_range(const BitVec *bv, bool bit, size_t from, size_t to)
{
	return _bitvec_count_func[bit](bv, from, to);
}


size_t _bitvec_select1(const BitVec *bv, size_t rank)
{
	byte_t *cur_byte = bv->bits;
	byte_t *last_byte = bv->bits + (bv->len / BYTESIZE);
	size_t count = 0, partial_count = 0;

	ullong *llarr = (ullong *)(bv->bits);
	while ( (byte_t *)(llarr + 1) <= last_byte &&
	        count + (partial_count = ullong_bitcount1(*llarr)) <= rank ) {
		count += partial_count;
		llarr++;
	}
	ulong *larr = (ulong *)llarr;
	while ( (byte_t *)(larr + 1) <= last_byte &&
	        count + (partial_count = ulong_bitcount1(*larr)) <= rank ) {
		count += partial_count;
		larr++;
	}
	uint *intarr = (uint *)larr;
	while ( (byte_t *)(intarr + 1) <= last_byte &&
	        count + (partial_count = uint_bitcount1(*intarr)) <= rank ) {
		count += partial_count;
		intarr++;
	}
	ushort *shrtarr = (ushort *)intarr;
	while ( (byte_t *)(shrtarr + 1) <= last_byte &&
	        count + (partial_count = ushort_bitcount1(*shrtarr)) <= rank ) {
		count += partial_count;
		shrtarr++;
	}
	cur_byte = (byte_t *)shrtarr;

	while ( (cur_byte + 1) <= last_byte &&
	        count + (partial_count = byte_bitcount1(*cur_byte)) <= rank ) {
		count += partial_count;
		cur_byte++;
	}
	// cur_byte is the rightmost byte with rank < desired rank
	// selected position has to be within cur_byte if it exists
	size_t ret = ((size_t)(cur_byte - bv->bits) * BYTESIZE) +
	             byte_select1(*cur_byte, MIN(BYTESIZE, rank - count));

	return MIN(bv->len, ret);
}


size_t _bitvec_select0(const BitVec *bv, size_t rank)
{
	byte_t *cur_byte = bv->bits;
	byte_t *last_byte = bv->bits + (bv->len / BYTESIZE);
	size_t count = 0, partial_count = 0;

	ullong *llarr = (ullong *)(bv->bits);
	while ( (byte_t *)(llarr + 1) <= last_byte &&
	        count + (partial_count = ullong_bitcount0(*llarr)) <= rank ) {
		count += partial_count;
		llarr++;
	}
	ulong *larr = (ulong *)llarr;
	while ( (byte_t *)(larr + 1) <= last_byte &&
	        count + (partial_count = ulong_bitcount0(*larr)) <= rank ) {
		count += partial_count;
		larr++;
	}
	uint *intarr = (uint *)larr;
	while ( (byte_t *)(intarr + 1) <= last_byte &&
	        count + (partial_count = uint_bitcount0(*intarr)) <= rank ) {
		count += partial_count;
		intarr++;
	}
	ushort *shrtarr = (ushort *)intarr;
	while ( (byte_t *)(shrtarr + 1) <= last_byte &&
	        count + (partial_count = ushort_bitcount0(*shrtarr)) <= rank ) {
		count += partial_count;
		shrtarr++;
	}
	cur_byte = (byte_t *)shrtarr;

	while ( (cur_byte + 1) <= last_byte &&
	        count + (partial_count = byte_bitcount0(*cur_byte)) <= rank ) {
		count += partial_count;
		cur_byte++;
	}
	// cur_byte is the rightmost byte with rank < desired rank
	// selected position has to be within cur_byte if it exists
	size_t ret = ((size_t)(cur_byte - bv->bits) * BYTESIZE) +
	             byte_select0(*cur_byte, MIN(BYTESIZE, rank - count));

	return MIN(bv->len, ret);
}

size_t bitvec_select(const BitVec *bv, bool bit, size_t rank)
{
	return bit ? _bitvec_select1(bv, rank) : _bitvec_select0(bv, rank);
}


inline void bitvec_set_bit(BitVec *bv, size_t pos, bool bit)
{
	bitarr_set_bit(bv->bits, pos, bit);
}


static void _growto_bits(BitVec *bv, size_t min_cap)
{
	size_t old_byte_cap = bv->cap / BYTESIZE;
	while (bv->cap < min_cap) {
		bv->cap *= GROW_BY;
	}
	size_t new_byte_cap = NBYTES(bv->cap);
	bv->cap = new_byte_cap * BYTESIZE;
	bv->bits = realloc(bv->bits, new_byte_cap);
	memset(bv->bits + old_byte_cap, 0, (new_byte_cap - old_byte_cap));
}


void bitvec_push(BitVec *bv, bool bit)
{
	if (bv->len == bv->cap) {
		_growto_bits(bv, bv->len + 1);
	}
	bitarr_set_bit(bv->bits, bv->len, bit);
	bv->len++;
}


void bitvec_push_n(BitVec *bv, size_t nbits, bool bit)
{
	if (bv->len + nbits > bv->cap) {
		_growto_bits(bv, bv->len + nbits);
	}
	if (bit) {
		size_t nleft = nbits;
		byte_t nxt_bit = bv->len % BYTESIZE;
		byte_t *last_byte = bv->bits + (bv->len / BYTESIZE);
		size_t m = MIN(nleft, BYTESIZE - nxt_bit);
		if (m == BYTESIZE)
			*(last_byte) = BYTE_MAX;
		else
			*(last_byte) |= (~(BYTE_MAX << m)) << (BYTESIZE - nxt_bit - m);
		last_byte += ((nxt_bit + m) / BYTESIZE);
		nxt_bit = ((nxt_bit + m) % BYTESIZE);
		nleft -= m;
		m = nleft / BYTESIZE;
		memset(last_byte, 0xFF, m);
		last_byte += m;
		nleft -= (m * BYTESIZE);
		*(last_byte) |= ~(BYTE_MAX >> nleft);
		bv->len += nbits;
		//bv->count1 += nbits;
	}
	else
		bv->len += nbits;

}

void bitvec_cat (BitVec *bv, const BitVec *src)
{
	if (bv->len + src->len > bv->cap) {
		_growto_bits(bv, bv->len + src->len);
	}
	bitarr_write(bv->bits, bv->len, src->bits, 0, src->len);
	bv->len += src->len;
}


void bitvec_to_string (const BitVec *bv, StrBuf *dest, size_t bytes_per_line)
{
	int line_label_width = (bv->len > 1) ? ceil(log10(bv->len)) : 1;
	char *lbl = cstr_new(line_label_width);
	size_t bits_per_line = bytes_per_line * BYTESIZE;
	for (size_t i = 0; i < bv->len; i++ )  {
		if ( i % bits_per_line == 0) {
			if (i) strbuf_append_char(dest, '\n');
			strbuf_append_char(dest, '[');
			sprintf(lbl, "%*zu", line_label_width, i);
			strbuf_nappend(dest, lbl, strlen(lbl));
			strbuf_append_char(dest, ':');
			sprintf(lbl, "%*zu", line_label_width, MAX(bv->len, i + bits_per_line));
			strbuf_nappend(dest, lbl, strlen(lbl));
			strbuf_append_char(dest, ']');
		}
		if ( i % BYTESIZE == 0 )
			strbuf_append_char(dest, ' ');
		strbuf_append_char(dest, bitvec_get_bit(bv, i) ? '1' : '0' );
	}
}



void bitvec_print(FILE *stream, const BitVec *bv, size_t bytes_per_row)
{
	fprintf(stream, "bitvector@%p {\n", bv);
	fprintf(stream, "  len     : %zu\n", bv->len);
	fprintf(stream, "  capacity: %zu\n", bv->cap);
	fprintf(stream, "  data:\n");
	bitarr_fprint(stream, bv->bits, bv->cap, bytes_per_row, 2);
	fprintf(stream, "}\n");

}

/* ------------------------ bitvec_format trait  -------------------------- */

struct _BitVecFormat {
	Format _t_Format;
	BitVec *src;
	uint bytes_per_row;
};


#define BITVEC_PRINT(TYPE)\
	int ret = 0;\
	BitVecFormat bf = *((BitVecFormat *)self->impltor);\
	ret += TYPE##printf(out, "bitvector@%p {\n", bf.src);\
	ret += TYPE##printf(out, "  len     : %zu\n", bf.src->len);\
	ret += TYPE##printf(out, "  capacity: %zu\n", bf.src->cap);\
	ret += TYPE##printf(out, "  data:\n");\
	ret += bitarr_##TYPE##print(out, bf.src->bits, bf.src->cap, bf.bytes_per_row, 2);\
	ret += TYPE##printf(out, "}\n");\
	return ret;


static int bitvec_format_fprint(Format *self, FILE *out)
{
	BITVEC_PRINT(f)
}


static int bitvec_format_sprint(Format *self, char *out)
{
	BITVEC_PRINT(s)
}


static int bitvec_format_sbprint(Format *self, StrBuf *out)
{
	BITVEC_PRINT(sb)
}


format_vt bitvec_format_vt = {.fprint = bitvec_format_fprint,
                              .sprint = bitvec_format_sprint,
                              .sbprint = bitvec_format_sbprint
                             };


BitVecFormat *bitvec_get_format(BitVec *self, uint bytes_per_row)
{
	BitVecFormat *ret = NEW(BitVecFormat);
	ret->_t_Format = (Format) {
		.vt = bitvec_format_vt, .impltor = ret
	};
	ret->src = self;
	ret->bytes_per_row = MAX(1, bytes_per_row);
	return ret;
}


void bitvec_format_free(BitVecFormat *self)
{
	FREE(self);
}


IMPL_TRAIT(BitVecFormat, Format);
