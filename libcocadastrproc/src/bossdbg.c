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

#include "alphabet.h"
#include "arrays.h"
#include "bitarr.h"
#include "bitbyte.h"
#include "bytearr.h"
#include "new.h"
#include "csrsbitarr.h"
#include "cstrutil.h"
#include "bossdbg.h"
#include "vec.h"
#include "strbuf.h"
#include "mathutil.h"
#include "strstream.h"
#include "wavtree.h"
#include "xchar.h"


struct _BOSSdBG {
	Alphabet     *input_ab; // input alphabet
	Alphabet     *ext_ab;   // internal extended alphabet
	usize        k;
	bool          multi;
	usize        nnodes;
	usize        nedges;
	WavTree      *edge_lbl_wt;
	CSRSBitArr *true_node;
	usize       *char_cumul_count;
	CSRSBitArr *node_lbl_last_char;
};


static const xchar SENTINEL = 0;

// input to extended char conversion
static inline xchar inp2ext (Alphabet *input_ab, xchar c)
{
	return (xchar)(alphabet_rank(input_ab, c) + 1);
}

// extended to input char conversion
static inline xchar ext2inp (Alphabet *input_ab, xchar c)
{
	if (c == SENTINEL || c > alphabet_size(input_ab)) return XEOF;
	return alphabet_char(input_ab, c - 1);
}

static inline bool is_neg_chr(Alphabet *input_ab, xchar ec)
{
	return (ec > alphabet_size(input_ab));
}


static inline xchar neg_char(Alphabet *input_ab, xchar ec)
{
	return is_neg_chr(input_ab, ec) ? ec : ec + alphabet_size(input_ab);
}


static inline xchar pos_chr(Alphabet *input_ab, xchar ec)
{
	return is_neg_chr(input_ab, ec) ?  ec - alphabet_size(input_ab) : ec;
}


static Alphabet *get_ext_ab(Alphabet *input_ab)
{
	return alphabet_new_int_ab( 2 * alphabet_size(input_ab) + 1 );
}


typedef struct {
	xstr  *txt;
	usize    pos;
	usize    k;
} kmer_t;


kmer_t *kmer_new(xstr *txt, usize pos, usize k)
{
	kmer_t *ret = NEW(kmer_t);
	ret->txt = txt;
	ret->k = k;
	ret->pos = pos;
	return ret;
}


static usize kmer_key_fn(const void *kp, usize d)
{
	kmer_t *km = *(kmer_t **)kp;
	if ( d == 0 )
		return xstr_get(km->txt, km->pos + km->k - 1);
	else
		return xstr_get(km->txt, km->pos + d - 1 );
}


static void _init_cumul_char_count(BOSSdBG *graph, usize *cumul_char_count)
{
	graph->char_cumul_count = cumul_char_count;
	usize eabsize = alphabet_size(graph->ext_ab);
	assert(graph->nedges == cumul_char_count[eabsize]);
	//ARR_PRINT(cumul_char_count, cumul_char_count, %zu, 0, eabsize, eabsize);
	usize l = eabsize + graph->nedges;
	byte *bits = bitarr_new(l);
	for (usize i = 0; i < eabsize; i++)
		bitarr_set_bit(bits, cumul_char_count[i + 1] + i, 1);
	graph->node_lbl_last_char = csrsbitarr_new(bits, l);
	//csrsbitarr_fprint(graph->node_lbl_last_char, 4);
}




