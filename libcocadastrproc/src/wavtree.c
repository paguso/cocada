/*
 * COCADA - COCADA Collection of Algorithms and DAta structures
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

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "alphabet.h"
#include "arrays.h"
#include "bitvec.h"
#include "csrsbitarr.h"
#include "cstrutil.h"
#include "huffcode.h"
#include "mathutil.h"
#include "new.h"
#include "stack.h"
#include "strbuf.h"
#include "reader.h"
#include "vec.h"
#include "wavtree.h"
#include "xstrread.h"
#include "xstrreader.h"


/****************************************************************************
 * Convenience constants & hash functions                                   *
 ****************************************************************************/

static const byte LEFT  = 0;
static const byte RIGHT = 1;

static const BitVec *NULL_CODE = NULL;

typedef struct {
	const BitVec *code;
	usize pos;
} CharCodeIter;


static const BitVec *get_charcode(Vec *code_tbl, xchar chr)
{
	if (chr < vec_len(code_tbl))
		return *((BitVec **)vec_get(code_tbl, chr));
	return NULL_CODE;
}


static void set_charcode(Vec *code_tbl, xchar chr, BitVec *code)
{
	for (usize i = vec_len(code_tbl); i <= (usize)chr; i++)
		vec_push(code_tbl, &NULL_CODE);
	vec_set(code_tbl, chr, &code);
}


bool charcode_iter_next(CharCodeIter *it)
{
	return (it->pos < bitvec_len(it->code)) ?
	       bitvec_get_bit(it->code, it->pos++) : 0;
}


void chrcode_incr(BitVec *code)
{
	byte bit, carry = 0x1;
	for (usize i = 0, l = bitvec_len(code); carry && (i < l); i++) {
		bit = bitvec_get_bit(code, i);
		bit ^= carry;
		carry &= (!bit);
		bitvec_set_bit(code, i, bit);
	}
	if (carry) {
		bitvec_push(code, 0x1);
	}
}


/****************************************************************************
 * Template wavelet tree apparatus used for construction                    *
 ****************************************************************************/

typedef struct _TmpWavTreeNode {
	usize  index;
	usize  offset;
	usize  height;
	byte   nxt_chd;
	BitVec *bv;
	struct _TmpWavTreeNode  *chd[2];
	xchar  chr[2];
}
TmpWavTreeNode;


typedef struct {
	usize nnodes;
	usize len;
	usize nchars;
	BitVec *nxt_charcode;
	Alphabet *ab;
	bool own_alphabet;
	Vec *chrcodes;
	HuffCode *hcode;
	BitVec *raw_bits;
	TmpWavTreeNode *tmp_root;
}
TmpWavTree;


static TmpWavTreeNode *tmp_wtnode_new(usize size)
{
	TmpWavTreeNode *node = NEW(TmpWavTreeNode);
	node->index      = 0;
	node->offset     = 0;
	node->height     = 0;
	node->nxt_chd    = LEFT;
	node->bv         = bitvec_new_with_capacity(size);
	node->chd[LEFT]  = NULL;
	node->chd[RIGHT] = NULL;
	node->chr[LEFT]  = XEOF;
	node->chr[RIGHT] = XEOF;
	return node;
}


static void tmp_wavtree_node_free(TmpWavTreeNode *node)
{
	if (node == NULL) return;
	tmp_wavtree_node_free(node->chd[LEFT]);
	tmp_wavtree_node_free(node->chd[RIGHT]);
	bitvec_free(node->bv);
	FREE(node);
}


static TmpWavTree *tmp_wavtree_new(const Alphabet *ab, bool own_alphabet)
{
	TmpWavTree *twt = NEW(TmpWavTree);
	twt->ab       = (Alphabet *)ab;
	twt->own_alphabet = own_alphabet;
	twt->chrcodes = vec_new(sizeof(BitVec *));
	twt->nxt_charcode = bitvec_new();
	bitvec_push(twt->nxt_charcode, 0x0);
	twt->nnodes   = 0;
	twt->len      = 0;
	twt->nchars   = 0;
	twt->hcode    = NULL;
	twt->raw_bits = bitvec_new();
	twt->tmp_root = tmp_wtnode_new(0);
	return twt;
}


