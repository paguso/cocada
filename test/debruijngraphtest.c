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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CuTest.h"

#include "arrutil.h"
#include "cstringutil.h"
#include "debruijngraph.h"
#include "dynstr.h"
#include "mathutil.h"
#include "strstream.h"

static size_t n;
static alphabet **ab;
static size_t *slen;
static char **str;
static char **padstr;
static size_t *padslen;
static size_t *dbg_order;
static dbgraph **g;
static dbgraph **mg;


static void random_seq(alphabet *ab, char *dest, size_t n)
{
    for (size_t i=0; i<n; i++) {
        dest[i] = ab_char(ab, abs(rand())%ab_size(ab));
    }
    dest[n] = '\0';
}

void dbgraph_test_setup(CuTest *tc) 
{
    n = 20;
    ab   = NEW_ARR(alphabet*, n);
    slen = NEW_ARR(size_t, n);
    str  = NEW_ARR(char*, n);
    padstr  = NEW_ARR(char*, n);
    padslen = NEW_ARR(size_t, n);
    dbg_order    = NEW_ARR(size_t, n);
    g    = NEW_ARR(dbgraph*, n);
    mg   = NEW_ARR(dbgraph*, n);
    for (size_t i=0; i<n; i++) {
        ab[i] = alphabet_new(4, "acgt");
        
        
        slen[i] = 10*i;
        str[i] = cstr_new(slen[i]);
        random_seq(ab[i], str[i], slen[i]);
        dbg_order[i] = 3 + rand()%((i+2)/2);
                
        
        
        //char *fixed = "gttgagaatatcgggtttcg";
        ////              "tacgacgtcgact";
        ////              //"tgtgtacagctggcggacttaatgattgaagtgcccggcaaaggcaaggacgaaaattgatctcgcgatt";
        ////              "gactggtgagtcggtcagctttttaacaacagcgcaaagcgcgctcgtta";
        //slen[i] = strlen(fixed);
        //str[i] = cstr_new(slen[i]);
        //strcpy(str[i], fixed);
        //dbg_order[i] = 3;
        
 
        g[i] = dbg_new_from_str(ab[i], str[i], dbg_order[i], false);
        //dbg_print(g[i]);

        mg[i] = dbg_new_from_str(ab[i], str[i], dbg_order[i], true);
        //dbg_print(mg[i]);

        padslen[i] = slen[i]+dbg_order[i]+1;
        padstr[i] = cstr_new(padslen[i]);
        for (size_t j=0; j<dbg_order[i]; j++)
            padstr[i][j] = '$';
        strcpy(padstr[i]+dbg_order[i], str[i]);
        padstr[i][slen[i]+dbg_order[i]] = dbg_sentinel(g[i]);
    }
}


void dbgraph_test_teardown(CuTest *tc) 
{
    for (size_t i=0; i<n; i++) {
        alphabet_free(ab[i]);
        FREE(str[i]);
        dbg_free(g[i]);
        dbg_free(mg[i]);
    }
    FREE(ab);
    FREE(slen);
    FREE(str);
    FREE(dbg_order);
    FREE(g);
    FREE(mg);
}


void test_dbgraph_new(CuTest *tc)
{
    dbgraph_test_setup(tc);
    /*
    for (size_t i=0; i<n; i++) {
        printf("Graph #%zu: \n",i);
        dbg_print(g[i]);
        printf("\nMultiraph #%zu:\n",i);
        dbg_print(mg[i]);
    }
    */
    dbgraph_test_teardown(tc);
}


static inline xchar_t plain_chr(alphabet *base_ab,  xchar_t c)
{
    return (c > ab_size(base_ab)) ? 
           ab_char(base_ab, c - ab_size(base_ab) - 1) :
           ab_char(base_ab, c - 1);
}

static void node_cstr(xstring *node, alphabet *ab, char *dest)
{
    for (size_t i=0, l=xstr_len(node); i<l; i++ ) {
        xchar_t c = xstr_get(node, i);
        dest[i] = (c==0) ? '$' : plain_chr(ab, c);
    }
    dest[xstr_len(node)] = '\0';
}

size_t _outdeg_bf(size_t cs, char *node, bool multigraph)
{
    char *str = padstr[cs];
    size_t slen = padslen[cs];
    size_t k = dbg_order[cs];
    alphabet *abt = dbg_ab(g[cs]);
    bool *outletters = NEW_ARR(bool, ab_size(abt));
    FILL_ARR(outletters, 0, ab_size(abt), false);
    size_t ret = 0;
    for (size_t i=0, l=slen-k; i<l; i++) {
        size_t j = 0;
        while (j<k && str[i+j]==node[j])
            j++;
        if (j==k) {
            size_t ocrk = ab_rank(abt, str[i+k]);
            if (multigraph || !outletters[ocrk]) 
                ret++;
            outletters[ocrk] = true;
        }
    }
    FREE(outletters);
    return ret;
}


