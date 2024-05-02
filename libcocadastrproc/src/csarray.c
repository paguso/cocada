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
#include "arrays.h"
#include "bitbyte.h"
#include "bitarr.h"
#include "bitvec.h"
#include "bytearr.h"
#include "new.h"
#include "csarray.h"
#include "csrsbitarr.h"
#include "strbuf.h"
#include "math.h"
#include "mathutil.h"
#include "memdbg.h"
#include "sais.h"
#include "strstream.h"
#include "wavtree.h"
#include "xchar.h"
#include "xstr.h"

#define MIN_NLEVELS 1
#define MAX_PLAIN_SA_LEN 3

static const char SENTINEL = '$';


struct _CSArray {
	Alphabet *xab;
	usize nlevels;
	usize *lvl_len;
	CSRSBitArr **even_bv;
	CSRSBitArr **char_stop_bv;
	WavTree **phi_wt;
	usize *root_sa;
	usize *root_sa_inv;
	//xstr **phi_str;
};


static void sarr_invert(usize *src, usize len, usize *dest)
{
	for (usize i = 0; i < len; i++)
		dest[src[i]] = i;
}


CSArray *csarray_new( char *str, usize len, Alphabet *ab )
{
	CSArray *csa = NEW(CSArray);

	csa->nlevels = 1;  // # of levels, including root level
	for ( usize lvl_len = len + 1; lvl_len > MAX_PLAIN_SA_LEN;
	        lvl_len = (usize) ceil(lvl_len / 2.0f) )
		csa->nlevels++;

	csa->lvl_len = ARR_NEW(usize, csa->nlevels);
	csa->even_bv = ARR_NEW(CSRSBitArr *, csa->nlevels);
	csa->char_stop_bv = ARR_NEW(CSRSBitArr *, csa->nlevels);
	csa->phi_wt = ARR_NEW(WavTree *, csa->nlevels);
	//csa->phi_str = NEW_ARR(xstr*, csa->nlevels);

	// build plain sarray and its inverse
	usize *sarr = sais(str, len, ab);
	usize *sarr_inv = ARR_NEW(usize, len + 1);
	sarr_invert(sarr, len + 1, sarr_inv);

	usize lvl_len = len + 1; // sentinel added by sais
	csa->lvl_len[0] = lvl_len;
	BitVec *xchar_stops = bitvec_new_with_capacity(lvl_len);
	bitvec_push_n(xchar_stops, lvl_len, 0);
	StrBuf *supp_ab_str = strbuf_new(); // string support alphabet chars
	usize ndiff_xchars = 1;            // has at least the SENTINEL
	strbuf_append_char(supp_ab_str, SENTINEL);
	bitvec_set_bit(xchar_stops, 0, 1);
	bitvec_set_bit(xchar_stops, lvl_len - 1, 1);
	//ndiff_xchars++;
	for (usize i = 1; i < lvl_len; i++) {
		if (str[sarr[i]] != str[sarr[i - 1]]) {
			bitvec_set_bit(xchar_stops, i - 1, 1);
			strbuf_append_char(supp_ab_str, str[sarr[i]]);
			ndiff_xchars++;
		}
	}
	csa->xab = alphabet_new(ndiff_xchars, strbuf_detach(supp_ab_str));

	// convert source string to "normalised" xstr
	assert(ndiff_xchars < XCHAR_MAX);
	xstr *cur_xstr = xstr_new_with_capacity(nbytes(ndiff_xchars), lvl_len);
	xstr_push_n(cur_xstr, 0, lvl_len);
	xchar cur_xchar = 0;
	for (usize i = 0; i < lvl_len; i++) {
		xstr_set(cur_xstr, sarr[i], cur_xchar);
		if ( bitvec_get_bit(xchar_stops, i) )
			cur_xchar++;
	}

	//printf("str[0]: %s\n", str);
	//xstr_print(cur_xstr);
	//ARR_PRINT(sarr, sarr[0], %zu, 0, lvl_len, lvl_len);
	//ARR_PRINT(sarr_inv, sarr_inv[0], %zu, 0, lvl_len, lvl_len);

	// iterate over levels
	for ( usize lvl = 0; lvl < csa->nlevels; lvl++ ) {
		// current SA, SA^-1, char stops and normalised string ready

		//printf("building level %zu\n",lvl);
		//ARR_PRINT(sarr, sarr, %zu, 0, lvl_len, 20);

		csa->lvl_len[lvl] = lvl_len;

		// build phi function wavelet tree representation
		csa->char_stop_bv[lvl] = csrsbitarr_new( bitvec_detach(xchar_stops),
		                         lvl_len );

		xstr *phi_xstr = xstr_new_with_capacity(nbytes(ndiff_xchars), lvl_len);
		xstr_push_n(phi_xstr, 0, lvl_len);
		for (usize i = 0; i < lvl_len; i++)
			xstr_set( phi_xstr, sarr_inv[(sarr[i] + 1) % lvl_len],
			          xstr_get(cur_xstr, sarr[i]) );
		csa->phi_wt[lvl] = wavtree_new_from_xstr( alphabet_new_int_ab(ndiff_xchars),
		                   phi_xstr,  WT_BALANCED );
		//csa->phi_str[lvl] = phi_xstr;
		xstr_free(phi_xstr);

		// build even-suffix indicator bitvector
		// push even entries to first half of sarr rescaling its value
		// if not last level
		BitVec *even_suff = bitvec_new_with_capacity(lvl_len);
		if (lvl == csa->nlevels - 1) {
			for (usize i = 0 ; i < lvl_len; i++)
				bitvec_push(even_suff, IS_EVEN(sarr[i]));
			break;
		}
		else {
			for (usize i = 0, last = 0; i < lvl_len; i++) {
				if (IS_EVEN(sarr[i])) {
					bitvec_push(even_suff, 1);
					sarr[last++] = sarr[i] / 2;
				}
				else
					bitvec_push(even_suff, 0);
			}
		}
		csa->even_bv[lvl] = csrsbitarr_new(bitvec_detach(even_suff), lvl_len);

		// prepare next level base string and sarr
		usize nxt_lvl_len = (usize) ceil(lvl_len / 2.0f);
		sarr_invert(sarr, nxt_lvl_len, sarr_inv);
		xchar_stops = bitvec_new_with_capacity(nxt_lvl_len);
		bitvec_push_n(xchar_stops, nxt_lvl_len, 0);
		ndiff_xchars = 1;
		xchar ai, bi, aiminus1, biminus1;
		ai = xstr_get(cur_xstr, 2 * sarr[0]);
		bi = (2 * sarr[0] + 1 < lvl_len) ? xstr_get(cur_xstr, 2 * sarr[0] + 1) : 0;
		for (usize i = 1; i < nxt_lvl_len; i++) {
			aiminus1 = ai;
			biminus1 = bi;
			ai = xstr_get(cur_xstr, 2 * sarr[i]);
			bi = (2 * sarr[i] + 1 < lvl_len) ? xstr_get(cur_xstr, 2 * sarr[i] + 1) : 0;
			if (ai != aiminus1 || bi != biminus1) {
				bitvec_set_bit(xchar_stops, i - 1, 1);
				ndiff_xchars++;
			}
		}
		assert(ndiff_xchars < XCHAR_MAX);
		bitvec_set_bit(xchar_stops, nxt_lvl_len - 1, 1);
		xstr_free(cur_xstr);
		cur_xstr = xstr_new_with_capacity( nbytes(ndiff_xchars), nxt_lvl_len );
		xstr_push_n(cur_xstr, 0, nxt_lvl_len);
		cur_xchar = 0;
		for (usize i = 0; i < nxt_lvl_len; i++) {
			xstr_set(cur_xstr, sarr[i], cur_xchar);
			if (bitvec_get_bit(xchar_stops, i))
				cur_xchar++;
		}

		lvl_len = nxt_lvl_len;
	}
	xstr_free(cur_xstr);

	csa->root_sa = realloc(sarr, csa->lvl_len[csa->nlevels - 1] * sizeof(usize));
	csa->root_sa_inv = realloc(sarr_inv,
	                           csa->lvl_len[csa->nlevels - 1] * sizeof(usize));
	return csa;
}