// Requires: node != NULL
static void _tmp_wavtree_init_bal( TmpWavTreeNode *node, Alphabet *ab,
                                   usize l, usize r, usize crk, usize depth )
{
	usize mid = (usize)(ceil((l + r) / 2.0));
	if (mid - l == 1) {
		node->chr[LEFT] = alphabet_char(ab, crk);
	}
	else if (mid - l >= 2) {
		node->chd[LEFT] = tmp_wtnode_new(0);
		_tmp_wavtree_init_bal(node->chd[LEFT], ab, l, mid, crk, depth + 1);
	}
	if (r - mid == 1) {
		node->chr[RIGHT] = alphabet_char(ab, crk | (1 << depth));
	}
	else if (r - mid >= 2) {
		node->chd[RIGHT] = tmp_wtnode_new(0);
		_tmp_wavtree_init_bal(node->chd[RIGHT], ab, mid, r, crk | (1 << depth),
		                      depth + 1);
	}
}


static TmpWavTree *tmp_wavtree_init_bal(Alphabet *ab, bool own_ab)
{
	TmpWavTree *twt = tmp_wavtree_new(ab, own_ab);
	if (ab == NULL) return twt;
	twt->nchars = alphabet_size(ab);
	for (usize i = 0; i < twt->nchars; i++) {
		set_charcode( twt->chrcodes, alphabet_char(ab, i),
		              bitvec_clone(twt->nxt_charcode));
		chrcode_incr(twt->nxt_charcode);
	}
	// twt already has one empty root node
	_tmp_wavtree_init_bal(twt->tmp_root, ab, 0, twt->nchars, 0, 0);
	return twt;
}


// Requires: node!=NULL
//           htnode not a leaf
static void _tmp_wavtree_init_huff( TmpWavTree *twt, TmpWavTreeNode *node,
                                    const HuffTreeNode *htnode, const HuffCode *hcode )
{
	for (byte dir = LEFT; dir <= RIGHT; dir++) {
		const HuffTreeNode *chd = ( (dir == LEFT) ? hufftreenode_left(htnode) :
		                            hufftreenode_right(htnode) );
		if (hufftreenode_is_leaf(chd)) {
			usize crk = hufftreenode_char_rank(chd);
			xchar c = alphabet_char(huffcode_ab(hcode), crk);
			set_charcode( twt->chrcodes, c,
			              bitvec_clone(huffcode_charcode(hcode, crk)) );
			node->chr[dir] = c;
		}
		else {
			node->chd[dir]  = tmp_wtnode_new(0);
			_tmp_wavtree_init_huff(twt, node->chd[dir], chd, hcode);
		}
	}
}


// requires hcode != NULL
static TmpWavTree *tmp_wavtree_init_huff(const HuffCode *hcode, bool own_ab)
{
	const Alphabet *hc_ab = huffcode_ab(hcode);
	TmpWavTree *twt = tmp_wavtree_new(hc_ab, own_ab);
	twt->nchars = alphabet_size(hc_ab);
	// tmp_wt has at least one empty root node
	if (twt->nchars == 1) {
		xchar c =  alphabet_char(hc_ab, 0);
		twt->tmp_root->chr[LEFT] = c;
		set_charcode( twt->chrcodes, c,
		              bitvec_clone(huffcode_charcode(hcode, 0)) );
	}
	else if (twt->nchars > 1) {
		_tmp_wavtree_init_huff(twt, twt->tmp_root, huffcode_tree(hcode), hcode);
	}
	//huffcode_print(hcode);
	twt->ab = alphabet_clone(hc_ab);
	twt->own_alphabet = true;
	return twt;
}


static void tmp_wavtree_free(TmpWavTree *twt)
{
	alphabet_free(twt->ab);
	huffcode_free(twt->hcode);
	DESTROY_FLAT(twt->chrcodes, vec);
	tmp_wavtree_node_free(twt->tmp_root);
	FREE(twt);
}


