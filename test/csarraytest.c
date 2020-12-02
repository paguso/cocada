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
#include <string.h>

#include "CuTest.h"

#include "alphabet.h"
#include "arrays.h"
#include "bytearr.h"
#include "cstrutil.h"
#include "csarray.h"
#include "sais.h"
#include "strstream.h"


#define MAX_STR_SIZE 100

static size_t Nab;
static size_t Narr;
static alphabet **ab;
static char** strings;
static size_t *slens;
static csarray **csarrays;
static size_t **sarrays;
static size_t **sarrinvs;



int suffix_compare(char *str, size_t len, size_t i, size_t j)
{
	//printf("comparing T[%d:]=%s to T[%d:]=%s\n", i, str+i, j, str+j);
	if (i==j) return 0;
	for (size_t k=0;; k++) {
		if (i+k==len && j+k<len) return -1;
		if (i+k<len && j+k==len) return +1;
		if (str[i+k]<str[j+k]) return -1;
		if (str[i+k]>str[j+k]) return +1;
	}
}

void csarray_sanity_check(CuTest *tc, csarray *csarr, char *str, size_t len)
{
	size_t sa_len = csarray_len(csarr);
	CuAssertSizeTEquals(tc, len+1, sa_len);
	CuAssertSizeTEquals(tc, len, csarray_get(csarr, 0));
	for (int i=1; i<sa_len-1; i++) {
		int cmp = suffix_compare(str, len, csarray_get(csarr, i), csarray_get(csarr, i+1));
		CuAssertIntEquals( tc, -1, cmp);
	}
}


/*
 * creates an alphabet with len ascii letters starting with 'a'
 */
static alphabet *seq_ab(size_t len)
{
	char *ab_letters = cstr_new(len);
	for (size_t i=0; i<len; i++)
		ab_letters[i] = 'a'+i;
	return alphabet_new(len, ab_letters);
}

static char *random_str(alphabet *ab, size_t len)
{
	char *ret = cstr_new(len);
	for (size_t i=0; i<len; i++)
		ret[i] = ab_char(ab, rand()%ab_size(ab));
	return ret;
}

static void sarr_invert(size_t *src, size_t len, size_t *dest)
{
	for (size_t i=0; i<len; i++)
		dest[src[i]] = i;
}


void xxxcsarray_test_setup(CuTest *tc)
{
	Nab = 1;
	Narr = 1;
	ab = NEW_ARR(alphabet*, Narr);
	strings = NEW_ARR(char*, Narr);
	slens = NEW_ARR(size_t, Narr);
	csarrays = NEW_ARR(csarray*, Narr);
	sarrays = NEW_ARR(size_t*, Narr);
	sarrinvs = NEW_ARR(size_t*, Narr);
	ab[0] = alphabet_new(4,"elns");
	strings[0] = cstr_new(strlen("senselessness"));
	strcat(strings[0], "senselessness");
	slens[0] = strlen(strings[0]);
	csarrays[0] = csarray_new(strings[0], slens[0], ab[0]);
	sarrays[0] = sais(strings[0], slens[0], ab[0]);
	sarrinvs[0] = NEW_ARR(size_t, slens[0]+1);
	sarr_invert(sarrays[0], slens[0]+1, sarrinvs[0]);
	csarray_print(stdout, csarrays[0]);
}


void csarray_test_setup(CuTest *tc)
{
	Nab = 10;
	Narr = Nab*MAX_STR_SIZE;
	ab = NEW_ARR(alphabet*, Narr);
	strings = NEW_ARR(char*, Narr);
	slens = NEW_ARR(size_t, Narr);
	sarrays = NEW_ARR(size_t*, Narr);
	sarrinvs = NEW_ARR(size_t*, Narr);
	csarrays = NEW_ARR(csarray*, Narr);
	for (int l=0; l<Nab; l++) {
		for (int i=0, j=0; i<MAX_STR_SIZE; i++) {
			//printf("l=%d i=%d\n",l,i);
			j = MAX_STR_SIZE*l + i;
			ab[j] = seq_ab(l);
			slens[j] = (l==0)?0:i;
			strings[j] = random_str(ab[j], slens[j]);
			csarrays[j] = csarray_new(strings[j], slens[j], ab[j]);
			sarrays[j] = sais(strings[j], slens[j], ab[j]);
			sarrinvs[j] = NEW_ARR(size_t, slens[j]+1);
			sarr_invert(sarrays[j], slens[j]+1, sarrinvs[j]);

			//csarray_print(csarrays[j]);
			//printf("string=%s\n", strings[j]);
			//ab_print(ab[j]);
		}
	}

}

