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
#include "arrayutil.h"
#include "binheap.h"
#include "bitsandbytes.h"
#include "bitarray.h"
#include "bitvector.h"
#include "bytearray.h"
#include "new.h"
#include "cstringutil.h"
#include "dynstr.h"
#include "huffcode.h"
#include "mathutil.h"
#include "strstats.h"
#include "strstream.h"

static const byte_t LEFT  = 0;
static const byte_t RIGHT = 1;

struct _hufftnode {
    size_t     chr_rank;
    hufftnode* chd[2];
    byte_t    *ab_mask;
};


struct _huffcode {
    alphabet  *ab;
    bool       own_ab;
    size_t     size;
    hufftnode *tree;
    bitvector **code;
};


typedef struct {
    size_t freq; // must come first
    size_t node;
} nodefreq;


static int nodefreq_cmp(const void *p1, const void *p2)
{
    return (*((size_t *)p1) - *((size_t *)p2));
}



static void fill_code_table( huffcode *hcode, hufftnode *node, size_t code_len,
                             byte_t *code )
{
    if (hufftnode_is_leaf(node)) {
        hcode->code[node->chr_rank] = bitvector_new_from_bitarray(code, code_len);
        //printf("code of %c = %s\n",ab_char(hcode->ab, node->chr_rank), code);
    }
    else {
        bitarr_set_bit(code, code_len, 0);
        fill_code_table(hcode, hufftnode_left(node), code_len+1, code);
        bitarr_set_bit(code, code_len, 1);
        fill_code_table(hcode, hufftnode_right(node), code_len+1, code);
    }
}


huffcode *huffcode_new(alphabet *ab, size_t freqs[])
{
    huffcode *hcode;

    hcode = NEW(huffcode);
    hcode->ab = ab;
    hcode->own_ab = false;
    hcode->size = ab_size(ab);

    size_t ab_bytesize = (size_t)multceil(hcode->size, BYTESIZE);
    hcode->tree = NEW_ARRAY(hufftnode, MAX(0, 2*hcode->size-1));
    for (size_t i = 0; i < hcode->size; i++) {
        hcode->tree[i].chr_rank = i;
        hcode->tree[i].chd[LEFT]     = &hcode->tree[i];
        hcode->tree[i].chd[RIGHT]    = &hcode->tree[i];
        hcode->tree[i].ab_mask  = bytearr_new(ab_bytesize);
        bitarr_set_bit(hcode->tree[i].ab_mask, i, 1);
    }

    binheap *nfheap = binheap_new(&nodefreq_cmp, sizeof(nodefreq), MIN_HEAP);
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

    hcode->code = NEW_ARRAY(bitvector*, hcode->size);
    byte_t *chrcode = bitarr_new(hcode->size);
    if (hcode->size>0) 
        fill_code_table(hcode, huffcode_tree(hcode), 0, chrcode);
    FREE(chrcode);
    return hcode;
}


huffcode *huffcode_new_from_str(alphabet *ab, char *src)
{
    strstream *sst = strstream_open_str(src, strlen(src));
    huffcode *hcode = huffcode_new_from_stream(ab, sst);
    strstream_close(sst);
    return hcode;
}


huffcode *huffcode_new_from_xstr(alphabet *ab, xstring *src)
{
    strstream *sst = strstream_open_xstr(src);
    huffcode *hcode = huffcode_new_from_stream(ab, sst);
    strstream_close(sst);
    return hcode;
}


huffcode *huffcode_new_from_stream(alphabet *ab, strstream *sst)
{
    size_t *freqs = char_count_stream(sst, ab);
    huffcode *hcode = huffcode_new(ab, freqs);
    FREE(freqs);
    return hcode;
}


huffcode *huffcode_new_online_from_stream(alphabet_type abtype, strstream *sst)
{  
    strstream_reset(sst);
    size_t szofchar = strstream_sizeof_char(sst);
    size_t all_len = 1 << (szofchar*BYTESIZE);
    size_t *all_freqs = NEW_ARRAY(size_t, all_len);
    size_t ab_len = 0;
    FILL_ARRAY(all_freqs, 0, all_len ,0);
    xchar_t xcmax = 0;
    for (xchar_t c; (c=strstream_getc(sst))!=XEOF;) {
        ab_len += (all_freqs[c]==0)?1:0;
        all_freqs[c] += 1;
        xcmax = MAX(xcmax, c);
    }
    alphabet *ab = NULL;
    size_t   *ab_freqs;
    switch(abtype) {
    case CHAR_TYPE:
        ;
        char *ab_str = cstr_new(ab_len);
        ab_freqs = NEW_ARRAY(size_t, ab_len);
        for (size_t i=0, k=0; i<all_len; i++) {
            if (all_freqs[i] > 0) {
                ab_str[k]  = (char)i;
                ab_freqs[k] = all_freqs[i];
                k++;
            }
        }
        ab = alphabet_new(ab_len, ab_str);
        FREE(all_freqs);
        break;
    case INT_TYPE:
        ab_freqs = realloc(all_freqs, (xcmax+1)*sizeof(size_t));
        ab = int_alphabet_new(xcmax+1);
        break;
    }
    huffcode *hcode = huffcode_new(ab, ab_freqs);
    hcode->own_ab = true;
    FREE(ab_freqs);
    return hcode;
}