static void tmp_wavtree_app_char(TmpWavTreeNode *root, const BitVec *chcode)
{
	CharCodeIter codeit = {.code = chcode, .pos = 0 };
	byte bit;
	while (root != NULL) {
		bit = charcode_iter_next(&codeit);
		bitvec_push(root->bv, bit);
		root = root->chd[bit];
	}
}


static void tmp_wavtree_app_new_char(TmpWavTreeNode *root, xchar c,
                                     BitVec *chcode)
{
	TmpWavTreeNode *node = root, *parent = NULL;
	while (true) {
		if (node != NULL) {
			bitvec_push(node->bv, node->nxt_chd);
			parent = node;
			node = node->chd[node->nxt_chd];
			parent->nxt_chd ^= 0x1;
		}
		else {
			// case first & second symbols: 'ignore'
			if (bitvec_len(chcode) < 2) return;
			// add new leaf
			byte bit = parent->nxt_chd ^ 0x1;
			usize q = bitvec_count(parent->bv, bit);
			TmpWavTreeNode *new_node = tmp_wtnode_new(q);
			bitvec_push_n(new_node->bv, q - 1, 0x0);
			bitvec_push(new_node->bv, 0x1);
			new_node->chr[LEFT] = parent->chr[bit];
			new_node->chr[RIGHT] = c;
			parent->chd[bit] = new_node;
			return;
		}
	}
}


static void tmp_wavtree_fill( TmpWavTree *twt, xstrRead *rdr )
{
	xstrread_reset(rdr);
	for (xwchar c; (c = xstrread_getc(rdr)) != XEOF;) {
		tmp_wavtree_app_char( twt->tmp_root, get_charcode(twt->chrcodes, c) );
		(twt->len)++;
	}
}


// online construction only available for CHAR_TYPE alphabets
static void tmp_wavtree_fill_online( TmpWavTree *twt, Reader *src )
{
	StrBuf *ab_chars = strbuf_new();
	reader_reset(src);
	for (int c; (c = reader_getc(src)) != EOF;) {
		BitVec *chcode = (BitVec *) get_charcode(twt->chrcodes, c);
		if (chcode != NULL_CODE) {
			tmp_wavtree_app_char(twt->tmp_root, chcode);
		}
		else {
			chcode = bitvec_clone(twt->nxt_charcode);
			chrcode_incr(twt->nxt_charcode);
			set_charcode(twt->chrcodes, c, chcode);
			strbuf_append_char(ab_chars, (char)c);
			tmp_wavtree_app_new_char(twt->tmp_root, c, chcode);
		}
		(twt->len)++;
	}
	twt->ab = alphabet_new(strbuf_len(ab_chars), strbuf_detach(ab_chars));
	twt->own_alphabet = true;
}


static usize tmp_wavtree_init_height(TmpWavTreeNode *root)
{
	if (root == NULL) {
		return 0;
	}
	else {
		usize l = tmp_wavtree_init_height(root->chd[LEFT]);
		usize r = tmp_wavtree_init_height(root->chd[RIGHT]);
		root->height = 1 + MAX(l, r);
		return root->height;
	}
}