void csarray_print(FILE *stream, CSArray *csa)
{
	if (csa == NULL) return;
	fprintf (stream, "csarray@%p {\n", csa);
	fprintf (stream, "\tlen:%zu\n", csa->lvl_len[0]);
	fprintf (stream, "\tlevels:%zu\n", csa->nlevels);
	for (usize lvl = 0; lvl < csa->nlevels - 1; lvl++) {
		fprintf(stream, "\t--- LEVEL %zu ---\n", lvl);
		fprintf(stream, "\teven_bv[%zu]:\n\t", lvl);
		bitarr_fprint( stream,  csrsbitarr_data(csa->even_bv[lvl]),
		               csrsbitarr_len(csa->even_bv[lvl]), 10, 0);
		//csrsbitarr_fprint(csa->even_bv[lvl], 4);
		fprintf(stream, "\tchar_stop_bv[%zu]:\n\t", lvl);
		bitarr_fprint( stream,  csrsbitarr_data(csa->char_stop_bv[lvl]),
		               csrsbitarr_len(csa->char_stop_bv[lvl]), 10, 0);
		//csrsbitarr_fprint(csa->char_stop_bv[lvl], 4);
		//printf("\tphi_wt[%zu]: ", lvl);
		//wavtree_print(csa->phi_wt[lvl]);
		//strbuf *phi = strbuf_new();
		//xstr_to_string(csa->phi_str[lvl], phi);
		//printf("\tphi_str[%zu]:\n\t", lvl);
		//printf("%s\n", strbuf_as_str(phi));
		//strbuf_free(phi);
	}
	ARR_FPRINT(stream, csa->root_sa, 0, csa->lvl_len[csa->nlevels - 1], 10,
	           "root_sa", "%zu", " ", "");
	ARR_FPRINT(stream, csa->root_sa_inv, 0, csa->lvl_len[csa->nlevels - 1], 10,
	           "root_sa_inv", "%zu", " ", "");

	fprintf (stream, "} #end of csarray@%p\n", csa);

}


