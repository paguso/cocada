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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alphabet.h"
#include "arrays.h"
#include "binheap.h"
#include "bitbyte.h"
#include "bitarr.h"
#include "bitvec.h"
#include "bytearr.h"
#include "new.h"
#include "cstrutil.h"
#include "strbuf.h"
#include "huffcode.h"
#include "mathutil.h"
#include "strread.h"
#include "xstr.h"
#include "xstrread.h"

static const byte_t LEFT  = 0;
static const byte_t RIGHT = 1;

struct _hufftnode {
	size_t     chr_rank;
	hufftnode *chd[2];
	byte_t    *ab_mask;
};


struct _huffcode {
	alphabet  *ab;
	size_t     size;
	hufftnode *tree;
	bitvec   **code;
};


typedef struct {
	size_t freq; // must come first
	size_t node;
} nodefreq;


static int nodefreq_cmp(const void *p1, const void *p2)
{
	return (*((size_t *)p2) - *((size_t *)p1));
}



static void fill_code_table( huffcode *hcode, const hufftnode *node,
                             size_t code_len, byte_t *code )
{
	if (hufftnode_is_leaf(node)) {
		hcode->code[node->chr_rank] = bitvec_new_from_bitarr(code, code_len);
		//printf("code of %c = %s\n",ab_char(hcode->ab, node->chr_rank), code);
	}
	else {
		bitarr_set_bit(code, code_len, 0);
		fill_code_table(hcode, hufftnode_left(node), code_len+1, code);
		bitarr_set_bit(code, code_len, 1);
		fill_code_table(hcode, hufftnode_right(node), code_len+1, code);
	}
}


huffcode *huffcode_new(const alphabet *ab, const size_t freqs[])
{
	huffcode *hcode;

	hcode = NEW(huffcode);
	hcode->ab = alphabet_clone(ab);
	hcode->size = ab_size(ab);

	size_t ab_bytesize = (size_t)DIVCEIL(hcode->size, BYTESIZE);
	hcode->tree = NEW_ARR(hufftnode, MAX(0, 2 * hcode->size - 1));
	for (size_t i = 0; i < hcode->size; i++) {
		hcode->tree[i].chr_rank = i;
		hcode->tree[i].chd[LEFT]     = &hcode->tree[i];
		hcode->tree[i].chd[RIGHT]    = &hcode->tree[i];
		hcode->tree[i].ab_mask  = bytearr_new(ab_bytesize);
		bitarr_set_bit(hcode->tree[i].ab_mask, i, 1);
	}

	binheap *nfheap = binheap_new(nodefreq_cmp, sizeof(nodefreq));
	for (size_t i=0; i<hcode->size; i++) {
		nodefreq nf = {.node =i, .freq=freqs[i]};
		binheap_push(nfheap, &nf);
	}
	size_t next = hcode->size;
	while (binheap_size(nfheap)>1) {
		nodefreq smallest, snd_smallest, new_nf;
		binheap_pop(nfheap, &smallest);
		binheap_pop(nfheap, &snd_smallest);
		new_nf.node = next;
		new_nf.freq = smallest.freq + snd_smallest.freq;
		binheap_push(nfheap, &new_nf);
		hcode->tree[next].chd[LEFT]  = &hcode->tree[smallest.node];
		hcode->tree[next].chd[RIGHT] = &hcode->tree[snd_smallest.node];
		hcode->tree[next].ab_mask = bytearr_new(ab_bytesize);
		hcode->tree[next].chr_rank = hcode->size;
		bitarr_or( hcode->tree[next].ab_mask,
		           hcode->tree[smallest.node].ab_mask, hcode->size );
		bitarr_or( hcode->tree[next].ab_mask,
		           hcode->tree[snd_smallest.node].ab_mask, hcode->size );
		next++;
	}
	//assert(next==(2*hcode->size-1));

	hcode->code = NEW_ARR(bitvec *, hcode->size);
	if (hcode->size) {
		byte_t *chrcode = bitarr_new(hcode->size);
		fill_code_table(hcode, huffcode_tree(hcode), 0, chrcode);
		FREE(chrcode);
	}
	return hcode;
}