static void _tmp_wavtree_init_veb_layout( TmpWavTreeNode *node, usize heig,
        usize *vebindex, usize *offset, BitVec *raw_bits )
{
	if (heig == 1) {
		node->index = (*vebindex)++;
		node->offset = *offset;
		(*offset) += bitvec_len(node->bv);
		bitvec_cat(raw_bits, node->bv);
		//printf("[id=%zu pos=%zu len=%zu]\n",node->id, node->offset, node->len);
	}
	else {
		usize depth, nxtchd;
		_tmp_wavtree_init_veb_layout(node, heig / 2, vebindex, offset, raw_bits);
		stack *stknode = stack_new(sizeof(TmpWavTreeNode *));
		stack *stkdepth = stack_new(sizeof(usize));
		stack *stknxtchd = stack_new(sizeof(usize));
		stack_push(stknode, &node);
		stack_push_usize(stkdepth, 1);
		stack_push_usize(stknxtchd, 0);
		while ( !stack_empty(stknode) ) {
			stack_pop(stknode, &node);
			depth = stack_pop_usize(stkdepth);
			nxtchd = stack_pop_usize(stknxtchd);
			if (node == NULL)
				continue;
			if (depth == heig / 2) {
				if (node->chd[LEFT] != NULL) {
					_tmp_wavtree_init_veb_layout( node->chd[LEFT],
					                              MIN( node->chd[LEFT]->height,
					                                   heig - (heig / 2) ),
					                              vebindex, offset, raw_bits );
				}
				if (node->chd[RIGHT] != NULL) {
					_tmp_wavtree_init_veb_layout( node->chd[RIGHT],
					                              MIN( node->chd[RIGHT]->height,
					                                   heig - (heig / 2) ),
					                              vebindex, offset, raw_bits );
				}
			}
			else if (nxtchd < 2) {
				stack_push(stknode, &node);
				stack_push_usize(stkdepth, depth);
				stack_push_usize(stknxtchd, nxtchd + 1);

				stack_push(stknode, (nxtchd == 0) ? node->chd + LEFT : node->chd + RIGHT);
				stack_push_usize(stkdepth, depth + 1);
				stack_push_usize(stknxtchd, 0);
			}
		}
		DESTROY_FLAT(stknode, stack);
		DESTROY_FLAT(stkdepth, stack);
		DESTROY_FLAT(stknxtchd, stack);
	}
}

/*
 * The nodes of the WT are arranged in an array according to the
 * van Emde Boas recursive partitioning of the tree to explore memory locality.
 */
static void tmp_wavtree_init_veb_layout( TmpWavTree *twt )
{
	if (twt->tmp_root == NULL)
		return;
	usize vebindex = 0;
	usize offset = 0;
	tmp_wavtree_init_height(twt->tmp_root);
	_tmp_wavtree_init_veb_layout( twt->tmp_root, twt->tmp_root->height,
	                              &vebindex, &offset, twt->raw_bits );
	twt->nnodes = vebindex;
}


static void tmp_wavtree_node_print(FILE *stream, TmpWavTreeNode *node,
                                   usize depth)
{
	usize i;
	char *margin = cstr_new(2 * depth + 2);
	for (i = 0; i < 2 * depth; i++) {
		margin[i] = ' ';
	}
	margin[i++] = '|';
	margin[i++] = ' ';
	margin[i++] = '\0';
	if (node == NULL) {
		fprintf (stream, "%s@tmp_wtree_node NULL\n", margin);
	}
	else {
		fprintf(stream, "%s@tmp_wtree_node %p\n", margin, node);
		fprintf(stream, "%ssize: %zu\n", margin, bitvec_len(node->bv));
		fprintf(stream, "%snxt_chd: %c\n", margin, node->nxt_chd ? '1' : '0');
		fprintf(stream, "%sbits: \n", margin);
		bitvec_print(stream, node->bv, 8);
		tmp_wavtree_node_print(stream, node->chd[LEFT], depth + 1);
		tmp_wavtree_node_print(stream, node->chd[RIGHT], depth + 1);
	}
}


static void tmp_wavtree_print(FILE *stream, TmpWavTree *twt)
{
	fprintf (stream, "tmp_wavtree@%p\n", twt);
	tmp_wavtree_node_print(stream, twt->tmp_root, 0);
}


/****************************************************************************
 * Wavelet tree ADT and construction                                        *
 ****************************************************************************/

typedef union  {
	usize chd;
	xchar chr;
} size_or_xchar;

typedef struct _wtnode {
	usize len;
	usize offset;
	usize cumul_bits[2];
	byte has_chd; //code: 0=none (leaf), 1=left chd only, 2=right only, 3=both
	size_or_xchar cc[2]; // if leaf stores left/right chars;
	// else stores left/right children indexes.
}
wtnode;


struct _WavTree {
	WavTreeShape       shape;
	usize        nnodes;
	wtnode       *nodes;
	Alphabet     *ab;
	bool          own_ab;
	Vec     *chrcodes;
	usize        len;
	CSRSBitArr *bitarr;
};


