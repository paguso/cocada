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

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "alphabet.h"
#include "arrays.h"
#include "bitbyte.h"
#include "bitvec.h"
#include "bytearr.h"
#include "csrsbitarr.h"
#include "cstrutil.h"
#include "hashmap.h"
#include "huffcode.h"
#include "mathutil.h"
#include "new.h"
#include "stack.h"
#include "strbuf.h"
#include "strread.h"
#include "strreader.h"
#include "vec.h"
#include "wavtree.h"
#include "xstrread.h"
#include "xstrreader.h"


/****************************************************************************
 * Convenience constants & hash functions                                   *
 ****************************************************************************/

static const byte_t LEFT  = 0x0;
static const byte_t RIGHT = 0x1;

static const bitvec *NULL_CODE = NULL;

typedef struct {
	const bitvec *code;
	size_t pos;
} charcode_iter;


static const bitvec *get_charcode(vec *code_tbl, xchar_t chr)
{
	if (chr < vec_len(code_tbl))
		return *((bitvec **)vec_get(code_tbl, chr));
	return NULL_CODE;
}


static void set_charcode(vec *code_tbl, xchar_t chr, bitvec *code)
{
	for (size_t i=vec_len(code_tbl); i<=(size_t)chr; i++)
		vec_push(code_tbl, &NULL_CODE);
	vec_set(code_tbl, chr, &code);
}


bool charcode_iter_next(charcode_iter *it)
{
	return (it->pos < bitvec_len(it->code)) ?
	       bitvec_get_bit(it->code, it->pos++) : 0;
}


