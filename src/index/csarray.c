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

#include <stdlib.h>
#include <stdio.h>

#include "alphabet.h"
#include "assert.h"
#include "arrutil.h"
#include "bitsandbytes.h"
#include "bitarr.h"
#include "bitvec.h"
#include "bytearr.h"
#include "new.h"
#include "csarray.h"
#include "csrsbitarray.h"
#include "strbuf.h"
#include "math.h"
#include "mathutil.h"
#include "sais.h"
#include "strstream.h"
#include "wavtree.h"
#include "xchar.h"
#include "xstring.h"

#define MIN_NLEVELS 1
#define MAX_PLAIN_SA_LEN 3

static const char SENTINEL='$';


struct _csarray {
	alphabet *xab;
	size_t nlevels;
	size_t *lvl_len;
	csrsbitarray **even_bv;
	csrsbitarray **char_stop_bv;
	wavtree **phi_wt;
	size_t *root_sa;
	size_t *root_sa_inv;
	//xstring **phi_str;
};


static void sarr_invert(size_t *src, size_t len, size_t *dest)
{
	for (size_t i=0; i<len; i++)
		dest[src[i]] = i;
}


csarray *csarray_new( char *str, size_t len, alphabet *ab )
{
	csarray *csa = NEW(csarray);

	csa->nlevels = 1;  // # of levels, including root level
	for ( size_t lvl_len = len+1; lvl_len > MAX_PLAIN_SA_LEN;
	        lvl_len = (size_t) ceil(lvl_len/2.0f) )
		csa->nlevels++;

	csa->lvl_len = NEW_ARR(size_t, csa->nlevels);
	csa->even_bv = NEW_ARR(csrsbitarray*, csa->nlevels);
	csa->char_stop_bv = NEW_ARR(csrsbitarray*, csa->nlevels);
	csa->phi_wt = NEW_ARR(wavtree*, csa->nlevels);
	//csa->phi_str = NEW_ARR(xstring*, csa->nlevels);

	// build plain sarray and its inverse
	size_t *sarr = sais(str, len, ab);
	size_t *sarr_inv = NEW_ARR(size_t, len+1);
	sarr_invert(sarr, len+1, sarr_inv);

	size_t lvl_len = len+1;  // sentinel added by sais
	csa->lvl_len[0] = lvl_len;
	bitvec *xchar_stops = bitvec_new_with_capacity(lvl_len);
	bitvec_push_n(xchar_stops, lvl_len, 0);
	strbuf *supp_ab_str = strbuf_new(); // string support alphabet chars
	size_t ndiff_xchars = 1;            // has at least the SENTINEL
	strbuf_append_char(supp_ab_str, SENTINEL);
	bitvec_set_bit(xchar_stops, 0, 1);
	bitvec_set_bit(xchar_stops, lvl_len-1, 1);
	//ndiff_xchars++;
	for (size_t i=1; i<lvl_len; i++) {
		if (str[sarr[i]] != str[sarr[i-1]]) {
			bitvec_set_bit(xchar_stops, i-1, 1);
			strbuf_append_char(supp_ab_str, str[sarr[i]]);
			ndiff_xchars++;
		}
	}
	csa->xab = alphabet_new(ndiff_xchars, strbuf_detach(supp_ab_str));

	// convert source string to "normalised" xstring
	assert(ndiff_xchars < XCHAR_MAX);
	xstring *cur_xstr = xstring_new_with_len(lvl_len, nbytes(ndiff_xchars));
	xchar_t cur_xchar = 0;
	for (size_t i=0; i<lvl_len; i++) {
		xstr_set(cur_xstr, sarr[i], cur_xchar);
		if ( bitvec_get_bit(xchar_stops, i) )
			cur_xchar++;
	}

	//printf("str[0]: %s\n", str);
	//xstr_print(cur_xstr);
	//PRINT_ARR(sarr, sarr[0], %zu, 0, lvl_len, lvl_len);
	//PRINT_ARR(sarr_inv, sarr_inv[0], %zu, 0, lvl_len, lvl_len);

	// iterate over levels
	for ( size_t lvl = 0; lvl < csa->nlevels; lvl++ ) {
		// current SA, SA^-1, char stops and normalised string ready

		//printf("building level %zu\n",lvl);
		//PRINT_ARR(sarr, sarr, %zu, 0, lvl_len, 20);

		csa->lvl_len[lvl] = lvl_len;

		// build phi function wavelet tree representation
		csa->char_stop_bv[lvl] = csrsbitarr_new( bitvec_detach(xchar_stops),
		                         lvl_len );

		xstring *phi_xstr = xstring_new_with_len(lvl_len, nbytes(ndiff_xchars));
		for (size_t i=0; i<lvl_len; i++)
			xstr_set( phi_xstr, sarr_inv[(sarr[i]+1)%lvl_len],
			          xstr_get(cur_xstr, sarr[i]) );
		csa->phi_wt[lvl] = wavtree_new_from_xstring( int_alphabet_new(ndiff_xchars),
		                   phi_xstr,  WT_BALANCED );
		//csa->phi_str[lvl] = phi_xstr;
		xstring_free(phi_xstr);

		// build even-suffix indicator bitvector
		// push even entries to first half of sarr rescaling its value
		// if not last level
		bitvec *even_suff = bitvec_new_with_capacity(lvl_len);
		if(lvl == csa->nlevels - 1) {
			for (size_t i = 0 ; i < lvl_len; i++)
				bitvec_push(even_suff, IS_EVEN(sarr[i]));
			break;
		} else {
			for (size_t i = 0, last = 0; i < lvl_len; i++) {
				if (IS_EVEN(sarr[i])) {
					bitvec_push(even_suff, 1);
					sarr[last++] = sarr[i]/2;
				} else
					bitvec_push(even_suff, 0);
			}
		}
		csa->even_bv[lvl] = csrsbitarr_new(bitvec_detach(even_suff), lvl_len);

		// prepare next level base string and sarr
		size_t nxt_lvl_len = (size_t) ceil(lvl_len/2.0f);
		sarr_invert(sarr, nxt_lvl_len, sarr_inv);
		xchar_stops = bitvec_new_with_capacity(nxt_lvl_len);
		bitvec_push_n(xchar_stops, nxt_lvl_len, 0);
		ndiff_xchars = 1;
		xchar_t ai, bi, aiminus1, biminus1;
		ai = xstr_get(cur_xstr, 2*sarr[0]);
		bi = (2*sarr[0]+1 < lvl_len) ? xstr_get(cur_xstr, 2*sarr[0]+1) : 0;
		for (size_t i=1; i<nxt_lvl_len; i++) {
			aiminus1 = ai;
			biminus1 = bi;
			ai = xstr_get(cur_xstr, 2*sarr[i]);
			bi= (2*sarr[i]+1 < lvl_len) ? xstr_get(cur_xstr, 2*sarr[i]+1) : 0;
			if (ai!=aiminus1 || bi!=biminus1) {
				bitvec_set_bit(xchar_stops, i-1, 1);
				ndiff_xchars++;
			}
		}
		assert(ndiff_xchars < XCHAR_MAX);
		bitvec_set_bit(xchar_stops, nxt_lvl_len-1, 1);
		xstring_free(cur_xstr);
		cur_xstr = xstring_new_with_len( nxt_lvl_len, nbytes(ndiff_xchars) );
		cur_xchar = 0;
		for (size_t i = 0; i < nxt_lvl_len; i++) {
			xstr_set(cur_xstr, sarr[i], cur_xchar);
			if (bitvec_get_bit(xchar_stops, i))
				cur_xchar++;
		}

		lvl_len = nxt_lvl_len;
	}
	xstring_free(cur_xstr);

	csa->root_sa = realloc(sarr, csa->lvl_len[csa->nlevels-1]*sizeof(size_t));
	csa->root_sa_inv = realloc(sarr_inv, csa->lvl_len[csa->nlevels-1]*sizeof(size_t));
	return csa;
}