static void _wavtree_build_from_tmp(wtnode *nodes, TmpWavTreeNode *tnode)
{
	if (tnode == NULL) return;
	nodes[tnode->index].len = bitvec_len(tnode->bv);
	nodes[tnode->index].offset = tnode->offset;
	nodes[tnode->index].has_chd = 0x0;
	if (tnode->chd[LEFT] != NULL) {
		nodes[tnode->index].has_chd |= 0x1;
		nodes[tnode->index].cc[LEFT].chd = tnode->chd[LEFT]->index;
		_wavtree_build_from_tmp(nodes, tnode->chd[LEFT]);
	}
	else {
		nodes[tnode->index].cc[LEFT].chr = tnode->chr[LEFT];
	}
	if (tnode->chd[RIGHT] != NULL) {
		nodes[tnode->index].has_chd |= 0x2;
		nodes[tnode->index].cc[RIGHT].chd = tnode->chd[RIGHT]->index;
		_wavtree_build_from_tmp(nodes, tnode->chd[RIGHT]);
	}
	else {
		nodes[tnode->index].cc[RIGHT].chr = tnode->chr[RIGHT];
	}
}


static WavTree *wavtree_build_from_tmp( TmpWavTree *twt, WavTreeShape shape )
{
	tmp_wavtree_init_veb_layout(twt);
	WavTree *wt = NEW(WavTree);
	wt->nnodes = twt->nnodes;
	wt->nodes = ARR_NEW(wtnode, twt->nnodes);
	_wavtree_build_from_tmp(wt->nodes, twt->tmp_root);
	wt->len = twt->len;
	usize nbits = bitvec_len(twt->raw_bits);
	//bitvec_print(tmp_wt->raw_bits, 4);
	wt->bitarr = csrsbitarr_new(bitvec_detach(twt->raw_bits), nbits);
	twt->raw_bits = NULL; // prevents from freeing on twt destruction
	for (usize i = 0; i < wt->nnodes; ++i) {
		wt->nodes[i].cumul_bits[1] = csrsbitarr_rank1( wt->bitarr,
		                             wt->nodes[i].offset );
		wt->nodes[i].cumul_bits[0] = wt->nodes[i].offset -
		                             wt->nodes[i].cumul_bits[1];
	}
	// character code table and its inverse
	wt->ab = twt->ab;
	wt->own_ab = twt->own_alphabet;
	twt->ab = NULL;
	wt->chrcodes = twt->chrcodes;
	twt->chrcodes = NULL;
	return wt;
}


static WavTree *wavtree_build( Alphabet *ab, xstrRead *rdr,
                               WavTreeShape shape )
{
	TmpWavTree *twt;
	switch (shape) {
	case WT_BALANCED:
		twt =  tmp_wavtree_init_bal(ab, ab == NULL);
		break;
	case WT_HUFFMAN:
		;
		HuffCode *hcode;
		hcode = huffcode_new_from_xstrread(ab, rdr);
		//huffcode_print(hcode);
		twt = tmp_wavtree_init_huff(hcode, ab == NULL);
		break;
	}
	tmp_wavtree_fill(twt, rdr);
	WavTree *wt = wavtree_build_from_tmp(twt, shape);
	wt->shape = shape;
	tmp_wavtree_free(twt);
	return wt;
}


WavTree *wavtree_new( Alphabet *ab, char *str, usize len, WavTreeShape shape )
{
	xstrReader *rdr = xstrreader_open_str(str, len);
	WavTree *wt = wavtree_build(ab, xstrReader_as_xstrRead(rdr), shape);
	xstrreader_close(rdr);
	return wt;
}


WavTree *wavtree_new_from_xstr( Alphabet *ab, xstr *str, WavTreeShape shape )
{
	xstrReader *rdr = xstrreader_open(str);
	WavTree *wt = wavtree_build(ab, xstrReader_as_xstrRead(rdr), shape);
	xstrreader_close(rdr);
	return wt;
}