void chrcode_incr(bitvec *code)
{
	byte_t bit, carry = 0x1;
	for (size_t i=0, l=bitvec_len(code); carry && (i<l); i++) {
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

typedef struct _tmp_wtnode {
	size_t               index;
	size_t               offset;
	size_t               height;
	byte_t               nxt_chd;
	bitvec           *bv;
	struct _tmp_wtnode  *chd[2];
	xchar_t              chr[2];
}
tmp_wtnode;


typedef struct _tmp_wavtree {
	size_t          nnodes;
	size_t          len;
	size_t          nchars;
	bitvec      *nxt_charcode;
	alphabet       *ab;
	bool            own_alphabet;
	vec       *chrcodes;
	huffcode       *hcode;
	bitvec      *raw_bits;
	tmp_wtnode     *tmp_root;
}
tmp_wavtree;


static tmp_wtnode *tmp_wtnode_new(size_t size)
{
	tmp_wtnode *node = NEW(tmp_wtnode);
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


static void tmp_wtnode_free(tmp_wtnode *node)
{
	if (node == NULL) return;
	tmp_wtnode_free(node->chd[LEFT]);
	tmp_wtnode_free(node->chd[RIGHT]);
	bitvec_free(node->bv);
	FREE(node);
}


static tmp_wavtree *tmp_wavtree_new(const alphabet *ab, bool own_alphabet)
{
	tmp_wavtree *twt = NEW(tmp_wavtree);
	twt->ab       = (alphabet *)ab;
	twt->own_alphabet = own_alphabet;
	twt->chrcodes = vec_new(sizeof(bitvec *));
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
static void _tmp_wt_init_bal( tmp_wtnode *node, alphabet *ab,
                              size_t l, size_t r, size_t crk, size_t depth )
{
	size_t mid = (size_t)(ceil((l+r)/2.0));
	if (mid-l == 1) {
		node->chr[LEFT] = ab_char(ab, crk);
	} else if (mid-l >= 2) {
		node->chd[LEFT] = tmp_wtnode_new(0);
		_tmp_wt_init_bal(node->chd[LEFT], ab, l, mid, crk, depth+1);
	}
	if (r-mid == 1) {
		node->chr[RIGHT] = ab_char(ab, crk|(1<<depth));
	} else if (r-mid >= 2) {
		node->chd[RIGHT] = tmp_wtnode_new(0);
		_tmp_wt_init_bal(node->chd[RIGHT], ab, mid, r, crk|(1<<depth), depth+1);
	}
}


static tmp_wavtree *tmp_wt_init_bal(alphabet *ab, bool own_ab)
{
	tmp_wavtree *twt = tmp_wavtree_new(ab, own_ab);
	if (ab==NULL) return twt;
	twt->nchars = ab_size(ab);
	for (size_t i=0; i<twt->nchars; i++) {
		set_charcode( twt->chrcodes, ab_char(ab, i),
		              bitvec_clone(twt->nxt_charcode));
		chrcode_incr(twt->nxt_charcode);
	}
	// twt already has one empty root node
	_tmp_wt_init_bal(twt->tmp_root, ab, 0, twt->nchars, 0, 0);
	return twt;
}


// Requires: node!=NULL
//           htnode not a leaf
static void _tmp_wt_init_huff( tmp_wavtree *twt, tmp_wtnode *node,
                               const hufftnode *htnode, const huffcode *hcode )
{
	for (byte_t dir=LEFT; dir<=RIGHT; dir++) {
		const hufftnode *chd = ( (dir==LEFT) ? hufftnode_left(htnode) :
		                         hufftnode_right(htnode) );
		if (hufftnode_is_leaf(chd)) {
			size_t crk = hufftnode_char_rank(chd);
			xchar_t c = ab_char(huffcode_ab(hcode), crk);
			set_charcode( twt->chrcodes, c,
			              bitvec_clone(huffcode_charcode(hcode, crk)) );
			node->chr[dir] = c;
		} else {
			node->chd[dir]  = tmp_wtnode_new(0);
			_tmp_wt_init_huff(twt, node->chd[dir], chd, hcode);
		}
	}
}


// requires hcode != NULL
static tmp_wavtree *tmp_wt_init_huff(const huffcode *hcode, bool own_ab)
{
	const alphabet *hc_ab = huffcode_ab(hcode);
	tmp_wavtree *twt = tmp_wavtree_new(hc_ab, own_ab);
	twt->nchars = ab_size(hc_ab);
	// tmp_wt has at least one empty root node
	if (twt->nchars == 1) {
		xchar_t c =  ab_char(hc_ab, 0);
		twt->tmp_root->chr[LEFT] = c;
		set_charcode( twt->chrcodes, c,
		              bitvec_clone(huffcode_charcode(hcode, 0)) );
	} else if (twt->nchars > 1) {
		_tmp_wt_init_huff(twt, twt->tmp_root, huffcode_tree(hcode), hcode);
	}
	//huffcode_print(hcode);
	twt->ab = alphabet_clone(hc_ab);
	twt->own_alphabet = true;
	return twt;
}


static void tmp_wt_free(tmp_wavtree *twt)
{
	alphabet_free(twt->ab);
	huffcode_free(twt->hcode);
	DESTROY_FLAT(twt->chrcodes, vec);
	tmp_wtnode_free(twt->tmp_root);
	FREE(twt);
}


static void tmp_wt_app_char(tmp_wtnode *root, const bitvec *chcode)
{
	charcode_iter codeit = {.code=chcode, .pos=0 };
	byte_t bit;
	while (root != NULL) {
		bit = charcode_iter_next(&codeit);
		bitvec_push(root->bv, bit);
		root = root->chd[bit];
	}
}


static void tmp_wt_app_new_char(tmp_wtnode *root, xchar_t c, bitvec *chcode)
{
	tmp_wtnode *node=root, *parent=NULL;
	while (true) {
		if (node!=NULL) {
			bitvec_push(node->bv, node->nxt_chd);
			parent = node;
			node = node->chd[node->nxt_chd];
			parent->nxt_chd ^= 0x1;
		} else {
			// case first & second symbols: 'ignore'
			if (bitvec_len(chcode) < 2) return;
			// add new leaf
			byte_t bit = parent->nxt_chd^0x1;
			size_t q = bitvec_count(parent->bv, bit);
			tmp_wtnode *new_node = tmp_wtnode_new(q);
			bitvec_push_n(new_node->bv, q-1, 0x0);
			bitvec_push(new_node->bv, 0x1);
			new_node->chr[LEFT] = parent->chr[bit];
			new_node->chr[RIGHT] = c;
			parent->chd[bit] = new_node;
			return;
		}
	}
}


static void tmp_wt_fill( tmp_wavtree *twt, xstrread *rdr )
{
	xstrread_reset(rdr);
	for (xchar_wt c; (c=xstrread_getc(rdr)) != XEOF;) {
		tmp_wt_app_char( twt->tmp_root, get_charcode(twt->chrcodes, c) );
		(twt->len)++;
	}
}


// online construction only available for CHAR_TYPE alphabets
static void tmp_wt_fill_online( tmp_wavtree *twt, strread *src )
{
	strbuf *ab_chars = strbuf_new();
	strread_reset(src);
	for (int c; (c=strread_getc(src))!=EOF;) {
		bitvec *chcode = (bitvec *) get_charcode(twt->chrcodes, c);
		if (chcode != NULL_CODE) {
			tmp_wt_app_char(twt->tmp_root, chcode);
		} else {
			chcode = bitvec_clone(twt->nxt_charcode);
			chrcode_incr(twt->nxt_charcode);
			set_charcode(twt->chrcodes, c, chcode);
			strbuf_append_char(ab_chars, (char)c);
			tmp_wt_app_new_char(twt->tmp_root, c, chcode);
		}
		(twt->len)++;
	}
	twt->ab = alphabet_new(strbuf_len(ab_chars), strbuf_detach(ab_chars));
	twt->own_alphabet = true;
}


static size_t tmp_wt_init_height(tmp_wtnode *root)
{
	if (root==NULL) {
		return 0;
	} else {
		size_t l = tmp_wt_init_height(root->chd[LEFT]);
		size_t r = tmp_wt_init_height(root->chd[RIGHT]);
		root->height = 1 + MAX(l, r);
		return root->height;
	}
}


static void _tmp_wavtree_init_veb_layout( tmp_wtnode *node, size_t heig,
        size_t *vebindex, size_t *offset, bitvec *raw_bits )
{
	if (heig==1) {
		node->index = (*vebindex)++;
		node->offset = *offset;
		(*offset) += bitvec_len(node->bv);
		bitvec_cat(raw_bits, node->bv);
		//printf("[id=%zu pos=%zu len=%zu]\n",node->id, node->offset, node->len);
	} else {
		size_t depth, nxtchd;
		_tmp_wavtree_init_veb_layout(node, heig/2, vebindex, offset, raw_bits);
		stack *stknode = stack_new(sizeof(tmp_wtnode *));
		stack *stkdepth = stack_new(sizeof(size_t));
		stack *stknxtchd = stack_new(sizeof(size_t));
		stack_push(stknode, &node);
		stack_push_size_t(stkdepth, 1);
		stack_push_size_t(stknxtchd, 0);
		while ( !stack_empty(stknode) ) {
			stack_pop(stknode, &node);
			depth = stack_pop_size_t(stkdepth);
			nxtchd = stack_pop_size_t(stknxtchd);
			if (node==NULL)
				continue;
			if (depth==heig/2) {
				if (node->chd[LEFT]!=NULL) {
					_tmp_wavtree_init_veb_layout( node->chd[LEFT],
					                              MIN( node->chd[LEFT]->height,
					                                   heig-(heig/2) ),
					                              vebindex, offset, raw_bits );
				}
				if (node->chd[RIGHT]!=NULL) {
					_tmp_wavtree_init_veb_layout( node->chd[RIGHT],
					                              MIN( node->chd[RIGHT]->height,
					                                   heig-(heig/2) ),
					                              vebindex, offset, raw_bits );
				}
			} else if (nxtchd<2) {
				stack_push(stknode, &node);
				stack_push_size_t(stkdepth, depth);
				stack_push_size_t(stknxtchd, nxtchd+1);

				stack_push(stknode, (nxtchd==0)?node->chd+LEFT:node->chd+RIGHT);
				stack_push_size_t(stkdepth, depth+1);
				stack_push_size_t(stknxtchd, 0);
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
static void tmp_wavtree_init_veb_layout( tmp_wavtree *twt )
{
	if (twt->tmp_root==NULL)
		return;
	size_t vebindex = 0;
	size_t offset = 0;
	tmp_wt_init_height(twt->tmp_root);
	_tmp_wavtree_init_veb_layout( twt->tmp_root, twt->tmp_root->height,
	                              &vebindex, &offset, twt->raw_bits );
	twt->nnodes = vebindex;
}


static void tmp_wtnode_print(FILE *stream, tmp_wtnode *node, size_t depth)
{
	size_t i;
	char *margin = cstr_new(2*depth+2);
	for (i=0; i<2*depth; i++) {
		margin[i] = ' ';
	}
	margin[i++] = '|';
	margin[i++] = ' ';
	margin[i++] = '\0';
	if (node==NULL) {
		fprintf (stream, "%s@tmp_wtree_node NULL\n",margin);
	} else {
		fprintf(stream, "%s@tmp_wtree_node %p\n",margin, node);
		fprintf(stream, "%ssize: %zu\n", margin, bitvec_len(node->bv));
		fprintf(stream, "%snxt_chd: %c\n",margin, node->nxt_chd?'1':'0');
		fprintf(stream, "%sbits: \n", margin);
		bitvec_print(stream, node->bv, 8);
		tmp_wtnode_print(stream, node->chd[LEFT], depth+1);
		tmp_wtnode_print(stream, node->chd[RIGHT], depth+1);
	}
}


static void tmp_wt_print(FILE *stream, tmp_wavtree *twt)
{
	fprintf (stream, "tmp_wavtree@%p\n",twt);
	tmp_wtnode_print(stream, twt->tmp_root, 0);
}


/****************************************************************************
 * Wavelet tree ADT and construction                                        *
 ****************************************************************************/

typedef union  {
	size_t  chd;
	xchar_t chr;
} size_or_xchar;

typedef struct _wtnode {
	size_t len;
	size_t offset;
	size_t cumul_bits[2];
	byte_t has_chd; //code: 0=none (leaf), 1=left chd only, 2=right only, 3=both
	size_or_xchar cc[2]; // if leaf stores left/right chars;
	// else stores left/right children indexes.
}
wtnode;


struct _wavtree {
	wtshape       shape;
	size_t        nnodes;
	wtnode       *nodes;
	alphabet     *ab;
	bool          own_ab;
	vec     *chrcodes;
	size_t        len;
	csrsbitarr *bitarr;
};


static void _wt_build_from_tmp(wtnode *nodes, tmp_wtnode *tnode)
{
	if (tnode == NULL) return;
	nodes[tnode->index].len = bitvec_len(tnode->bv);
	nodes[tnode->index].offset = tnode->offset;
	nodes[tnode->index].has_chd = 0x0;
	if (tnode->chd[LEFT] != NULL) {
		nodes[tnode->index].has_chd |= 0x1;
		nodes[tnode->index].cc[LEFT].chd = tnode->chd[LEFT]->index;
		_wt_build_from_tmp(nodes, tnode->chd[LEFT]);
	} else {
		nodes[tnode->index].cc[LEFT].chr = tnode->chr[LEFT];
	}
	if (tnode->chd[RIGHT] != NULL) {
		nodes[tnode->index].has_chd |= 0x2;
		nodes[tnode->index].cc[RIGHT].chd = tnode->chd[RIGHT]->index;
		_wt_build_from_tmp(nodes, tnode->chd[RIGHT]);
	} else {
		nodes[tnode->index].cc[RIGHT].chr = tnode->chr[RIGHT];
	}
}


static wavtree *wt_build_from_tmp( tmp_wavtree *twt, wtshape shape )
{
	tmp_wavtree_init_veb_layout(twt);
	wavtree *wt = NEW(wavtree);
	wt->nnodes = twt->nnodes;
	wt->nodes = ARR_NEW(wtnode, twt->nnodes);
	_wt_build_from_tmp(wt->nodes, twt->tmp_root);
	wt->len = twt->len;
	size_t nbits = bitvec_len(twt->raw_bits);
	//bitvec_print(tmp_wt->raw_bits, 4);
	wt->bitarr = csrsbitarr_new(bitvec_detach(twt->raw_bits), nbits);
	twt->raw_bits = NULL; // prevents from freeing on twt destruction
	for (size_t i = 0; i < wt->nnodes; ++i) {
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


static wavtree *wt_build( alphabet *ab, xstrread *rdr,
                          wtshape shape )
{
	tmp_wavtree *twt;
	switch (shape) {
	case WT_BALANCED:
		twt =  tmp_wt_init_bal(ab, ab==NULL);
		break;
	case WT_HUFFMAN:
		;
		huffcode *hcode;
		hcode = huffcode_new_from_xstrread(ab, rdr);
		//huffcode_print(hcode);
		twt = tmp_wt_init_huff(hcode, ab==NULL);
		break;
	}
	tmp_wt_fill(twt, rdr);
	wavtree *wt = wt_build_from_tmp(twt, shape);
	wt->shape = shape;
	tmp_wt_free(twt);
	return wt;
}


wavtree *wavtree_new( alphabet *ab, char *str, size_t len, wtshape shape )
{
	xstrreader *rdr = xstrreader_open_str(str, len);
	wavtree *wt = wt_build(ab, xstrreader_as_xstrread(rdr), shape);
	xstrreader_close(rdr);
	return wt;
}


wavtree *wavtree_new_from_xstr( alphabet *ab, xstr *str, wtshape shape )
{
	xstrreader *rdr = xstrreader_open(str);
	wavtree *wt = wt_build(ab, xstrreader_as_xstrread(rdr), shape);
	xstrreader_close(rdr);
	return wt;
}


wavtree *wavtree_new_from_reader( alphabet *ab, xstrread *src, wtshape shape )
{
	return wt_build( ab, src, shape);
}


wavtree *wavtree_new_online( strread *src )
{
	tmp_wavtree *twt =  tmp_wt_init_bal(NULL, true);
	tmp_wt_fill_online(twt, src);
	wavtree *wt = wt_build_from_tmp(twt, WT_BALANCED);
	wt->shape = WT_BALANCED;
	tmp_wt_free(twt);
	return wt;
}


void wavtree_free(wavtree *wt)
{
	if (wt==NULL) return;
	if (wt->own_ab) alphabet_free(wt->ab);
	DESTROY_FLAT(wt->chrcodes, vec);
	csrsbitarr_free(wt->bitarr, true);
	FREE(wt->nodes);
	FREE(wt);
}


/****************************************************************************
 * Wavelet tree operations                                                  *
 ****************************************************************************/

inline size_t wavtree_len(wavtree *wt)
{
	return wt->len;
}


size_t wavtree_rank_pos(wavtree *wt, size_t pos)
{
	if (pos>=wt->len) return SIZE_MAX;
	size_t cur = 0;
	size_t rank = pos;
	byte_t bit;
	while ( true ) {
		bit = csrsbitarr_get(wt->bitarr, wt->nodes[cur].offset+rank);
		rank = csrsbitarr_rank(wt->bitarr, wt->nodes[cur].offset+rank, bit)
		       - wt->nodes[cur].cumul_bits[bit];
		if ( wt->nodes[cur].has_chd & (bit+1) )
			cur = wt->nodes[cur].cc[bit].chd;
		else
			break;
	}
	return rank;
}


size_t wavtree_rank(wavtree *wt, size_t pos, xchar_t c)
{
	size_t cur = 0;
	charcode_iter codeit = {.code=get_charcode(wt->chrcodes, c), .pos=0};
	if ( codeit.code==NULL_CODE || wt->len==0 ) return 0;
	size_t rank = MIN(wt->nodes[cur].len, pos);
	byte_t bit;
	while ( true ) {
		bit = charcode_iter_next(&codeit);
		rank = csrsbitarr_rank( wt->bitarr,
		                        wt->nodes[cur].offset +
		                        MIN(rank, wt->nodes[cur].len),
		                        bit )
		       - wt->nodes[cur].cumul_bits[bit];
		if ( (rank > 0) && (wt->nodes[cur].has_chd & (bit+1)) )
			cur = wt->nodes[cur].cc[bit].chd;
		else
			break;
	}
	return rank;
}


static size_t _wavtree_select( wavtree *wt, size_t cur,
                               charcode_iter *codeit, size_t rank )
{
	if (cur>=wt->nnodes || wt->nodes[cur].len==0) return 0;
	size_t sel;
	byte_t bit;
	bit = charcode_iter_next(codeit);
	if ( !(wt->nodes[cur].has_chd & (bit+1)) ) {
		sel = csrsbitarr_select( wt->bitarr,
		                         wt->nodes[cur].cumul_bits[bit]+rank, bit )
		      - wt->nodes[cur].offset;
	} else {
		sel = _wavtree_select( wt, wt->nodes[cur].cc[bit].chd, codeit, rank );
		sel = csrsbitarr_select( wt->bitarr,
		                         wt->nodes[cur].cumul_bits[bit]+sel, bit )
		      - wt->nodes[cur].offset;
	}
	return MIN(sel, wt->nodes[cur].len);
}


size_t wavtree_select(wavtree *wt, xchar_t c, size_t rank)
{
	charcode_iter codeit = { .code=get_charcode(wt->chrcodes, c), .pos=0 };
	if (codeit.code==NULL_CODE) return wt->len;
	return _wavtree_select(wt, 0, &codeit, rank);
}


size_t wavtree_pred(wavtree *wt, size_t pos, xchar_t c)
{
	size_t rank = wavtree_rank(wt, pos, c);
	return (rank>0) ? wavtree_select(wt, c, rank-1) : wt->len;
}


size_t wavtree_succ(wavtree *wt, size_t pos, xchar_t c)
{
	if (pos >= wt->len) return wt->len;
	size_t rank = wavtree_rank(wt, pos, c);
	return (wavtree_char(wt,pos)==c) ? wavtree_select(wt, c, rank+1):
	       wavtree_select(wt, c, rank);
}


xchar_t wavtree_char(wavtree *wt, size_t pos)
{
	if ( pos >= wt->len ) return XEOF;
	size_t cur = 0;
	size_t rank = pos;
	byte_t bit;
	while ( true ) {
		bit = csrsbitarr_get(wt->bitarr, wt->nodes[cur].offset+rank);
		rank = csrsbitarr_rank(wt->bitarr, wt->nodes[cur].offset+rank, bit)
		       - wt->nodes[cur].cumul_bits[bit];
		if ( wt->nodes[cur].has_chd & (bit+1) )
			cur = wt->nodes[cur].cc[bit].chd;
		else
			break;
	}
	return wt->nodes[cur].cc[bit].chr;
}


// Print

void _wt_node_print(wavtree *wt, size_t cur, size_t depth)
{
	size_t i;
	strbuf *dmargin = strbuf_new_with_capacity(2*depth+2);
	for (i=0; i<depth; i++)
		strbuf_append(dmargin, "  ", 2);
	strbuf_append(dmargin, "| ", 2);
	char *margin = strbuf_detach(dmargin);
	if (cur >= wt->nnodes) {
		printf ("%s@wtree_node NULL\n",margin);
	} else {
		printf ("%s@wtree_node #%zu\n",margin, cur);
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
			_wt_node_print(wt, wt->nodes[cur].cc[LEFT].chd, depth+1);
		if (wt->nodes[cur].has_chd & (0x2) ) // has right chd
			_wt_node_print(wt, wt->nodes[cur].cc[RIGHT].chd, depth+1);
	}
}


void wavtree_print(wavtree *wt)
{
	char *shapes[2] = {"BAL", "HUFF"};
	printf ("wavelet_tree@%p {\n",wt);
	printf ("  tshape: %s\n",shapes[wt->shape]);
	printf ("  tree:\n");
	_wt_node_print(wt, 0, 0);
	//printf ("  bitarray:\n");
	//csrsbitarr_print(wt->bitarr, 4);
	if (ab_type(wt->ab)==CHAR_TYPE) {
		printf ("  char codes:\n");
		strbuf *codestr = strbuf_new_with_capacity(2);
		for (xchar_t c=0; c<=UCHAR_MAX; c++) {
			const bitvec *code = get_charcode(wt->chrcodes, c);
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
	printf ("} #end of wavelet_tree@%p\n\n",wt);
}


