#include <stdlib.h>
#include <stdio.h>

#include "alphabet.h"
#include "arrayutil.h"
#include "bitvector.h"
#include "csarray.h"
#include "strstream.h"


static const char SENTINEL='$';

static const byte_t S = 0x0;
static const byte_t L = 0x1;

#define UNSET SIZE_MAX
#define CHAR_AT(S,I) ((ab_type==CHAR_TYPE)?((char *)(S))[(I)]:((size_t *)(S))[(I)])


void build_sarr( void *str, size_t len, alphabet *ab, size_t *sarr, 
                 bool add_sentinel );


static void get_bkt_start( size_t *bkts, size_t *dest, size_t l) 
{
    for (size_t i=0, sum=0; i<l; i++) {
        dest[i] = sum;
        sum += bkts[i];
    }
}


static void get_bkt_end( size_t *bkts, size_t *dest, size_t l) 
{
    for (size_t i=0, sum=0; i<l; i++) {
        sum += bkts[i];
        dest[i] = sum;
    }
}


static void init_LS( void *str, size_t len, alphabet *ab, alphabet_type ab_type,
                     bitvector *lsvec, bitvector *lmsvec, size_t *bkts, 
                     bool add_sentinel ) 
{
    size_t last=0;
    byte_t ls, lastls;
    int cmp;
    for (size_t i=1; i<len; i++) {
        cmp = ab_cmp(ab, CHAR_AT(str, i-1), CHAR_AT(str,i));
        if (cmp==0) continue;
        ls = (cmp>0);
        bitvec_append_n(lsvec, i-last, ls);
        bitvec_append(lmsvec, (last>0 && ls==S && lastls==L));
        bitvec_append_n(lmsvec, (i-last-1), false);
        bkts[ab_rank(ab, CHAR_AT(str,i-1))] += (i-last);
        last = i;
        lastls = ls;
    }
    if (add_sentinel) {
        // last run
        bitvec_append_n(lsvec, len-last, L);
        bitvec_append(lmsvec, (last>0 && ls==S && lastls==L));
        bitvec_append_n(lmsvec, (len-last-1), false);
        bkts[ab_rank(ab, CHAR_AT(str,len-1))] += (len-last);
    }
    // last run must be the sentinel
    bitvec_append(lsvec, S);
    bitvec_append(lmsvec, true);        
    bkts[0] = 1;
}


static void induce_L( char *str, alphabet *ab, alphabet_type ab_type, 
                      size_t *sarr, bitvector *ls,  
                      size_t *bkts, size_t *offsets ) 
{
    size_t l = ab_size(ab);
    get_bkt_start(bkts, offsets, l);
    size_t n = bitvec_len(ls);    
    for (size_t i=0, j; i<n; i++) {
        j = sarr[i];
        if ( j!=UNSET && j>0 && bitvec_get_bit(ls, j-1)==L ) {
            j -= 1;
            sarr[offsets[ab_rank(ab, CHAR_AT(str,j))]++] = j;
        }
    }    
}


static void induce_S( char *str, alphabet *ab, alphabet_type ab_type, 
                      size_t *sarr, bitvector *ls,  
                      size_t *bkts, size_t *offsets ) 
{
    size_t l = ab_size(ab);
    get_bkt_end(bkts, offsets, l);
    size_t n = bitvec_len(ls);    
    for (size_t i=n-1, j; i>0; i--) {
        j = sarr[i];
        if ( j!=UNSET && j>0 && bitvec_get_bit(ls, j-1)==S ) {
            j -= 1;
            sarr[--offsets[ab_rank(ab, CHAR_AT(str,j))]] = j;
        }
    }    
}


