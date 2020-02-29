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
#include "arrayutil.h"
#include "bitarray.h"
#include "bitsandbytes.h"
#include "bytearray.h"
#include "new.h"
#include "csrsbitarray.h"
#include "cstringutil.h"
#include "debruijngraph.h"
#include "vec.h"
#include "dynstr.h"
#include "mathutil.h"
#include "strstream.h"
#include "wavtree.h"
#include "xchar.h"


struct _dbgraph {
    alphabet     *input_ab; // input alphabet 
    alphabet     *ext_ab;   // internal extended alphabet
    size_t        k;
    bool          multi;
    size_t        nnodes;
    size_t        nedges;
    wavtree      *edge_lbl_wt;
    csrsbitarray *true_node;
    size_t       *char_cumul_count;
    csrsbitarray *node_lbl_last_char;
};


static const xchar_t SENTINEL = 0;


// input to extended char conversion
static inline xchar_t inp2ext (alphabet *input_ab, xchar_t c)
{
    return (xchar_t)(ab_rank(input_ab, c) + 1);
}

// extended to input char conversion
static inline xchar_t ext2inp (alphabet *input_ab, xchar_t c)
{
    if (c==SENTINEL || c>ab_size(input_ab)) return XEOF;
    return ab_char(input_ab, c - 1);    
}

static inline bool is_neg_chr(alphabet *input_ab, xchar_t ec)
{
    return (ec > ab_size(input_ab));
}


static inline xchar_t neg_char(alphabet *input_ab, xchar_t ec)
{
    return is_neg_chr(input_ab, ec) ? ec : ec + ab_size(input_ab);
}


static inline xchar_t pos_chr(alphabet *input_ab, xchar_t ec)
{
    return is_neg_chr(input_ab, ec) ?  ec - ab_size(input_ab) : ec;
}


static alphabet *get_ext_ab(alphabet *input_ab) {
    return int_alphabet_new( 2 * ab_size(input_ab) + 1 );
}


typedef struct {
    xstring  *txt;
    size_t    pos;
    size_t    k;
} kmer_t;


kmer_t *kmer_new(xstring *txt, size_t pos, size_t k)
{
    kmer_t *ret = NEW(kmer_t);
    ret->txt = txt;
    ret->k = k;
    ret->pos = pos;
    return ret;
}


void kmer_free(kmer_t *kmer)
{
    FREE(kmer);
}


static size_t kmer_key_fn(const void *kp, size_t d)
{
    kmer_t *km = *(kmer_t **)kp;
    if ( d == 0 )
        return xstr_get(km->txt, km->pos + km->k - 1);
    else
        return xstr_get(km->txt, km->pos + d - 1 );
}


static void _init_cumul_char_count(dbgraph *graph, size_t *cumul_char_count)
{
    graph->char_cumul_count = cumul_char_count;
    size_t eabsize = ab_size(graph->ext_ab);
    assert(graph->nedges == cumul_char_count[eabsize]);
    //PRINT_ARRAY(cumul_char_count, cumul_char_count, %zu, 0, eabsize, eabsize);
    size_t l = eabsize + graph->nedges;
    byte_t *bits = bitarr_new(l);
    for (size_t i=0; i<eabsize; i++) 
        bitarr_set_bit(bits, cumul_char_count[i+1]+i, 1);
    graph->node_lbl_last_char = csrsbitarr_new(bits, l);
    //csrsbitarr_print(graph->node_lbl_last_char, 4);
}




