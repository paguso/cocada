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

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "arrays.h"
#include "bitarr.h"
#include "bitbyte.h"
#include "bitvec.h"
#include "cstrutil.h"
#include "format.h"
#include "mathutil.h"
#include "memdbg.h"
#include "new.h"


static const float GROW_BY = 1.5f;
static const size_t MIN_BYTE_SIZE = 2; // GROW_BY*MIN_BYTE_SIZE has to be >1
static const byte_t BYTE_MSB = 0x80;


struct _bitvec {
	byte_t *bits;
	size_t  len;
	size_t  cap;
	size_t  byte_cap;
};


bitvec *bitvec_new()
{
	return bitvec_new_with_capacity(1);
}


bitvec *bitvec_new_with_capacity(size_t capacity)
{
	bitvec *bv = NEW(bitvec);
	bv->len = 0;
	bv->byte_cap = MAX(MIN_BYTE_SIZE, (size_t)DIVCEIL(capacity, BYTESIZE));
	bv->cap = bv->byte_cap*BYTESIZE;
	bv->bits = malloc(bv->byte_cap);
	memset(bv->bits, 0x00, bv->byte_cap); //(!) unused positions must be 0
	return bv;
}


bitvec *bitvec_new_from_bitarr(const byte_t *src, size_t len)
{
	bitvec *bv = bitvec_new_with_capacity(len);
	memcpy(bv->bits, src, DIVCEIL(len, BYTESIZE));
	bv->len = len;
	return bv;
}


void bitvec_finalise(void *ptr, const finaliser *fnr)
{
	FREE(((bitvec *)ptr)->bits);
}


void bitvec_free(bitvec *bv)
{
	FREE(bv->bits);
	FREE(bv);
}


bitvec *bitvec_cropped_clone(const bitvec *src, size_t nbits)
{
	bitvec *bv = bitvec_new_with_capacity(nbits);
	size_t nbytes = (size_t) DIVCEIL(nbits, BYTESIZE);
	memcpy(bv->bits, src->bits, nbytes);
	bv->len = nbits;
	for (size_t i=bv->len; i<nbytes*BYTESIZE; i++)
		bitvec_set_bit(bv, i, 0);
	return bv;
}


bitvec *bitvec_clone(const bitvec *src)
{
	return bitvec_cropped_clone(src, src->len);
}


size_t bitvec_len(const bitvec *bv)
{
	return bv->len;
}


const byte_t *bitvec_as_bytes(const bitvec *bv)
{
	return bv->bits;
}


inline bool bitvec_get_bit (const bitvec *bv, size_t pos)
{
	return bitarr_get_bit(bv->bits, pos);
}


inline void bitvec_set_bit (bitvec *bv, size_t pos, bool bit)
{
	bitarr_set_bit(bv->bits, pos, bit);
}


static inline size_t _bitvec_count1(const bitvec *bv)
{
	size_t byte_pos = 0;
	size_t used_bytes = (size_t)DIVCEIL(bv->len, BYTESIZE);
	size_t cnt = 0;
#if BYTEWORDSIZE==8
	while (byte_pos+8 < used_bytes) {
		cnt += uint64_bitcount1(*((uint64_t *)(bv->bits+byte_pos)));
		byte_pos += 8;
	}
	while (byte_pos+4 < used_bytes) {
		cnt += uint32_bitcount1(*((uint32_t *)(bv->bits+byte_pos)));
		byte_pos += 4;
	}
	while (byte_pos+2 < used_bytes) {
		cnt += uint16_bitcount1(*((uint16_t *)(bv->bits+byte_pos)));
		byte_pos += 2;
	}
#elif BYTEWORDSIZE==4
	while (byte_pos+4 < used_bytes) {
		cnt += uint32_bitcount1(*((uint32_t *)(bv->bits+byte_pos)));
		byte_pos += 4;
	}
	while (byte_pos+2 < used_bytes) {
		cnt += uint16_bitcount1(*((uint16_t *)(bv->bits+byte_pos)));
		byte_pos += 2;
	}
#endif
	while (byte_pos < used_bytes) {
		cnt += byte_bitcount1(bv->bits[byte_pos]);
		byte_pos++;
	}
	return cnt;
}


static inline size_t _bitvec_count0(const bitvec *bv)
{
	return bv->len - _bitvec_count1(bv);
}

/*
static inline size_t __bitvec_count1(bitvector *bv) {
    return bv->count1;
}

static inline size_t __bitvec_count0(bitvector *bv) {
    return bv->len - bv->count1;
}
*/

typedef size_t (*_bv_cnt_func)(const bitvec *);


static _bv_cnt_func _bitvec_count_func[2] = {_bitvec_count0, _bitvec_count1};


size_t bitvec_count(const bitvec *bv, bool bit)
{
	return _bitvec_count_func[bit](bv);
}


static void _growto(bitvec *bv, size_t new_byte_cap)
{
	bv->bits = realloc(bv->bits, new_byte_cap);
	memset(bv->bits+bv->byte_cap, 0x00, (new_byte_cap - bv->byte_cap));
	bv->byte_cap = new_byte_cap;
	bv->cap = bv->byte_cap*BYTESIZE;
}


void bitvec_push (bitvec *bv, bool bit)
{
	//bv->bits[bv->len/BYTESIZE] ^= ( ((-bit)^(bv->bits[bv->len/BYTESIZE]))
	//                                 & (BYTE_MSB>>(bv->len % BYTESIZE)) );
	bitarr_set_bit(bv->bits, bv->len, bit);
	if ((++bv->len)==bv->cap)
		_growto(bv, GROW_BY*bv->byte_cap);
}