static void sort_LMS( void *str, alphabet *ab, alphabet_type ab_type, 
                      size_t *sarr, bitvector *ls, bitvector *lms,
                      size_t *bkts, size_t *offsets ) 
{
    size_t n = bitvec_len(ls);
    size_t l = ab_size(ab);
    
    // 1. Sort LMS segments
    get_bkt_end(bkts, offsets, l);
    sarr[0] = n-1;
    for (size_t i=0; i<n-1; i++) {
        if (!bitvec_get_bit(lms, i)) 
            continue;
        sarr[--offsets[ab_rank(ab, CHAR_AT(str, i))]] = i;
    }
    //printf("1)\n");
    //PRINT_ARRAY(sarr, SA, %zu, 0, n, 10);
    induce_L(str, ab, ab_type, sarr, ls, bkts, offsets);
    //printf("2)\n");
    //PRINT_ARRAY(sarr, SA, %zu, 0, n, 10);
    induce_S(str, ab, ab_type, sarr, ls, bkts, offsets);
    //printf("3)\n");
    //PRINT_ARRAY(sarr, SA, %zu, 0, n, 10);

    // 2. Reduce the problem
    // 2.1 move sorted LMS segments to 1st half of the SA
    size_t nlms = 0;
    for (size_t i=0; i<n; i++) 
        if (bitvec_get_bit(lms, sarr[i])) 
            sarr[nlms++] = sarr[i];
    FILL_ARRAY(sarr, nlms, n, UNSET);
    //printf("4)\n");
    //PRINT_ARRAY(sarr, SA, %zu, 0, n, 10);
    // 2.2 compute the # of different LMS segments
    //     and rename each of them as different macro int character
    size_t ndifflms=0;
    size_t cur_lms=0, prev_lms=0, pos=0;
    bool diff = false;
    for (size_t i=0; i<nlms; i++) {
        diff = false;
        cur_lms = sarr[i];
        for ( size_t k=0;; k++ ) {
            if (i==0 || CHAR_AT(str, prev_lms+k)!=CHAR_AT(str, cur_lms+k) ||
                bitvec_get_bit(ls, prev_lms+k)!=bitvec_get_bit(ls,cur_lms+k)) {
                diff = true;
                break;
            }
            else if ( k>0 && ( bitvec_get_bit(lms, prev_lms+k) 
                               || bitvec_get_bit(lms, cur_lms+k) ) ) {
                break;
            }
        } 
        if (diff) 
            ndifflms++;
        // since more than half of the SA is free 
        // (each LMS requires a preceding L)
        // use the second half to store the macro characters 
        // initially at scatered positions
        pos = ((cur_lms%2)==0)?(cur_lms/2):((cur_lms-1)/2);
        sarr[nlms+pos] = ndifflms-1;
        prev_lms = cur_lms;
    }
    //printf("5)\n");
    //PRINT_ARRAY(sarr, SA, %zu, 0, n, 10);
    // and finally push all the macro chars towards the end of the SA
    for (size_t i=n-1, j=n-1; i>=nlms; i--) 
        if (sarr[i]!=UNSET) {
            sarr[j]=sarr[i];
            if (i!=j) sarr[i]=UNSET;
            j--;
        }
    //printf("6)\n");
    //PRINT_ARRAY(sarr, SA, %zu, 0, n, 10);
            
    // 3. use the reduced string to sort the LMS suffixes
    size_t *red_sarr = sarr;
    size_t *red_str  = sarr+n-nlms;
    
    // 3.1 if all (sorted) macro chars are distinct, 
    //     the LMS suffixes are already sorted
    if (nlms==ndifflms) {
        for (size_t i=0; i<nlms; i++) 
            red_sarr[red_str[i]] = i;
    }
    // 3.2 otherwise, solve the reduced problem
    else {
        alphabet *red_ab = int_alphabet_new(ndifflms);
        build_sarr(red_str, nlms, red_ab, red_sarr, 0);
        alphabet_free(red_ab);
    }
    // now the first positions of sarr correspond to the number of
    // LMS suffixes (not their original positions) in lexicographic order
    // (sarr[0] = ndifflms-1)
    //printf("7)\n");
    //PRINT_ARRAY(sarr, SA, %zu, 0, n, 10);
    
    // 4. finally put the starting positions of the sorted LMS suffixes
    //    at their correct place in the SA
    get_bkt_end(bkts, offsets, l);
    for (size_t i=0, j=0; i<n; i++)
        if (bitvec_get_bit(lms, i))
            red_str[j++] = i;
    //printf("8)\n");
    //PRINT_ARRAY(sarr, SA, %zu, 0, n, 10);
    for (size_t i=0; i<nlms; i++)
        red_sarr[i] = red_str[red_sarr[i]];
    FILL_ARRAY(sarr, nlms, n, UNSET);
    //printf("9)\n");
    //PRINT_ARRAY(sarr, SA, %zu, 0, n, 10);
    for (size_t i=nlms-1, j; i>0; i--) {
        j = sarr[i];
        sarr[i] = UNSET;
        sarr[--offsets[ab_rank(ab, CHAR_AT(str,j))]] = j;
    }
    // Done. All LMS suffixes are sorted and correctly placed in the SA
    //printf("10)\n");
    //PRINT_ARRAY(sarr, SA, %zu, 0, n, 10);
}