WavTree *wavtree_new_from_reader( Alphabet *ab, xstrRead *src,
                                  WavTreeShape shape )
{
	return wavtree_build( ab, src, shape);
}


WavTree *wavtree_new_online( Reader *src )
{
	TmpWavTree *twt =  tmp_wavtree_init_bal(NULL, true);
	tmp_wavtree_fill_online(twt, src);
	WavTree *wt = wavtree_build_from_tmp(twt, WT_BALANCED);
	wt->shape = WT_BALANCED;
	tmp_wavtree_free(twt);
	return wt;
}


void wavtree_free(WavTree *wt)
{
	if (wt == NULL) return;
	if (wt->own_ab) alphabet_free(wt->ab);
	DESTROY_FLAT(wt->chrcodes, vec);
	csrsbitarr_free(wt->bitarr, true);
	FREE(wt->nodes);
	FREE(wt);
}


/****************************************************************************
 * Wavelet tree operations                                                  *
 ****************************************************************************/

inline usize wavtree_len(WavTree *wt)
{
	return wt->len;
}


usize wavtree_rank_pos(WavTree *wt, usize pos)
{
	if (pos >= wt->len) return SIZE_MAX;
	usize cur = 0;
	usize rank = pos;
	byte bit;
	while ( true ) {
		bit = csrsbitarr_get(wt->bitarr, wt->nodes[cur].offset + rank);
		rank = csrsbitarr_rank(wt->bitarr, wt->nodes[cur].offset + rank, bit)
		       - wt->nodes[cur].cumul_bits[bit];
		if ( wt->nodes[cur].has_chd & (bit + 1) )
			cur = wt->nodes[cur].cc[bit].chd;
		else
			break;
	}
	return rank;
}


usize wavtree_rank(WavTree *wt, usize pos, xchar c)
{
	usize cur = 0;
	CharCodeIter codeit = {.code = get_charcode(wt->chrcodes, c), .pos = 0};
	if ( codeit.code == NULL_CODE || wt->len == 0 ) return 0;
	usize rank = MIN(wt->nodes[cur].len, pos);
	byte bit;
	while ( true ) {
		bit = charcode_iter_next(&codeit);
		rank = csrsbitarr_rank( wt->bitarr,
		                        wt->nodes[cur].offset +
		                        MIN(rank, wt->nodes[cur].len),
		                        bit )
		       - wt->nodes[cur].cumul_bits[bit];
		if ( (rank > 0) && (wt->nodes[cur].has_chd & (bit + 1)) )
			cur = wt->nodes[cur].cc[bit].chd;
		else
			break;
	}
	return rank;
}


static usize _wavtree_select( WavTree *wt, usize cur,
                              CharCodeIter *codeit, usize rank )
{
	if (cur >= wt->nnodes || wt->nodes[cur].len == 0) return 0;
	usize sel;
	byte bit;
	bit = charcode_iter_next(codeit);
	if ( !(wt->nodes[cur].has_chd & (bit + 1)) ) {
		sel = csrsbitarr_select( wt->bitarr,
		                         wt->nodes[cur].cumul_bits[bit] + rank, bit )
		      - wt->nodes[cur].offset;
	}
	else {
		sel = _wavtree_select( wt, wt->nodes[cur].cc[bit].chd, codeit, rank );
		sel = csrsbitarr_select( wt->bitarr,
		                         wt->nodes[cur].cumul_bits[bit] + sel, bit )
		      - wt->nodes[cur].offset;
	}
	return MIN(sel, wt->nodes[cur].len);
}


usize wavtree_select(WavTree *wt, xchar c, usize rank)
{
	CharCodeIter codeit = { .code = get_charcode(wt->chrcodes, c), .pos = 0 };
	if (codeit.code == NULL_CODE) return wt->len;
	return _wavtree_select(wt, 0, &codeit, rank);
}


usize wavtree_pred(WavTree *wt, usize pos, xchar c)
{
	usize rank = wavtree_rank(wt, pos, c);
	return (rank > 0) ? wavtree_select(wt, c, rank - 1) : wt->len;
}