void csarray_free(CSArray *csa)
{
	if (csa == NULL) return;
	for (usize l = 0; l < csa->nlevels - 1; l++) {
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


usize csarray_len(CSArray *csarr)
{
	return csarr->lvl_len[0];
}



static usize csa_phi(CSArray *csa, usize lvl, usize i)
{
	xchar c = csrsbitarr_rank1(csa->char_stop_bv[lvl], i);
	usize  r = csrsbitarr_pred1(csa->char_stop_bv[lvl], i);
	r = ( r < csa->lvl_len[lvl] ) ? i - r - 1 : i ;
	return wavtree_select(csa->phi_wt[lvl], c, r);
}


usize csarray_phi(CSArray *csa, usize i)
{
	return csa_phi(csa, 0, i);
}


static usize csa_get(CSArray *csa, usize lvl, usize i)
{
	if ( lvl == csa->nlevels - 1 )
		return csa->root_sa[i];
	if ( csrsbitarr_get(csa->even_bv[lvl], i) ) {
		usize epos = csrsbitarr_rank1(csa->even_bv[lvl], i);
		return 2 * csa_get( csa, lvl + 1, epos );
	}
	else {
		usize phi = csa_phi(csa, lvl, i);
		usize sa_i_plus1 = csa_get( csa, lvl, phi );
		return (sa_i_plus1 > 0) ? sa_i_plus1 - 1 : csa->lvl_len[lvl] - 1;
	}
}


usize csarray_get(CSArray *csa, usize i)
{
	return csa_get(csa, 0, i);
}


static usize csa_get_inv(CSArray *csa, usize lvl, usize i)
{
	if (lvl == csa->nlevels - 1)
		return csa->root_sa_inv[i];
	if ( IS_EVEN(i) ) {
		usize rec_inv = csa_get_inv(csa, lvl + 1, i / 2);
		return csrsbitarr_select1(csa->even_bv[lvl], rec_inv);
	}
	else {
		usize inv_i_minus1 = csa_get_inv( csa, lvl, i - 1 );
		return csa_phi(csa, lvl, inv_i_minus1);
	}
}


usize csarray_get_inv(CSArray *csa, usize i)
{
	return csa_get_inv(csa, 0, i);
}


xchar csarray_get_char(CSArray *csa, usize i)
{
	usize inv = csarray_get_inv(csa, i);
	usize crk = csrsbitarr_rank1(csa->char_stop_bv[0], inv);
	return alphabet_char(csa->xab, crk);
}