void test_dbgraph_outdeg(CuTest *tc) 
{
    dbgraph_test_setup(tc);
    
    for (size_t i=0; i<n; i++) {
        dbgraph *dbg = g[i];
        //printf("T:%s\n",padstr[i]);
        //dbg_print(dbg);
        xstring *xnode = xstring_new_with_capacity( nbytes(ab_size(dbg_ext_ab(dbg))), 
                                                    dbg_k(dbg) );
        char *node = cstr_new(dbg_k(dbg));
    
        for (size_t nrk=0, V=dbg_nnodes(dbg); nrk<V; nrk++) {
            size_t nid = dbg_node_id(dbg, nrk);
            size_t deg = dbg_outdeg(dbg, nid);
            dbg_node_lbl(dbg, nid, xnode);
            node_cstr(xnode, ab[i], node);
            size_t deg_bf = _outdeg_bf(i, node, dbg_is_multigraph(dbg));
            if (deg_bf!=deg) {
                printf("T=%s\n",padstr[i]);
                dbg_print(dbg);
                printf ("Node %s rk=%zu id=%zu deg=%zu\n", node, nrk, nid, deg);
                
            }
            CuAssertSizeTEquals(tc, deg_bf, deg);
        }

        dbg = mg[i];
        //dbg_print(dbg);
    
        for (size_t nrk=0, V=dbg_nnodes(dbg); nrk<V; nrk++) {
            size_t nid = dbg_node_id(dbg, nrk);
            size_t deg = dbg_outdeg(dbg, nid);
            dbg_node_lbl(dbg, nid, xnode);
            node_cstr(xnode, ab[i], node);            
            size_t deg_bf = _outdeg_bf(i, node, dbg_is_multigraph(dbg));
            //printf ("MGNode %s rk=%zu id=%zu deg=%zu\n", node, nrk, nid, deg);
            CuAssertSizeTEquals(tc, deg_bf, deg);
        }
        FREE(node);
    }
    dbgraph_test_teardown(tc);
}


size_t _lbl_outdeg_bf(size_t cs, char *node, char c, bool multigraph)
{
    char *str = padstr[cs];
    size_t slen = padslen[cs];
    size_t k = dbg_order[cs];
    size_t ret = 0;
    for (size_t i=0, l=slen-k; i<l; i++) {
        size_t j = 0;
        while (j<k && str[i+j]==node[j])
            j++;
        if (j==k && str[i+j]==c)
            ret++;
    }
    return multigraph ? ret : MIN(1,ret);
}


void test_dbg_lbl_outdeg(CuTest *tc) 
{
    dbgraph_test_setup(tc);
    
    for (size_t i=0; i<n; i++)  {
        dbgraph *dbg = g[i];
        alphabet *abt = ab[i];
        xstring *xnode = xstring_new_with_capacity( nbytes(ab_size(dbg_ext_ab(dbg))),
                                                    dbg_k(dbg) );
        char *node = cstr_new(dbg_k(dbg));
        for (size_t nrk=0, V=dbg_nnodes(dbg); nrk<V; nrk++) {
            size_t nid = dbg_node_id(dbg, nrk);
            dbg_node_lbl(dbg, nid, xnode);
            node_cstr(xnode, abt, node);
            for (size_t cr=0, abs=ab_size(abt); cr<abs; cr++) {
                char c = ab_char(abt, cr);
                size_t deg = dbg_lbl_outdeg(dbg, nid, c);
                //printf("outdeg(%zu=%s, %c) = %zu\n", nid, node, c, deg);
                size_t deg_bf = _lbl_outdeg_bf(i, node, c, dbg_is_multigraph(dbg));
                CuAssertSizeTEquals(tc, deg_bf, deg);
            }
        }

        dbg = mg[i];
        abt = ab[i];
        for (size_t nrk=0, V=dbg_nnodes(dbg); nrk<V; nrk++) {
            size_t nid = dbg_node_id(dbg, nrk);
            dbg_node_lbl(dbg, nid, xnode);
            node_cstr(xnode, abt, node);
            for (size_t cr=0, abs=ab_size(abt); cr<abs; cr++) {
                char c = ab_char(abt, cr);
                size_t deg = dbg_lbl_outdeg(dbg, nid, c);
                //printf("multi outdeg(%zu=%s, %c) = %zu\n", nid, node, c, deg);
                size_t deg_bf = _lbl_outdeg_bf(i, node, c, dbg_is_multigraph(dbg));
                CuAssertSizeTEquals(tc, deg_bf, deg);
            }
        }
        FREE(node);
    }
    dbgraph_test_teardown(tc);
}