void bitvec_push_n (bitvec *bv, size_t nbits, bool bit)
{
	if (bv->len+nbits >= bv->cap) { // HAS to be >=, not >
		_growto(bv, GROW_BY*(size_t)(DIVCEIL(bv->len+nbits, BYTESIZE)));
	}
	if (bit) {
		size_t nleft=nbits;
		byte_t nxt_bit = bv->len % BYTESIZE;
		byte_t *last_byte = bv->bits + (bv->len/BYTESIZE);
		size_t m = MIN(nleft, BYTESIZE-nxt_bit);
		if (m==BYTESIZE)
			*(last_byte) = BYTE_MAX;
		else
			*(last_byte) |= (~(BYTE_MAX<<m)) << (BYTESIZE - nxt_bit - m);
		last_byte += ((nxt_bit+m)/BYTESIZE);
		nxt_bit = ((nxt_bit+m)%BYTESIZE);
		nleft -= m;
		m = nleft/BYTESIZE;
		memset(last_byte, 0xFF, m);
		last_byte += m;
		nleft -= (m*BYTESIZE);
		*(last_byte) |= ~(BYTE_MAX>>nleft);
		bv->len += nbits;
		//bv->count1 += nbits;
	}
	else
		bv->len += nbits;

}

void bitvec_cat (bitvec *bv, const bitvec *src)
{
	size_t nbits = bitvec_len(src);
	if (bv->len+nbits >= bv->cap) { // HAS to be >=, not >
		_growto(bv, GROW_BY*(size_t)(DIVCEIL(bv->len+nbits, BYTESIZE)));
	}
	bitarr_write(bv->bits, bv->len, src->bits, 0, nbits);
	bv->len += nbits;
}


void bitvec_fit(bitvec *bv)
{
	bv->byte_cap = MAX(1, (size_t)DIVCEIL(bv->len, BYTESIZE));
	bv->cap = bv->byte_cap*BYTESIZE;
	bv->bits = realloc(bv->bits, bv->byte_cap);
}


byte_t *bitvec_detach (bitvec *bv)
{
	byte_t *ret = (byte_t *)bv->bits;
	ret = realloc(ret, (size_t)DIVCEIL(bv->len, BYTESIZE));
	FREE(bv);
	return ret;
}


void bitvec_to_string (const bitvec *bv, strbuf *dest, size_t bytes_per_line)
{
	int line_label_width = (bv->len>1)?ceil(log10(bv->len)):1;
	char *lbl = cstr_new(line_label_width);
	size_t bits_per_line = bytes_per_line * BYTESIZE;
	for (size_t i=0; i<bv->len; i++ )  {
		if ( i % bits_per_line == 0) {
			if (i) strbuf_append_char(dest, '\n');
			strbuf_append_char(dest, '[');
			sprintf(lbl, "%*zu", line_label_width, i);
			strbuf_nappend(dest, lbl, strlen(lbl));
			strbuf_append_char(dest, ':');
			sprintf(lbl, "%*zu", line_label_width, MAX(bv->len, i+bits_per_line));
			strbuf_nappend(dest, lbl, strlen(lbl));
			strbuf_append_char(dest, ']');
		}
		if ( i % BYTESIZE == 0 )
			strbuf_append_char(dest, ' ');
		strbuf_append_char(dest, bitvec_get_bit(bv,i) ? '1' : '0' );
	}
}



void bitvec_print(FILE *stream, const bitvec *bv, size_t bytes_per_row)
{
	fprintf(stream, "bitvector@%p {\n", bv);
	fprintf(stream, "  len     : %zu\n", bv->len);
	fprintf(stream, "  capacity: %zu\n", bv->cap);
	fprintf(stream, "  data:\n");
	bitarr_fprint(stream, bv->bits, bv->cap, bytes_per_row, 2);
	fprintf(stream, "}\n");

}


struct _bitvec_format {
	format _t_format;
	bitvec *src;
	uint bytes_per_row;
};

#define BITVEC_PRINT(TYPE)\
	int ret = 0;\
	bitvec_format bf = *((bitvec_format *)self->impltor);\
	ret += TYPE##printf(out, "bitvector@%p {\n", bf.src);\
	ret += TYPE##printf(out, "  len     : %zu\n", bf.src->len);\
	ret += TYPE##printf(out, "  capacity: %zu\n", bf.src->cap);\
	ret += TYPE##printf(out, "  data:\n");\
	ret += bitarr_##TYPE##print(out, bf.src->bits, bf.src->cap, bf.bytes_per_row, 2);\
	ret += TYPE##printf(out, "}\n");\
	return ret;


static int bitvec_format_fprint(format *self, FILE *out)
{
	BITVEC_PRINT(f)
}
static int bitvec_format_sprint(format *self, char *out)
{
	BITVEC_PRINT(s)
}
static int bitvec_format_sbprint(format *self, strbuf *out)
{
	BITVEC_PRINT(sb)
}

format_vt bitvec_format_vt = {.fprint = bitvec_format_fprint,
                              .sprint = bitvec_format_sprint,
                              .sbprint = bitvec_format_sbprint
                             };


bitvec_format *bitvec_get_format(bitvec *self, uint bytes_per_row)
{
	bitvec_format *ret = NEW(bitvec_format);
	ret->_t_format = (format) {
		.vt = bitvec_format_vt, .impltor = ret
	};
	ret->src = self;
	ret->bytes_per_row = MAX(1, bytes_per_row);
	return ret;
}


void bitvec_format_free(bitvec_format *self)
{
	FREE(self);
}

IMPL_TRAIT(bitvec_format, format);