static BOSSdBG *_dbg_init( Alphabet *ab, StrStream *sst, usize k,
                           bool multigraph )
{
	Alphabet *ext_ab = get_ext_ab(ab);
	usize sizeof_ext_char = nbytes(alphabet_size(ext_ab));

	// build padded string with k sentinels at the beginning an one at the end
	xstr *padstr = xstr_new(sizeof_ext_char);
	for (usize i = 0; i < k; i++)
		xstr_push(padstr, SENTINEL);
	for (xchar c; (c = strstream_getc(sst)) != XEOF; )
		xstr_push(padstr, (xchar)(alphabet_rank(ab, c) + 1));
	xstr_push(padstr, SENTINEL);
	xstr_fit(padstr);
	//xstr_print(padstr);

	// build list of k+1mers
	Vec *kp1mers = vec_new(sizeof(kmer_t *));
	for (usize i = 0, padslen = xstr_len(padstr), l = padslen - (k + 1); i <= l;
	        i++) {
		kmer_t *kmer = kmer_new(padstr, i, k + 1);
		vec_push(kp1mers, &kmer);
	}

	//printf("kmers before sort:\n");
	//xstr *kmstr = xstr_new(sizeof_ext_char);
	//strbuf *kmdstr = strbuf_new();
	//for (usize i=0, l=vec_len(kp1mers); i<l; i++) {
	//    kmer_t *kp1mer = *(kmer_t **)vec_get(kp1mers, i);
	//    xstr_ncpy(kmstr, 0, kp1mer->txt, kp1mer->pos, k+1);
	//    xstr_to_string(kmstr, kmdstr);
	//    printf("kmer[%*zu]=%s\n",2, i, strbuf_as_str(kmdstr));
	//    strbuf_clear(kmdstr);
	//}
	//xstr_free(kmstr);
	//strbuf_free(kmdstr);

	// sort the k+1-mers
	vec_radixsort(kp1mers, &kmer_key_fn, k + 1, alphabet_size(ext_ab));

	//printf("kmers after sort:\n");
	//kmstr = xstr_new(sizeof_ext_char);
	//kmdstr = strbuf_new();
	//for (usize i=0, l=vec_len(kp1mers); i<l; i++) {
	//    kmer_t *kp1mer = *(kmer_t **)vec_get(kp1mers, i);
	//    xstr_ncpy(kmstr, 0, kp1mer->txt, kp1mer->pos, k+1);
	//    xstr_to_string(kmstr, kmdstr);
	//    printf("kmer[%*zu]=%s\n",2, i, strbuf_as_str(kmdstr));
	//    strbuf_clear(kmdstr);
	//}
	//xstr_free(kmstr);
	//strbuf_free(kmdstr);

	xstr *edge_labels  = xstr_new_with_capacity( sizeof_ext_char, xstr_len(padstr));
	byte *last_node   = bitarr_new(vec_len(kp1mers));
	usize *char_count = ARR_NEW(usize, alphabet_size(ext_ab) + 1);
	ARR_FILL(char_count, 0, alphabet_size(ext_ab) + 1, 0);

	usize nnodes = 0; // # of *distinct* nodes (k-mers)
	usize nedges = 0; // # of *distinct* edges (k+1-mers)

	byte *km1mers_chars = bitarr_new(sizeof_ext_char);
	xstr *lastkp1mers[2];
	lastkp1mers[0] = xstr_new_with_capacity(sizeof_ext_char, k + 1);
	lastkp1mers[1] = xstr_new_with_capacity(sizeof_ext_char, k + 1);
	xstr *lastkm1mers[2];
	lastkm1mers[0] = xstr_new_with_capacity(sizeof_ext_char, k - 1);
	lastkm1mers[1] = xstr_new_with_capacity(sizeof_ext_char, k - 1);
	usize this_line, last_line;
	kmer_t *kp1mer;
	bool new_edge = false;
	xchar edge_chr;

	// scan sorted k+1-mers to identify nodes and edges
	for (usize i = 0, nkp1mers = vec_len(kp1mers); i < nkp1mers; i++) {
		this_line = i % 2;
		last_line = (i + 1) % 2;
		kp1mer = *(kmer_t **)vec_get(kp1mers, i);
		xstr_ncpy(lastkp1mers[this_line], 0, kp1mer->txt, kp1mer->pos, k + 1);

		// compare this k+1-mer with the previous
		// if the first k chars are different from previous line,
		//              then it's a new node (and also necessarily a new edge)
		if (xstr_ncmp(lastkp1mers[this_line], lastkp1mers[last_line], k)) {
			if (nedges > 0)
				bitarr_set_bit(last_node, nedges - 1, 1);
			nnodes++;
			new_edge = true;
		}
		else {
			// else if the first k chars (node label) are the same, but the
			// last one (edge label) is different, then it is new edge
			if ( xstr_get(lastkp1mers[this_line], k)
			        != xstr_get(lastkp1mers[last_line], k) )
				new_edge = true;

			// otherwise, then the entire k+1-mer is the same, and so
			// it is neither a new node or edge,
			// unless we consider the dbg a multigraph
			else
				new_edge = multigraph;
		}

		// now, check the (k-1)-mers (suffix of the node label).
		// if the last node label suffix is different from previous
		// we clear edge labels marks and start afresh
		xstr_ncpy(lastkm1mers[this_line], 0, kp1mer->txt, kp1mer->pos + 1, k - 1);
		if (xstr_cmp(lastkm1mers[this_line], lastkm1mers[last_line])) {
			ARR_FILL(km1mers_chars, 0, sizeof_ext_char, 0x0);
		}

		// then, if we are adding another edge...
		if ( new_edge ) {
			nedges++;
			edge_chr = xstr_get(lastkp1mers[this_line], k);
			// ... and the edge char has already been marked with the same
			// node label suffix, the edge label should be its corresponding
			// extended char
			if (bitarr_get_bit(km1mers_chars, alphabet_rank(ext_ab, edge_chr)))
				edge_chr = neg_char(ab, edge_chr);
			// if not, then whe mark it.
			else
				bitarr_set_bit(km1mers_chars, alphabet_rank(ext_ab, edge_chr), 1);
			// whatever the case, set the new edge label char
			xstr_push(edge_labels, edge_chr);
			// and update the count of the last node label char
			char_count[alphabet_rank(ext_ab, xstr_get(lastkp1mers[this_line],
			                                               k - 1)) + 1]++;
		}
	}
	xstr_fit(edge_labels);
	//xstr_print(edge_labels);
	nedges = xstr_len(edge_labels);
	bitarr_set_bit(last_node, nedges - 1, 1);

	BOSSdBG *graph = NEW(BOSSdBG);
	graph->input_ab = ab;
	graph->ext_ab = ext_ab;
	//graph->txt = padstr;
	graph->k = k;
	graph->multi = multigraph;
	graph->nnodes = nnodes;
	graph->nedges = nedges;
	graph->edge_lbl_wt = wavtree_new_from_xstr( ext_ab, edge_labels,
	                     WT_HUFFMAN );
	graph->true_node = csrsbitarr_new(last_node, nedges);
	for (usize i = 1, l = alphabet_size(ext_ab) + 1; i < l; i++) {
		char_count[i] += char_count[i - 1];
	}
	_init_cumul_char_count(graph, char_count);

	//printf("edgelabels =",edge_labels );
	//wavtree_print(graph->edge_lbl_wt);

	// clean up temporary stuff
	xstr_free(padstr);
	xstr_free(edge_labels);
	DESTROY(kp1mers, finaliser_cons(FNR(vec), finaliser_new_ptr()));
	FREE(lastkm1mers[0]);
	FREE(lastkm1mers[1]);
	FREE(lastkp1mers[0]);
	FREE(lastkp1mers[1]);
	FREE(km1mers_chars);

	return graph;
}