void _child_bf(size_t cs, char *node, char c, bool multigraph, char *dest)
{
    char *str = padstr[cs];
    size_t slen = padslen[cs];
    size_t k = dbg_order[cs];
    cstr_fill(dest, 0, dbg_order[cs], '\0');
    for (size_t i=0, l=slen-k; i<l; i++) {
        size_t j = 0;
        while (j<k && str[i+j]==node[j])
            j++;
        if (j==k && str[i+j]==c) {
            strncpy(dest, str+i+1, k-1);
            dest[k-1]=c;
            return;
        }
    }
    return;
}


void test_dbgraph_child(CuTest *tc) 
{
    dbgraph_test_setup(tc);
    
    for (size_t i=0; i<n; i++)  {
        dbgraph *dbg = g[i];
        alphabet *abt = ab[i];
        char *par_lbl, *chd_lbl, *chd_lbl_bf;
        xstring *xpar_lbl, *xchd_lbl, *xchd_lbl_bf;
        par_lbl     = cstr_new(dbg_k(dbg));
        chd_lbl     = cstr_new(dbg_k(dbg));
        chd_lbl_bf  = cstr_new(dbg_k(dbg));
        xpar_lbl    = xstring_new_with_capacity(nbytes(ab_size(dbg_ext_ab(dbg))), dbg_k(dbg));
        xchd_lbl    = xstring_new_with_capacity(nbytes(ab_size(dbg_ext_ab(dbg))), dbg_k(dbg));
        xchd_lbl_bf = xstring_new_with_capacity(nbytes(ab_size(dbg_ext_ab(dbg))), dbg_k(dbg));

        for (size_t nrk=0, V=dbg_nnodes(dbg); nrk<V; nrk++) {
            size_t nid = dbg_node_id(dbg, nrk);
            dbg_node_lbl(dbg, nid, xpar_lbl);
            for (size_t cr=0, abs=ab_size(abt); cr<abs; cr++) {
                xchar_t c = ab_char(abt, cr);
                size_t chd = dbg_child(dbg, nid, c);
                dbg_node_lbl(dbg, chd, xchd_lbl);
                node_cstr(xchd_lbl, abt, chd_lbl);
                //printf("%zu=%s --- %c --> %zu=%s\n", 
                //        nid, par_lbl, c, chd, chd_lbl);
                _child_bf(i, par_lbl, c, dbg_is_multigraph(dbg), chd_lbl_bf);
                CuAssertStrEquals(tc, chd_lbl_bf, chd_lbl);
            }
        }
        FREE(par_lbl);
        FREE(chd_lbl);
        FREE(chd_lbl_bf);

        dbg = mg[i];
        abt = ab[i];
        par_lbl = cstr_new(dbg_k(dbg));
        chd_lbl = cstr_new(dbg_k(dbg));
        chd_lbl_bf = cstr_new(dbg_k(dbg));
        for (size_t nrk=0, V=dbg_nnodes(dbg); nrk<V; nrk++) {
            size_t nid = dbg_node_id(dbg, nrk);
            dbg_node_lbl(dbg, nid, xpar_lbl);
            for (size_t cr=0, abs=ab_size(abt); cr<abs; cr++) {
                char c = ab_char(abt, cr);
                size_t chd = dbg_child(dbg, nid, c);
                dbg_node_lbl(dbg, chd, chd_lbl);
                //printf("%zu=%s -- %c --> %zu=%s\n", nid, par_lbl, c, chd, chd_lbl);
                _child_bf(i, par_lbl, c, dbg_is_multigraph(dbg), chd_lbl_bf);
                CuAssertStrEquals(tc, chd_lbl_bf, chd_lbl);
            }
        }
        FREE(par_lbl);
        FREE(chd_lbl);
    }
    dbgraph_test_teardown(tc);
}


CuSuite *dbgraph_get_test_suite() 
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_dbgraph_new);
    SUITE_ADD_TEST(suite, test_dbgraph_outdeg);
    SUITE_ADD_TEST(suite, test_dbg_lbl_outdeg);
    SUITE_ADD_TEST(suite, test_dbgraph_child);
    return suite;
}