void csarray_print(csarray *csa)
{
	if (csa==NULL) return;
	printf ("csarray@%p {\n", csa);
	printf ("\tlen:%zu\n", csa->lvl_len[0]);
	printf ("\tlevels:%zu\n", csa->nlevels);
	for (size_t lvl=0; lvl<csa->nlevels-1; lvl++) {
		printf("\t--- LEVEL %zu ---\n", lvl);
		printf("\teven_bv[%zu]:\n\t", lvl);
		bitarr_print( csrsbitarr_data(csa->even_bv[lvl]),
		              csrsbitarr_len(csa->even_bv[lvl]), 10 );
		//csrsbitarr_print(csa->even_bv[lvl], 4);
		printf("\tchar_stop_bv[%zu]:\n\t", lvl);
		bitarr_print( csrsbitarr_data(csa->char_stop_bv[lvl]),
		              csrsbitarr_len(csa->char_stop_bv[lvl]), 10 );
		//csrsbitarr_print(csa->char_stop_bv[lvl], 4);
		//printf("\tphi_wt[%zu]: ", lvl);
		//wavtree_print(csa->phi_wt[lvl]);
		//strbuf *phi = strbuf_new();
		//xstr_to_string(csa->phi_str[lvl], phi);
		//printf("\tphi_str[%zu]:\n\t", lvl);
		//printf("%s\n", strbuf_as_str(phi));
		//strbuf_free(phi);
	}
	PRINT_ARR(csa->root_sa, root_sa, %zu, 0, csa->lvl_len[csa->nlevels-1], 10);
	PRINT_ARR(csa->root_sa_inv, root_sa_inv, %zu, 0, csa->lvl_len[csa->nlevels-1], 10);
	printf ("} #end of csarray@%p\n", csa);

}