static dbgraph *_dbg_init( alphabet *ab, strstream *sst, size_t k, 
                           bool multigraph )
{
    alphabet *ext_ab = get_ext_ab(ab);
    size_t sizeof_ext_char = nbytes(ab_size(ext_ab));
    
    // build padded string with k sentinels at the beginning an one at the end
    xstring *padstr = xstring_new(sizeof_ext_char);
    for (size_t i=0; i<k; i++)
        xstr_push(padstr, SENTINEL);
    for (xchar_t c; (c=strstream_getc(sst))!=XEOF; )
        xstr_push(padstr, (xchar_t)(ab_rank(ab, c) + 1));
    xstr_push(padstr, SENTINEL);
    xstr_trim(padstr);
    //xstr_print(padstr);

    // build list of k+1mers
    vec *kp1mers = vec_new(sizeof(kmer_t *));
    for (size_t i=0, padslen=xstr_len(padstr), l=padslen-(k+1); i<=l; i++) {
        kmer_t *kmer = kmer_new(padstr, i, k+1);
        vec_app(kp1mers, &kmer);
    }

    //printf("kmers before sort:\n");
    //xstring *kmstr = xstring_new(sizeof_ext_char);
    //dynstr *kmdstr = dynstr_new();
    //for (size_t i=0, l=vec_len(kp1mers); i<l; i++) {
    //    kmer_t *kp1mer = *(kmer_t **)vec_get(kp1mers, i);
    //    xstr_ncpy(kmstr, 0, kp1mer->txt, kp1mer->pos, k+1);
    //    xstr_to_string(kmstr, kmdstr);
    //    printf("kmer[%*zu]=%s\n",2, i, dstr_as_str(kmdstr));
    //    dstr_clear(kmdstr);
    //}
    //xstring_free(kmstr);
    //dynstr_free(kmdstr);

    // sort the k+1-mers
    vec_radixsort(kp1mers, &kmer_key_fn, k+1, ab_size(ext_ab));

    //printf("kmers after sort:\n");
    //kmstr = xstring_new(sizeof_ext_char);
    //kmdstr = dynstr_new();
    //for (size_t i=0, l=vec_len(kp1mers); i<l; i++) {
    //    kmer_t *kp1mer = *(kmer_t **)vec_get(kp1mers, i);
    //    xstr_ncpy(kmstr, 0, kp1mer->txt, kp1mer->pos, k+1);
    //    xstr_to_string(kmstr, kmdstr);
    //    printf("kmer[%*zu]=%s\n",2, i, dstr_as_str(kmdstr));
    //    dstr_clear(kmdstr);
    //}
    //xstring_free(kmstr);
    //dynstr_free(kmdstr);
    
    xstring *edge_labels  = xstring_new_with_capacity( xstr_len(padstr), 
                                                       sizeof_ext_char );
    byte_t *last_node   = bitarr_new(vec_len(kp1mers));
    size_t *char_count = NEW_ARRAY(size_t, ab_size(ext_ab)+1);
    FILL_ARRAY(char_count, 0, ab_size(ext_ab)+1, 0);

    size_t nnodes = 0; // # of *distinct* nodes (k-mers)
    size_t nedges = 0; // # of *distinct* edges (k+1-mers)

    byte_t *km1mers_chars = bitarr_new(sizeof_ext_char);
    xstring *lastkp1mers[2];
    lastkp1mers[0] = xstring_new_with_capacity(k+1, sizeof_ext_char);
    lastkp1mers[1] = xstring_new_with_capacity(k+1, sizeof_ext_char);
    xstring *lastkm1mers[2];
    lastkm1mers[0] = xstring_new_with_capacity(k-1, sizeof_ext_char);
    lastkm1mers[1] = xstring_new_with_capacity(k-1, sizeof_ext_char);
    size_t this_line, last_line;
    kmer_t *kp1mer;
    bool new_edge = false;
    xchar_t edge_chr;

    // scan sorted k+1-mers to identify nodes and edges
    for (size_t i=0, nkp1mers = vec_len(kp1mers); i<nkp1mers; i++) {
        this_line = i%2;
        last_line = (i+1)%2;
        kp1mer = *(kmer_t **)vec_get(kp1mers, i);
        xstr_ncpy(lastkp1mers[this_line], 0, kp1mer->txt, kp1mer->pos, k+1);

        // compare this k+1-mer with the previous
        // if the first k chars are different from previous line,
        //              then it's a new node (and also necessarily a new edge)
        if (xstr_ncmp(lastkp1mers[this_line], lastkp1mers[last_line], k)) {
            if (nedges > 0)
                bitarr_set_bit(last_node, nedges-1, 1);
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
        xstr_ncpy(lastkm1mers[this_line], 0, kp1mer->txt, kp1mer->pos+1, k-1);
        if (xstr_cmp(lastkm1mers[this_line], lastkm1mers[last_line])) {
            FILL_ARRAY(km1mers_chars, 0, sizeof_ext_char, 0x0);
        }

        // then, if we are adding another edge...
        if ( new_edge ) {
            nedges++;
            edge_chr = xstr_get(lastkp1mers[this_line], k);
            // ... and the edge char has already been marked with the same
            // node label suffix, the edge label should be its corresponding
            // extended char
            if (bitarr_get_bit(km1mers_chars, ab_rank(ext_ab, edge_chr))) 
                edge_chr = neg_char(ab, edge_chr);
            // if not, then whe mark it.
            else 
                bitarr_set_bit(km1mers_chars, ab_rank(ext_ab, edge_chr), 1);
            // whatever the case, set the new edge label char
            xstr_push(edge_labels, edge_chr);
            // and update the count of the last node label char
            char_count[ab_rank(ext_ab, xstr_get(lastkp1mers[this_line], k-1)) + 1]++;
        }
    }
    xstr_trim(edge_labels);
    //xstr_print(edge_labels);
    nedges = xstr_len(edge_labels);
    bitarr_set_bit(last_node, nedges-1, 1);

    dbgraph *graph = NEW(dbgraph);
    graph->input_ab = ab;
    graph->ext_ab = ext_ab;
    //graph->txt = padstr;
    graph->k = k;
    graph->multi = multigraph;
    graph->nnodes = nnodes;
    graph->nedges = nedges;
    graph->edge_lbl_wt = wavtree_new_from_xstring( ext_ab, edge_labels, 
                                                   WT_HUFFMAN );
    graph->true_node = csrsbitarr_new(last_node, nedges);
    for (size_t i=1, l=ab_size(ext_ab)+1; i<l; i++) {
        char_count[i] += char_count[i-1];
    }
    _init_cumul_char_count(graph, char_count);

    //printf("edgelabels =",edge_labels );
    //wavtree_print(graph->edge_lbl_wt);
    
    // clean up temporary stuff
    xstring_free(padstr);
    xstring_free(edge_labels);
    vec_free(kp1mers, true);
    FREE(lastkm1mers[0]);
    FREE(lastkm1mers[1]);
    FREE(lastkp1mers[0]);
    FREE(lastkp1mers[1]);
    FREE(km1mers_chars);

    return graph;
}


dbgraph *dbg_new_from_str(alphabet *ab, char *txt, size_t k, bool multigraph)
{
    strstream *sst = strstream_open_str(txt, strlen(txt));
    dbgraph *dbg = _dbg_init(ab, sst, k, multigraph);
    strstream_close(sst);
    return dbg;
}


dbgraph *dbg_new_from_stream( alphabet *ab, strstream *sst, size_t k,
                              bool multigraph )
{
    return _dbg_init(ab, sst, k, multigraph);
}


void dbg_free(dbgraph *g)
{
    if (g==NULL) return;
    alphabet_free(g->ext_ab);
    wavtree_free(g->edge_lbl_wt);
    csrsbitarr_free(g->true_node, true);
    FREE(g->char_cumul_count);
    FREE(g);
}


char dbg_sentinel(dbgraph *g)
{
    return SENTINEL;
}


alphabet *dbg_ab(dbgraph *g)
{
    return g->input_ab;
}

alphabet *dbg_ext_ab(dbgraph *g)
{
    return g->ext_ab;
}


size_t dbg_nnodes(dbgraph *g)
{
    return g->nnodes;
}


size_t dbg_nedges(dbgraph *g)
{
    return g->nedges;
}


size_t dbg_k(dbgraph *g)
{
    return g->k;
}


bool dbg_is_multigraph(dbgraph *g)
{
    return g->multi;
}


static size_t _true_node(dbgraph *g, size_t nid)
{
    if (csrsbitarr_get(g->true_node, nid)==1)
        return nid;
    else
        return csrsbitarr_succ1(g->true_node, nid);
}


size_t dbg_node_id(dbgraph *g, size_t nrk)
{
    //assert(nrk<g->nnodes);
    return csrsbitarr_select1(g->true_node, nrk);
}


size_t dbg_node_rank(dbgraph *g, size_t nid)
{
    //assert(nid<g->nedges && csrsbitarr_get(g->true_node, nid));
    return csrsbitarr_rank1(g->true_node, nid);
}


static size_t _last_node_char_rank(dbgraph *g, size_t nid)
{
    size_t p = csrsbitarr_select0(g->node_lbl_last_char, nid);
    return csrsbitarr_rank1(g->node_lbl_last_char, p);
}


void dbg_node_lbl(dbgraph *g, size_t nid, xstring *dest)
{
    if (nid >= g->nedges) return;
    size_t l=0;
    xstr_nset(dest, g->k, SENTINEL);
    for (size_t cur=nid; l<g->k && 0<cur && cur<g->nedges; l++) {
        size_t crk = _last_node_char_rank(g, cur);
        xchar_t c = ab_char(g->ext_ab, crk);
        xstr_set(dest, g->k-1-l, c);
        cur = dbg_parent(g, cur);
    }
    xstr_clip(dest, 0, g->k);
}


size_t dbg_outdeg(dbgraph *g, size_t nid)
{
    if (nid==0) 
        return MIN(1, g->nnodes);
    return nid-csrsbitarr_pred1(g->true_node, nid);
}


size_t dbg_lbl_outdeg(dbgraph *g, size_t nid, xchar_t c)
{
    if (!ab_contains(g->input_ab, c)) return 0;
    xchar_t cp = inp2ext(g->input_ab, c);
    xchar_t cn = neg_char(g->input_ab, cp);
    size_t ret = 0;
    ret = wavtree_rank(g->edge_lbl_wt, nid+1, cp)  
          + wavtree_rank(g->edge_lbl_wt, nid+1, cn);
    if (nid!=0) { 
        size_t prev = csrsbitarr_pred1(g->true_node, nid);
        ret -= ( wavtree_rank(g->edge_lbl_wt, prev+1, cp) 
                 + wavtree_rank(g->edge_lbl_wt, prev+1, cn) );
    }
    return ret;
}


size_t dbg_child(dbgraph *g, size_t nid, xchar_t c)
{
    size_t l = (nid==0)?0:csrsbitarr_pred1(g->true_node, nid)+1;
    size_t r = nid+1;
    // nodes of the same label are in the range [l,r)
    // get the position p of edge label == c within this range
    size_t p = wavtree_pred(g->edge_lbl_wt, r, c);
    if ( l<=p && p<r ) {
        size_t crk = ab_rank(g->ext_ab, c);
        size_t elrk = wavtree_rank_pos(g->edge_lbl_wt, p);
        size_t past1 = (crk==0)?0:csrsbitarr_rank1(g->true_node,
                                                   g->char_cumul_count[crk]);
        size_t chd = csrsbitarr_select1(g->true_node, past1+elrk);
        return chd;
    }
    // if c not found in [l,r), try the extendedversion
    p = wavtree_pred(g->edge_lbl_wt, r, neg_char(g->input_ab, c));
    if ( l<=p && p<r ) {
        // if found, then by construction there is a preceding node
        // with same suffix that has an outgoing edge labeled c
        p = wavtree_pred(g->edge_lbl_wt, p, c);
        size_t crk = ab_rank(g->ext_ab, c);
        size_t elrk = wavtree_rank_pos(g->edge_lbl_wt, p);
        size_t past1 = (crk==0)?0:csrsbitarr_rank1( g->true_node,
                       g->char_cumul_count[crk]);
        size_t chd = csrsbitarr_select1(g->true_node, past1+elrk);
        return chd;
    }
    // if the extended version also not found, then return a null id
    return g->nedges;
}


size_t dbg_parent(dbgraph *g, size_t nid)
{
    if (nid==0) 
        return g->nedges; 
    size_t  crk = _last_node_char_rank(g, nid);
    xchar_t c   = ab_char(g->ext_ab, crk);
    size_t  r   = csrsbitarr_rank1(g->true_node, nid) 
                  - csrsbitarr_rank1(g->true_node, g->char_cumul_count[crk]);
    size_t par  = wavtree_select(g->edge_lbl_wt, c, r);
    return _true_node(g, par);
}



static void node_cstr(xstring *node, alphabet *ab, char *dest)
{
    for (size_t i=0, l=xstr_len(node); i<l; i++ ) {
        xchar_t c = xstr_get(node, i);
        dest[i] = (c==SENTINEL) ? '$' : ext2inp(ab, c);
    }
    dest[xstr_len(node)] = '\0';
}

void dbg_print(dbgraph *g)
{
    printf("dbgraph@%p\n",g);
    //csrsbitarr_print(g->node_lbl_last_char, 10);
    size_t ncols = 4;
    char *headers[4] = {"nid", "real", "node", "edge"};
    int *cols = NEW_ARRAY(int, ncols);
    for (size_t c=0; c<ncols; c++)
        cols[c] = strlen(headers[c]);
    cols[0] = MAX(cols[0], (int)ceil(log(g->nedges))+1);
    cols[2] = MAX(cols[2], g->k);
    
    char *edge, *node;
    xstring *xnode = xstring_new_with_len(g->k, nbytes(ab_size(g->ext_ab)));
    node = cstr_new(g->k);
    cstr_fill(node, 0, g->k, '?');
    edge = cstr_new(2);
    printf("%*s %*s %*s %*s\n",
               cols[0], headers[0],
               cols[1], headers[1],
               cols[2], headers[2],
               cols[3], headers[3]);
    for (size_t i=0; i<g->nedges; i++) {
        dbg_node_lbl(g, _true_node(g, i), xnode);
        node_cstr(xnode, g->input_ab, node);
        char e = wavtree_char(g->edge_lbl_wt, i);
        if (is_neg_chr(g->input_ab, e)) {
            edge[0] = '-';
            edge[1] = ext2inp(g->input_ab, pos_chr(g->input_ab, e));
        }
        else {
            edge[0] = ' ';
            edge[1] = e==0 ? '$' : ab_char(g->input_ab, e-1);
        }
        printf("%*zu %*c %*s %*s\n",
               cols[0], i,
               cols[1], csrsbitarr_get(g->true_node, i)?'1':'0',
               cols[2], node, 
               cols[3], edge);
    }
    for (size_t i=0, l=ab_size(g->input_ab)+1; i<l; i++) {
        printf( "cumul_count[%c]=%zu\n", (char)ab_char(g->ext_ab, i), 
                g->char_cumul_count[i] );
    }
}