huffcode *huffcode_new_from_str(const alphabet *ab, const char *src)
{
	size_t *counts =  NEW_ARR_0(size_t, ab_size(ab));
	FOREACH_IN_CSTR(c, src) {
		counts[ab_rank(ab, c)]++;
	}
	huffcode *hc = huffcode_new(ab, counts);
	FREE(counts);
	return hc;
}


huffcode *huffcode_new_from_strread(const alphabet *ab, strread *reader)
{
	size_t *counts =  NEW_ARR_0(size_t, ab_size(ab));
	for (int c; (c=strread_getc(reader)) != EOF;) {
		counts[ab_rank(ab, c)]++;
	}
	huffcode *hc = huffcode_new(ab, counts);
	FREE(counts);
	return hc;
}


huffcode *huffcode_new_from_xstr(const alphabet *ab, const xstr *src)
{
	size_t *counts =  NEW_ARR_0(size_t, ab_size(ab));
	FOREACH_IN_XSTR(c, src) {
		counts[ab_rank(ab, c)]++ ;
	}
	huffcode *hc = huffcode_new(ab, counts);
	FREE(counts);
	return hc;
}


huffcode *huffcode_new_from_xstrread(const alphabet *ab, xstrread *reader)
{
	size_t *counts =  NEW_ARR_0(size_t, ab_size(ab));
	for (xchar_wt c; (c=xstrread_getc(reader)) != XEOF;) {
		counts[ab_rank(ab, c)]++;
	}
	huffcode *hc = huffcode_new(ab, counts);
	FREE(counts);
	return hc;
}


void huffcode_free(huffcode *hcode)
{
	if (hcode==NULL) return;
	alphabet_free(hcode->ab);
	for (size_t i=0; i<hcode->size; i++) {
		//FREE(hcode->code[i], bitvec); // no null codes
		bitvec_free(hcode->code[i]);
	}
	FREE(hcode->code);
	for (size_t i=0; hcode->size>0 && i<(2*hcode->size)-1; i++) {
		if (hcode->tree[i].ab_mask)
			FREE(hcode->tree[i].ab_mask);
	}
	FREE(hcode->tree);
	FREE(hcode);
}


static void _print_htree( FILE *stream, const huffcode *hc,
                          const hufftnode *node, size_t level, const char *code )
{
	if (node==NULL) return;
	char *space = cstr_new(4*level);
	cstr_fill(space, 0, 4*level, ' ');
	if (hufftnode_is_leaf(node)) {
		fprintf(stream,"%s[%p code=%s chr=%c(%d)]\n", space, node, code,
		        ab_char(hc->ab, node->chr_rank), (int)(ab_char(hc->ab, node->chr_rank)));
		//bytearr_print(hufftnode_ab_mask(node), (size_t)mult_ceil(ab_size(hc->ab), BYTESIZE), 4, space);
	}
	else {
		fprintf(stream, "%s[%p code=%s]\n", space, node, code);
		//bytearr_print(hufftnode_ab_mask(node), (size_t)mult_ceil(ab_size(hc->ab), BYTESIZE), 4, space);
		char *ccode = cstr_new(level+1);
		strcpy(ccode, code);
		ccode[level] = '0';
		_print_htree(stream, hc, hufftnode_left(node), level+1, ccode);
		ccode[level] = '1';
		_print_htree(stream, hc, hufftnode_right(node), level+1, ccode);
	}
	FREE(space);
}


void huffcode_print(FILE *stream, const huffcode *hcode)
{
	fprintf(stream, "huffcode@%p {\n",(void *)hcode);
	fprintf(stream, "    size: %zu\n",hcode->size);
	_print_htree(stream, hcode, huffcode_tree(hcode), 0, "");
	fprintf(stream, "    codes:\n");
	switch ( ab_type(hcode->ab) ) {
	case CHAR_TYPE:
		for (size_t i=0; i<hcode->size; i++) {
			fprintf(stream, "%c: ", ab_char(hcode->ab, i) );
			bitvec_print(stream, hcode->code[i], 8);
		}
		break;
	case INT_TYPE:
		for (size_t i=0; i<hcode->size; i++) {
			fprintf(stream,"%"XCHAR_FMT": ", ab_char(hcode->ab, i) );
			bitvec_print(stream, hcode->code[i], 8);
		}
		break;
	}
	fprintf(stream, "} // end of huffcode@%p\n", (void *)hcode);
}


