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
#include "read.h"
#include "xstr.h"
#include "xstrread.h"

static const byte LEFT = 0;
static const byte RIGHT = 1;

struct _hufftnode {
	size_t chr_rank;
	hufftnode *chd[2];
	byte *ab_mask;
};

struct _huffcode {
	Alphabet *ab;
	size_t size;
	hufftnode *tree;
	BitVec **code;
};

typedef struct {
	size_t freq; // must come first
	size_t node;
} nodefreq;

static int nodefreq_cmp(const void *p1, const void *p2)
{
	return (*((size_t *)p2) - * ((size_t *)p1));
}

static void fill_code_table(huffcode *hcode, const hufftnode *node,
                            size_t code_len, byte *code)
{
	if (hufftnode_is_leaf(node)) {
		hcode->code[node->chr_rank] = bitvec_new_from_bitarr(code, code_len);
		// printf("code of %c = %s\n",ab_char(hcode->ab, node->chr_rank), code);
	}
	else {
		bitarr_set_bit(code, code_len, 0);
		fill_code_table(hcode, hufftnode_left(node), code_len + 1, code);
		bitarr_set_bit(code, code_len, 1);
		fill_code_table(hcode, hufftnode_right(node), code_len + 1, code);
	}
}

huffcode *huffcode_new(const Alphabet *ab, const size_t freqs[])
{
	huffcode *hcode;

	hcode = NEW(huffcode);
	hcode->ab = alphabet_clone(ab);
	hcode->size = alphabet_size(ab);

	size_t ab_bytesize = (size_t)DIVCEIL(hcode->size, BYTESIZE);
	hcode->tree = ARR_NEW(hufftnode, MAX(0, 2 * hcode->size - 1));
	for (size_t i = 0; i < hcode->size; i++) {
		hcode->tree[i].chr_rank = i;
		hcode->tree[i].chd[LEFT] = &hcode->tree[i];
		hcode->tree[i].chd[RIGHT] = &hcode->tree[i];
		hcode->tree[i].ab_mask = bytearr_new(ab_bytesize);
		bitarr_set_bit(hcode->tree[i].ab_mask, i, 1);
	}

	BinHeap *nfheap = binheap_new(sizeof(nodefreq), nodefreq_cmp);
	for (size_t i = 0; i < hcode->size; i++) {
		nodefreq nf = {.node = i, .freq = freqs[i]};
		binheap_ins(nfheap, &nf);
	}
	size_t next = hcode->size;
	while (binheap_size(nfheap) > 1) {
		nodefreq smallest, snd_smallest, new_nf;
		binheap_remv(nfheap, &smallest);
		binheap_remv(nfheap, &snd_smallest);
		new_nf.node = next;
		new_nf.freq = smallest.freq + snd_smallest.freq;
		binheap_ins(nfheap, &new_nf);
		hcode->tree[next].chd[LEFT] = &hcode->tree[smallest.node];
		hcode->tree[next].chd[RIGHT] = &hcode->tree[snd_smallest.node];
		hcode->tree[next].ab_mask = bytearr_new(ab_bytesize);
		hcode->tree[next].chr_rank = hcode->size;
		bitarr_or(hcode->tree[next].ab_mask,
		          hcode->tree[smallest.node].ab_mask, hcode->size);
		bitarr_or(hcode->tree[next].ab_mask,
		          hcode->tree[snd_smallest.node].ab_mask, hcode->size);
		next++;
	}
	// assert(next==(2*hcode->size-1));

	hcode->code = ARR_NEW(BitVec *, hcode->size);
	if (hcode->size) {
		byte *chrcode = bitarr_new(hcode->size);
		fill_code_table(hcode, huffcode_tree(hcode), 0, chrcode);
		FREE(chrcode);
	}
	return hcode;
}

huffcode *huffcode_new_from_str(const Alphabet *ab, const char *src)
{
	size_t *counts = ARR_OF_0_NEW(size_t, alphabet_size(ab));
	FOREACH_IN_CSTR(c, src) {
		counts[alphabet_rank(ab, c)]++;
	}
	huffcode *hc = huffcode_new(ab, counts);
	FREE(counts);
	return hc;
}