void csarray_free(csarray *csa)
{
	if (csa==NULL) return;
	for (size_t l=0; l<csa->nlevels-1; l++) {
		csrsbitarr_free(csa->even_bv[l], true);
		csrsbitarr_free(csa->char_stop_bv[l], true);
		wavtree_free(csa->phi_wt[l]);
	}
	alphabet_free(csa->xab);
	FREE(csa->lvl_len);
	FREE(csa->even_bv);
	FREE(csa->char_stop_bv);
	FREE(csa->phi_wt);
	FREE(csa->root_sa);
	FREE(csa->root_sa_inv);
}


size_t csarray_len(csarray *csarr)
{
	return csarr->lvl_len[0];
}



static size_t csa_phi(csarray *csa, size_t lvl, size_t i)
{
	xchar_t c = csrsbitarr_rank1(csa->char_stop_bv[lvl], i);
	size_t  r = csrsbitarr_pred1(csa->char_stop_bv[lvl], i);
	r = ( r < csa->lvl_len[lvl] ) ? i-r-1 : i ;
	return wavtree_select(csa->phi_wt[lvl], c, r);
}


size_t csarray_phi(csarray *csa, size_t i)
{
	return csa_phi(csa, 0, i);
}


static size_t csa_get(csarray *csa, size_t lvl, size_t i)
{
	if ( lvl == csa->nlevels-1 )
		return csa->root_sa[i];
	if ( csrsbitarr_get(csa->even_bv[lvl], i) ) {
		size_t epos = csrsbitarr_rank1(csa->even_bv[lvl], i);
		return 2*csa_get( csa, lvl+1, epos );
	} else {
		size_t phi = csa_phi(csa, lvl, i);
		size_t sa_i_plus1 = csa_get( csa, lvl, phi );
		return (sa_i_plus1 > 0) ? sa_i_plus1 - 1 : csa->lvl_len[lvl] - 1;
	}
}


size_t csarray_get(csarray *csa, size_t i)
{
	return csa_get(csa, 0, i);
}


static size_t csa_get_inv(csarray *csa, size_t lvl, size_t i)
{
	if (lvl == csa->nlevels - 1)
		return csa->root_sa_inv[i];
	if ( IS_EVEN(i) ) {
		size_t rec_inv = csa_get_inv(csa, lvl+1, i/2);
		return csrsbitarr_select1(csa->even_bv[lvl], rec_inv);
	} else {
		size_t inv_i_minus1 = csa_get_inv( csa, lvl, i-1 );
		return csa_phi(csa, lvl, inv_i_minus1);
	}
}


size_t csarray_get_inv(csarray *csa, size_t i)
{
	return csa_get_inv(csa, 0, i);
}


xchar_t csarray_get_char(csarray *csa, size_t i)
{
	size_t inv = csarray_get_inv(csa, i);
	size_t crk = csrsbitarr_rank1(csa->char_stop_bv[0], inv);
	return ab_char(csa->xab, crk);
}