void huffcode_free(huffcode *hcode)
{
    if (hcode==NULL) return;
    if (hcode->own_ab)
        alphabet_free(hcode->ab);
    for (size_t i=0; i<hcode->size; i++) {
        bitvector_free(hcode->code[i]); // no null codes
    }
    FREE(hcode->code);
    for (size_t i=0; hcode->size>0 && i<(2*hcode->size)-1; i++) {
        if (hcode->tree[i].ab_mask)
            FREE(hcode->tree[i].ab_mask);
    }
    FREE(hcode->tree);
    FREE(hcode);
}


void _print_htree(huffcode *hc, hufftnode *node, size_t level, char *code)
{
    if (node==NULL) return;
    char *space = cstr_new(4*level);
    cstr_fill(space, 0, 4*level, ' ');
    if (hufftnode_is_leaf(node)) {
        printf("%s[%p code=%s chr=%c(%d)]\n", space, node, code, 
                ab_char(hc->ab, node->chr_rank), (int)(ab_char(hc->ab, node->chr_rank)));
        //bytearr_print(hufftnode_ab_mask(node), (size_t)mult_ceil(ab_size(hc->ab), BYTESIZE), 4, space);
    }
    else {
        printf("%s[%p code=%s]\n", space, node, code);
        //bytearr_print(hufftnode_ab_mask(node), (size_t)mult_ceil(ab_size(hc->ab), BYTESIZE), 4, space);
        char *ccode = cstr_new(level+1);
        strcpy(ccode, code);
        ccode[level] = '0';
        _print_htree(hc, hufftnode_left(node), level+1, ccode);
        ccode[level] = '1';
        _print_htree(hc, hufftnode_right(node), level+1, ccode);
    }
    FREE(space);
}


void huffcode_print(huffcode *hcode)
{
    printf("huffcode@%p {\n",(void *)hcode);
    printf("    size: %zu\n",hcode->size);
    _print_htree(hcode, huffcode_tree(hcode), 0, "");
    printf("    codes:\n");
    switch ( ab_type(hcode->ab) ) {
    case CHAR_TYPE:
        for (size_t i=0; i<hcode->size; i++) {
            printf("%c: ", ab_char(hcode->ab, i) );
            bitvec_print(hcode->code[i], 8);    
        }
        break;
    case INT_TYPE:
        for (size_t i=0; i<hcode->size; i++) {
            printf(XCHAR_FMT": ", ab_char(hcode->ab, i) );
            bitvec_print(hcode->code[i], 8);    
        }
        break;
    }
    printf("} // end of huffcode@%p\n",(void *)hcode);
}


bitvector *huffcode_encode(huffcode *hcode, strstream *sst) 
{
    bitvector *enc = bitvector_new();
    for (xchar_t c; (c=strstream_getc(sst))!=XEOF;) {
        bitvec_cat(enc, hcode->code[ab_rank(hcode->ab, c)]);
    }
    return enc;
}


xstring *huffcode_decode(huffcode *hcode, bitvector *bcode)
{
    xstring *dec = xstring_new(nbytes(ab_size(hcode->ab)));
    hufftnode *cur = huffcode_tree(hcode);
    for (size_t i=0, l=bitvec_len(bcode); i<l; i++) {
        cur = cur->chd[bitvec_get_bit(bcode, i)];
        if (hufftnode_is_leaf(cur)) {
            xstr_push(dec, ab_char(hcode->ab, cur->chr_rank));
            cur = huffcode_tree(hcode);
        }
    }
    return dec;
}


const bitvector *huffcode_charcode(huffcode *hcode, size_t char_rank)
{
    return hcode->code[char_rank];
} 


hufftnode *huffcode_tree(huffcode *code)
{
    return (code->size>0)?code->tree+(2*code->size)-2:NULL;
}

alphabet *huffcode_ab(huffcode *code)
{
    return code->ab;
}


bool hufftnode_is_leaf(hufftnode *node)
{
    return node->chd[LEFT]==node->chd[RIGHT];
}


hufftnode *hufftnode_left(hufftnode *node)
{
    return node->chd[LEFT];
}


hufftnode *hufftnode_right(hufftnode *node)
{
    return node->chd[RIGHT];
}


byte_t *hufftnode_ab_mask(hufftnode *node)
{
    return node->ab_mask;
}

size_t hufftnode_char_rank(hufftnode *node)
{
    return node->chr_rank;
}