huffcode *huffcode_new_from_strread(const Alphabet *ab, Read *reader)
{
	size_t *counts = ARR_OF_0_NEW(size_t, alphabet_size(ab));
	for (int c; (c = read_getc(reader)) != EOF;) {
		counts[alphabet_rank(ab, c)]++;
	}
	huffcode *hc = huffcode_new(ab, counts);
	FREE(counts);
	return hc;
}

huffcode *huffcode_new_from_xstr(const Alphabet *ab, const xstr *src)
{
	size_t *counts = ARR_OF_0_NEW(size_t, alphabet_size(ab));
	FOREACH_IN_XSTR(c, src) {
		counts[alphabet_rank(ab, c)]++;
	}
	huffcode *hc = huffcode_new(ab, counts);
	FREE(counts);
	return hc;
}

huffcode *huffcode_new_from_xstrread(const Alphabet *ab, xstrRead *reader)
{
	size_t *counts = ARR_OF_0_NEW(size_t, alphabet_size(ab));
	for (xwchar c; (c = xstrread_getc(reader)) != XEOF;) {
		counts[alphabet_rank(ab, c)]++;
	}
	huffcode *hc = huffcode_new(ab, counts);
	FREE(counts);
	return hc;
}

void huffcode_free(huffcode *hcode)
{
	if (hcode == NULL)
		return;
	alphabet_free(hcode->ab);
	for (size_t i = 0; i < hcode->size; i++) {
		// DESTROY_FLAT(hcode->code[i], bitvec); // no null codes
		bitvec_free(hcode->code[i]);
	}
	FREE(hcode->code);
	for (size_t i = 0; hcode->size > 0 && i < (2 * hcode->size) - 1; i++) {
		if (hcode->tree[i].ab_mask)
			FREE(hcode->tree[i].ab_mask);
	}
	FREE(hcode->tree);
	FREE(hcode);
}

static void _print_htree(FILE *stream, const huffcode *hc,
                         const hufftnode *node, size_t level, const char *code)
{
	if (node == NULL)
		return;
	char *space = cstr_new(4 * level);
	cstr_fill(space, 0, 4 * level, ' ');
	if (hufftnode_is_leaf(node)) {
		fprintf(stream, "%s[%p code=%s chr=%c(%d)]\n", space, node, code,
		        alphabet_char(hc->ab, node->chr_rank), (int)(alphabet_char(hc->ab,
		                node->chr_rank)));
		// bytearr_print(hufftnode_ab_mask(node), (size_t)mult_ceil(ab_size(hc->ab), BYTESIZE), 4, space);
	}
	else {
		fprintf(stream, "%s[%p code=%s]\n", space, node, code);
		// bytearr_print(hufftnode_ab_mask(node), (size_t)mult_ceil(ab_size(hc->ab), BYTESIZE), 4, space);
		char *ccode = cstr_new(level + 1);
		strcpy(ccode, code);
		ccode[level] = '0';
		_print_htree(stream, hc, hufftnode_left(node), level + 1, ccode);
		ccode[level] = '1';
		_print_htree(stream, hc, hufftnode_right(node), level + 1, ccode);
	}
	FREE(space);
}

void huffcode_print(FILE *stream, const huffcode *hcode)
{
	fprintf(stream, "huffcode@%p {\n", (void *)hcode);
	fprintf(stream, "    size: %zu\n", hcode->size);
	_print_htree(stream, hcode, huffcode_tree(hcode), 0, "");
	fprintf(stream, "    codes:\n");
	switch (alphabet_type(hcode->ab)) {
	case CHAR_TYPE:
		for (size_t i = 0; i < hcode->size; i++) {
			fprintf(stream, "%c: ", alphabet_char(hcode->ab, i));
			bitvec_print(stream, hcode->code[i], 8);
		}
		break;
	case INT_TYPE:
		for (size_t i = 0; i < hcode->size; i++) {
			fprintf(stream, "%" XCHAR_FMT ": ", alphabet_char(hcode->ab, i));
			bitvec_print(stream, hcode->code[i], 8);
		}
		break;
	}
	fprintf(stream, "} // end of huffcode@%p\n", (void *)hcode);
}