usize wavtree_succ(WavTree *wt, usize pos, xchar c)
{
	if (pos >= wt->len) return wt->len;
	usize rank = wavtree_rank(wt, pos, c);
	return (wavtree_char(wt, pos) == c) ? wavtree_select(wt, c, rank + 1) :
	       wavtree_select(wt, c, rank);
}


xchar wavtree_char(WavTree *wt, usize pos)
{
	if ( pos >= wt->len ) return XEOF;
	usize cur = 0;
	usize rank = pos;
	byte bit;
	while ( true ) {
		bit = csrsbitarr_get(wt->bitarr, wt->nodes[cur].offset + rank);
		rank = csrsbitarr_rank(wt->bitarr, wt->nodes[cur].offset + rank, bit)
		       - wt->nodes[cur].cumul_bits[bit];
		if ( wt->nodes[cur].has_chd & (bit + 1) )
			cur = wt->nodes[cur].cc[bit].chd;
		else
			break;
	}
	return wt->nodes[cur].cc[bit].chr;
}


// Print

void _wt_node_print(WavTree *wt, usize cur, usize depth)
{
	usize i;
	StrBuf *dmargin = strbuf_new_with_capacity(2 * depth + 2);
	for (i = 0; i < depth; i++)
		strbuf_nappend(dmargin, "  ", 2);
	strbuf_nappend(dmargin, "| ", 2);
	char *margin = strbuf_detach(dmargin);
	if (cur >= wt->nnodes) {
		printf ("%s@wtree_node NULL\n", margin);
	}
	else {
		printf ("%s@wtree_node #%zu\n", margin, cur);
		printf ("%slen: %zu\n", margin, wt->nodes[cur].len);
		printf ("%soffset: %zu\n", margin, wt->nodes[cur].offset);
		if (!(wt->nodes[cur].has_chd & (0x1) )) // no left chd
			printf ( "%schar[0]: %"XCHAR_FMT" (%c)\n", margin,
			         wt->nodes[cur].cc[LEFT].chr,
			         (char)wt->nodes[cur].cc[LEFT].chr );
		if (!(wt->nodes[cur].has_chd & (0x2) )) // no right chd
			printf ( "%schar[1]: %"XCHAR_FMT" (%c)\n", margin,
			         wt->nodes[cur].cc[RIGHT].chr,
			         (char)wt->nodes[cur].cc[RIGHT].chr  );
		if (wt->nodes[cur].has_chd & (0x1) ) // has left chd
			_wt_node_print(wt, wt->nodes[cur].cc[LEFT].chd, depth + 1);
		if (wt->nodes[cur].has_chd & (0x2) ) // has right chd
			_wt_node_print(wt, wt->nodes[cur].cc[RIGHT].chd, depth + 1);
	}
}


void wavtree_print(WavTree *wt)
{
	char *shapes[2] = {"BAL", "HUFF"};
	printf ("wavelet_tree@%p {\n", wt);
	printf ("  tshape: %s\n", shapes[wt->shape]);
	printf ("  tree:\n");
	_wt_node_print(wt, 0, 0);
	//printf ("  bitarray:\n");
	//csrsbitarr_fprint(wt->bitarr, 4);
	if (alphabet_type(wt->ab) == CHAR_TYPE) {
		printf ("  char codes:\n");
		StrBuf *codestr = strbuf_new_with_capacity(2);
		for (xchar c = 0; c <= UCHAR_MAX; c++) {
			const BitVec *code = get_charcode(wt->chrcodes, c);
			if (code != NULL_CODE) {
				strbuf_clear(codestr);
				bitvec_to_string(code, codestr, 4);
				printf("charcode %c:\n%s\n", (char)c, strbuf_as_str(codestr));
				//uint_to_cstr(chcodestr, code, 'b');
				//cstr_revert(chcodestr, chcode_bits);
				//printf ("    %c : %s\n", (char)c, chcodestr);
			}
		}
	}
	printf ("} #end of wavelet_tree@%p\n\n", wt);
}


