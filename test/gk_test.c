#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "CuTest.h"

#include "order.h"
#include "gk.h"
#include "mathutil.h"
#include "randutil.h"

static void print_int(FILE *stream, const void *val)
{
    fprintf(stream, "%d", *((int*)val));
}


void test_gk_upd(CuTest *tc)
{
    size_t univ = 1 << 10;
    size_t *bfcount = calloc(univ, sizeof(size_t));
    size_t nupd = 4 * univ;
    gksketch *sk = gk_new(sizeof(int), cmp_int, 0.05);
    size_t max_count = 0;
    for (size_t i=0; i<nupd; i++) {
        double xval;
        do {
            xval = (rand_norm() + 1.0) / 7.0;
        }
        while (xval < 0 || xval >= 1);
        int val = xval * univ;
        //printf("Insert %d:\n", val);
        gk_upd(sk, &val);
        bfcount[val]++;
        max_count = MAX(max_count, bfcount[val]);
        //gk_print(sk, stdout, print_int);
        //printf("\n");
    }

    printf("\nGK for %zu values in [0,%zu):\n", nupd, univ );
    gk_print(sk, stdout, print_int);


    size_t nbins = 50;
    double resol = 100.0 / (max_count * (univ/nbins));
    printf("\n\nGK Values distribution (each * for approx %f units)", (max_count * ((double)univ/(double)nbins))/100.0 );
    for (size_t i = 0, bin = 0; bin < nbins; bin++) {
        size_t bin_count = 0;
        while ( i < ((bin + 1) * univ) / nbins ) {
            bin_count += bfcount[i++];
        }
        printf("\n%10zu|",(bin*univ)/nbins);
        for (size_t j=0; j < resol * bin_count; j++) {
            printf("*");
        }
    }
    printf("\n");

    free(bfcount);
}


CuSuite *gk_get_test_suite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_gk_upd);
    return suite;
}