bitvec *huffcode_encode(const char *src, size_t len, const huffcode *hcode)
{
	bitvec *enc = bitvec_new();
	huffcode_encode_to(enc, src, len, hcode);
	return enc;
}


void huffcode_encode_to(bitvec *dest, const char *src, size_t len,
                        const huffcode *hcode)
{
	for (int i=0; i<len; i++) {
		bitvec_cat(dest, hcode->code[ab_rank(hcode->ab, src[i])]);
	}
}


bitvec *huffcode_encode_xstr(const xstr *src, const huffcode *hcode)
{
	bitvec *enc = bitvec_new();
	huffcode_encode_xstr_to(enc, src, hcode);
	return enc;
}


void huffcode_encode_xstr_to(bitvec *dest, const xstr *src,
                             const huffcode *hcode)
{
	FOREACH_IN_XSTR(c, src) {
		bitvec_cat(dest, hcode->code[ab_rank(hcode->ab, c)]);
	}
}


bitvec *huffcode_encode_strread(strread *src, const huffcode *hcode)
{
	bitvec *enc = bitvec_new();
	huffcode_encode_strread_to(enc, src, hcode);
	return enc;
}


void huffcode_encode_strread_to(bitvec *dest, strread *src,
                                const huffcode *hcode)
{
	for (int c; (c = strread_getc(src)) != EOF;) {
		bitvec_cat(dest, hcode->code[ab_rank(hcode->ab, c)]);
	}
}


bitvec *huffcode_encode_xstrread(xstrread *src, const huffcode *hcode)
{
	bitvec *enc = bitvec_new();
	huffcode_encode_xstrread_to(enc, src, hcode);
	return enc;
}


void huffcode_encode_xstrread_to(bitvec *dest, xstrread *src,
                                 const huffcode *hcode)
{
	for (xchar_wt c; (c = xstrread_getc(src)) != XEOF;) {
		bitvec_cat(dest, hcode->code[ab_rank(hcode->ab, c)]);
	}
}


xstr *huffcode_decode(const bitvec *bcode, const huffcode *hcode)
{
	xstr *dec = xstr_new(nbytes(ab_size(hcode->ab)));
	hufftnode *cur = (hufftnode *) huffcode_tree(hcode);
	for (size_t i=0, l=bitvec_len(bcode); i<l; i++) {
		cur = cur->chd[bitvec_get_bit(bcode, i)];
		if (hufftnode_is_leaf(cur)) {
			xstr_push(dec, ab_char(hcode->ab, cur->chr_rank));
			cur = (hufftnode *) huffcode_tree(hcode);
		}
	}
	return dec;
}


const bitvec *huffcode_charcode(const huffcode *hcode, size_t char_rank)
{
	return hcode->code[char_rank];
}


const hufftnode *huffcode_tree(const huffcode *code)
{
	return (code->size>0)?code->tree+(2*code->size)-2:NULL;
}


const alphabet *huffcode_ab(const huffcode *code)
{
	return code->ab;
}


bool hufftnode_is_leaf(const hufftnode *node)
{
	return node->chd[LEFT]==node->chd[RIGHT];
}


const hufftnode *hufftnode_left(const hufftnode *node)
{
	return node->chd[LEFT];
}


const hufftnode *hufftnode_right(const hufftnode *node)
{
	return node->chd[RIGHT];
}


const byte_t *hufftnode_ab_mask(const hufftnode *node)
{
	return node->ab_mask;
}


size_t hufftnode_char_rank(const hufftnode *node)
{
	return node->chr_rank;
}