BOSSdBG *bossdbg_new_from_str(Alphabet *ab, char *txt, usize k,
                              bool multigraph)
{
	StrStream *sst = strstream_open_str(txt, strlen(txt));
	BOSSdBG *dbg = _dbg_init(ab, sst, k, multigraph);
	strstream_close(sst);
	return dbg;
}


BOSSdBG *bossdbg_new_from_stream( Alphabet *ab, StrStream *sst, usize k,
                                  bool multigraph )
{
	return _dbg_init(ab, sst, k, multigraph);
}


void bossdbg_free(BOSSdBG *g)
{
	if (g == NULL) return;
	alphabet_free(g->ext_ab);
	wavtree_free(g->edge_lbl_wt);
	csrsbitarr_free(g->true_node, true);
	FREE(g->char_cumul_count);
	FREE(g);
}


char bossdbg_sentinel(BOSSdBG *g)
{
	return SENTINEL;
}


Alphabet *bossdbg_ab(BOSSdBG *g)
{
	return g->input_ab;
}

Alphabet *bossdbg_ext_ab(BOSSdBG *g)
{
	return g->ext_ab;
}


usize bossdbg_nnodes(BOSSdBG *g)
{
	return g->nnodes;
}


usize bossdbg_nedges(BOSSdBG *g)
{
	return g->nedges;
}