/*
 * The alphabet MUST ALREADY contain the sentinel
 * If add_sentinel is true, a sentinel char is virtually appended to the string.
 * If add_sentinel is false, the smallest char in the alphabet must occur
 * exactly once at the end of the string.
 */
void build_sarr( void *str, size_t len, alphabet *ab, size_t *sarr, 
                 bool add_sentinel ) {
    size_t ab_sz = ab_size(ab);
    alphabet_type ab_t = ab_type(ab);

    bitvector *ls   = bitvector_new_with_capacity(len+add_sentinel);
    bitvector *lms  = bitvector_new_with_capacity(len+add_sentinel);
    size_t    *bkts = calloc(ab_sz, sizeof(size_t)); //NEW_ARRAY(size_t, ab_sz);
  
    init_LS(str, len, ab, ab_t, ls, lms, bkts, add_sentinel);
    
    //printf("-------------------------------------------------------------\n");
    //bitvec_print(ls, 4);
    //bitvec_print(lms, 4);
    //PRINT_ARRAY(bkts, bkts, %zu, 0, ab_sz, 10);

    FILL_ARRAY(sarr, 0, len+1, UNSET);
    size_t *offsets = NEW_ARRAY(size_t, ab_sz);
    
    sort_LMS(str, ab, ab_t, sarr, ls, lms, bkts, offsets);
    //PRINT_ARRAY(sarr, SA, %zu, 0, len+1, 10);
    induce_L(str, ab, ab_t, sarr, ls, bkts, offsets);
    //PRINT_ARRAY(sarr, SA, %zu, 0, len+1, 10);
    induce_S(str, ab, ab_t, sarr, ls, bkts, offsets);
    //PRINT_ARRAY(sarr, SA, %zu, 0, len+1, 10);
    
    // clean up
    bitvector_free(ls);
    bitvector_free(lms);
    FREE(bkts);
    FREE(offsets);
}


size_t *sais(char *str, size_t len, alphabet *ab) {
    if (len<2) {
        size_t *sarr = NEW_ARRAY(size_t, len+1);
        for (size_t i=0; i<=len; i++) sarr[i]=len-i;
        return sarr;
    }
    // creates the extended alphabet containing the sentinel
    size_t xl = ab_size(ab)+1;
    char *xchars = cstr_new(xl);
    xchars[0] = SENTINEL;
    for (size_t i=1; i<xl; i++) 
        xchars[i]=ab_char(ab,i-1);
    alphabet *xab = alphabet_new(xl, xchars);
    
    // build the suffix array
    size_t *sarr = NEW_ARRAY(size_t, len+1);
    build_sarr(str, len, xab, sarr, 1);
    
    // clean up
    alphabet_free(xab);
    
    return sarr;    
}