BitVec *huffcode_encode(const char *src, size_t len, const huffcode *hcode)
{
	BitVec *enc = bitvec_new();
	huffcode_encode_to(enc, src, len, hcode);
	return enc;
}

void huffcode_encode_to(BitVec *dest, const char *src, size_t len,
                        const huffcode *hcode)
{
	for (int i = 0; i < len; i++) {
		bitvec_cat(dest, hcode->code[alphabet_rank(hcode->ab, src[i])]);
	}
}

BitVec *huffcode_encode_xstr(const xstr *src, const huffcode *hcode)
{
	BitVec *enc = bitvec_new();
	huffcode_encode_xstr_to(enc, src, hcode);
	return enc;
}

void huffcode_encode_xstr_to(BitVec *dest, const xstr *src,
                             const huffcode *hcode)
{
	FOREACH_IN_XSTR(c, src) {
		bitvec_cat(dest, hcode->code[alphabet_rank(hcode->ab, c)]);
	}
}

BitVec *huffcode_encode_strread(Read *src, const huffcode *hcode)
{
	BitVec *enc = bitvec_new();
	huffcode_encode_strread_to(enc, src, hcode);
	return enc;
}

void huffcode_encode_strread_to(BitVec *dest, Read *src,
                                const huffcode *hcode)
{
	for (int c; (c = read_getc(src)) != EOF;) {
		bitvec_cat(dest, hcode->code[alphabet_rank(hcode->ab, c)]);
	}
}

BitVec *huffcode_encode_xstrread(xstrRead *src, const huffcode *hcode)
{
	BitVec *enc = bitvec_new();
	huffcode_encode_xstrread_to(enc, src, hcode);
	return enc;
}

void huffcode_encode_xstrread_to(BitVec *dest, xstrRead *src,
                                 const huffcode *hcode)
{
	for (xwchar c; (c = xstrread_getc(src)) != XEOF;) {
		bitvec_cat(dest, hcode->code[alphabet_rank(hcode->ab, c)]);
	}
}

xstr *huffcode_decode(const BitVec *bcode, const huffcode *hcode)
{
	xstr *dec = xstr_new(nbytes(alphabet_size(hcode->ab)));
	hufftnode *cur = (hufftnode *)huffcode_tree(hcode);
	for (size_t i = 0, l = bitvec_len(bcode); i < l; i++) {
		cur = cur->chd[bitvec_get_bit(bcode, i)];
		if (hufftnode_is_leaf(cur)) {
			xstr_push(dec, alphabet_char(hcode->ab, cur->chr_rank));
			cur = (hufftnode *)huffcode_tree(hcode);
		}
	}
	return dec;
}

const BitVec *huffcode_charcode(const huffcode *hcode, size_t char_rank)
{
	return hcode->code[char_rank];
}

const hufftnode *huffcode_tree(const huffcode *code)
{
	return (code->size > 0) ? code->tree + (2 * code->size) - 2 : NULL;
}

const Alphabet *huffcode_ab(const huffcode *code)
{
	return code->ab;
}

bool hufftnode_is_leaf(const hufftnode *node)
{
	return node->chd[LEFT] == node->chd[RIGHT];
}

const hufftnode *hufftnode_left(const hufftnode *node)
{
	return node->chd[LEFT];
}

const hufftnode *hufftnode_right(const hufftnode *node)
{
	return node->chd[RIGHT];
}

const byte *hufftnode_ab_mask(const hufftnode *node)
{
	return node->ab_mask;
}

size_t hufftnode_char_rank(const hufftnode *node)
{
	return node->chr_rank;
}