usize bossdbg_k(BOSSdBG *g)
{
	return g->k;
}


bool bossdbg_is_multigraph(BOSSdBG *g)
{
	return g->multi;
}


static usize _true_node(BOSSdBG *g, usize nid)
{
	if (csrsbitarr_get(g->true_node, nid) == 1)
		return nid;
	else
		return csrsbitarr_succ1(g->true_node, nid);
}


usize bossdbg_node_id(BOSSdBG *g, usize nrk)
{
	//assert(nrk<g->nnodes);
	return csrsbitarr_select1(g->true_node, nrk);
}


usize bossdbg_node_rank(BOSSdBG *g, usize nid)
{
	//assert(nid<g->nedges && csrsbitarr_get(g->true_node, nid));
	return csrsbitarr_rank1(g->true_node, nid);
}


static usize _last_node_char_rank(BOSSdBG *g, usize nid)
{
	usize p = csrsbitarr_select0(g->node_lbl_last_char, nid);
	return csrsbitarr_rank1(g->node_lbl_last_char, p);
}


void bossdbg_node_lbl(BOSSdBG *g, usize nid, xstr *dest)
{
	if (nid >= g->nedges) return;
	usize l = 0;
	for (usize i = 0; i < g->k; i++) {
		xstr_set(dest, i, SENTINEL);
	}
	for (usize cur = nid; l < g->k && 0 < cur && cur < g->nedges; l++) {
		usize crk = _last_node_char_rank(g, cur);
		xchar c = alphabet_char(g->ext_ab, crk);
		xstr_set(dest, g->k - 1 - l, c);
		cur = bossdbg_parent(g, cur);
	}
	xstr_clip(dest, 0, g->k);
}


usize bossdbg_outdeg(BOSSdBG *g, usize nid)
{
	if (nid == 0)
		return MIN(1, g->nnodes);
	return nid - csrsbitarr_pred1(g->true_node, nid);
}


usize bossdbg_lbl_outdeg(BOSSdBG *g, usize nid, xchar c)
{
	if (!alphabet_contains(g->input_ab, c)) return 0;
	xchar cp = inp2ext(g->input_ab, c);
	xchar cn = neg_char(g->input_ab, cp);
	usize ret = 0;
	ret = wavtree_rank(g->edge_lbl_wt, nid + 1, cp)
	      + wavtree_rank(g->edge_lbl_wt, nid + 1, cn);
	if (nid != 0) {
		usize prev = csrsbitarr_pred1(g->true_node, nid);
		ret -= ( wavtree_rank(g->edge_lbl_wt, prev + 1, cp)
		         + wavtree_rank(g->edge_lbl_wt, prev + 1, cn) );
	}
	return ret;
}