void csarray_test_teardown(CuTest *tc)
{
	for (int i=0; i<Narr; i++) {
		csarray_free(csarrays[i]);
		FREE(strings[i]);
		FREE(ab[i]);
	}
	FREE(csarrays);
	FREE(strings);
	FREE(slens);
	FREE(ab);

}


static size_t phi_bf(size_t *sa, size_t len, size_t pos)
{
	size_t i, target = (sa[pos]+1)<len ? sa[pos]+1 : 0;
	for (i=0; sa[i]!=target; i++);
	return i;
}


void csarray_test_phi(CuTest *tc)
{
	for (size_t i=0; i<Narr; i++) {
		csarray *csa = csarrays[i];
		size_t slen = slens[i];
		size_t *sa = sarrays[i];
		for (size_t j=0; j<slen+1; j++) {
			size_t phi = csarray_phi(csa, j);
			size_t phibf = phi_bf(sa, slen+1, j);
			//printf("sa[%zu].phi(%zu) = %zu (bf = %zu)\n", i, j, phi, phibf);
			CuAssertSizeTEquals(tc, phibf, phi);
		}
	}
}


void csarray_test_get(CuTest *tc)
{
	for (size_t i=0; i<Narr; i++) {
		csarray *csa = csarrays[i];
		size_t  *sa = sarrays[i];
		size_t sa_len = csarray_len(csa);
		for (size_t j=0; j<sa_len; j++) {
			size_t s = csarray_get(csa, j);
			//printf("csa[%zu].get(%zu) = %zu (bf = %zu)\n", i, j, s, sa[j]);
			if (sa[j]!=s) {
				csarray_print(stdout, csa);
				PRINT_ARR(sa, sa, %zu, 0, sa_len, 10);
			}
			CuAssertSizeTEquals(tc, sa[j], s);
		}
	}
}


void csarray_test_get_inv(CuTest *tc)
{
	for (size_t k=0; k<Narr; k++) {
		csarray *csa = csarrays[k];
		size_t  *sainv = sarrinvs[k];
		size_t sa_len = csarray_len(csa);
		for (size_t i=0; i<sa_len; i++) {
			size_t j = csarray_get_inv(csa, i);
			//printf("csa[%zu].get_inv(%zu) = %zu (bf = %zu)\n", k, i, j, sainv[i]);
			CuAssertSizeTEquals(tc, sainv[i], j);
		}
	}

}


void csarray_test_get_char(CuTest *tc)
{
	for (size_t k=0; k<Narr; k++) {
		csarray *csa = csarrays[k];
		char *str = strings[k];
		size_t slen = slens[k];
		for (size_t i=0; i<slen; i++) {
			xchar_t c = csarray_get_char(csa, i);
			//printf("csa[%zu].get_char(%zu) = "XCHAR_FMT"(%c) (bf = %c)\n", k, i,
			//        c, (char)c, str[i]);
			CuAssertCharEquals(tc, str[i], (char)c);
		}
	}

}


CuSuite *csarray_get_test_suite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, csarray_test_setup);
	SUITE_ADD_TEST(suite, csarray_test_phi);
	SUITE_ADD_TEST(suite, csarray_test_get);
	SUITE_ADD_TEST(suite, csarray_test_get_inv);
	SUITE_ADD_TEST(suite, csarray_test_get_char);
	SUITE_ADD_TEST(suite, csarray_test_teardown);
	return suite;
}