usize bossdbg_child(BOSSdBG *g, usize nid, xchar c)
{
	usize l = (nid == 0) ? 0 : csrsbitarr_pred1(g->true_node, nid) + 1;
	usize r = nid + 1;
	// nodes of the same label are in the range [l,r)
	// get the position p of edge label == c within this range
	usize p = wavtree_pred(g->edge_lbl_wt, r, c);
	if ( l <= p && p < r ) {
		usize crk = alphabet_rank(g->ext_ab, c);
		usize elrk = wavtree_rank_pos(g->edge_lbl_wt, p);
		usize past1 = (crk == 0) ? 0 : csrsbitarr_rank1(g->true_node,
		              g->char_cumul_count[crk]);
		usize chd = csrsbitarr_select1(g->true_node, past1 + elrk);
		return chd;
	}
	// if c not found in [l,r), try the extendedversion
	p = wavtree_pred(g->edge_lbl_wt, r, neg_char(g->input_ab, c));
	if ( l <= p && p < r ) {
		// if found, then by construction there is a preceding node
		// with same suffix that has an outgoing edge labeled c
		p = wavtree_pred(g->edge_lbl_wt, p, c);
		usize crk = alphabet_rank(g->ext_ab, c);
		usize elrk = wavtree_rank_pos(g->edge_lbl_wt, p);
		usize past1 = (crk == 0) ? 0 : csrsbitarr_rank1( g->true_node,
		              g->char_cumul_count[crk]);
		usize chd = csrsbitarr_select1(g->true_node, past1 + elrk);
		return chd;
	}
	// if the extended version also not found, then return a null id
	return g->nedges;
}


usize bossdbg_parent(BOSSdBG *g, usize nid)
{
	if (nid == 0)
		return g->nedges;
	usize  crk = _last_node_char_rank(g, nid);
	xchar c   = alphabet_char(g->ext_ab, crk);
	usize  r   = csrsbitarr_rank1(g->true_node, nid)
	             - csrsbitarr_rank1(g->true_node, g->char_cumul_count[crk]);
	usize par  = wavtree_select(g->edge_lbl_wt, c, r);
	return _true_node(g, par);
}



static void node_cstr(xstr *node, Alphabet *ab, char *dest)
{
	for (usize i = 0, l = xstr_len(node); i < l; i++ ) {
		xchar c = xstr_get(node, i);
		dest[i] = (c == SENTINEL) ? '$' : ext2inp(ab, c);
	}
	dest[xstr_len(node)] = '\0';
}

void bossdbg_print(BOSSdBG *g)
{
	printf("dbgraph@%p\n", g);
	//csrsbitarr_fprint(g->node_lbl_last_char, 10);
	usize ncols = 4;
	char *headers[4] = {"nid", "real", "node", "edge"};
	int *cols = ARR_NEW(int, ncols);
	for (usize c = 0; c < ncols; c++)
		cols[c] = strlen(headers[c]);
	cols[0] = MAX(cols[0], (int)ceil(log(g->nedges)) + 1);
	cols[2] = MAX(cols[2], g->k);

	char *edge, *node;
	xstr *xnode = xstr_new_with_capacity(nbytes(alphabet_size(g->ext_ab)), g->k);
	xstr_push_n(xnode, 0, g->k);
	node = cstr_new(g->k);
	cstr_fill(node, 0, g->k, '?');
	edge = cstr_new(2);
	printf("%*s %*s %*s %*s\n",
	       cols[0], headers[0],
	       cols[1], headers[1],
	       cols[2], headers[2],
	       cols[3], headers[3]);
	for (usize i = 0; i < g->nedges; i++) {
		bossdbg_node_lbl(g, _true_node(g, i), xnode);
		node_cstr(xnode, g->input_ab, node);
		char e = wavtree_char(g->edge_lbl_wt, i);
		if (is_neg_chr(g->input_ab, e)) {
			edge[0] = '-';
			edge[1] = ext2inp(g->input_ab, pos_chr(g->input_ab, e));
		}
		else {
			edge[0] = ' ';
			edge[1] = e == 0 ? '$' : alphabet_char(g->input_ab, e - 1);
		}
		printf("%*zu %*c %*s %*s\n",
		       cols[0], i,
		       cols[1], csrsbitarr_get(g->true_node, i) ? '1' : '0',
		       cols[2], node,
		       cols[3], edge);
	}
	for (usize i = 0, l = alphabet_size(g->input_ab) + 1; i < l; i++) {
		printf( "cumul_count[%c]=%zu\n", (char)alphabet_char(g->ext_ab, i),
		        g->char_cumul_count[i] );
	}
}

