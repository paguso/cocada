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

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CuTest.h"

#include "bitsandbytes.h"
#include "bitarray.h"
#include "cstringutil.h"
#include "new.h"
#include "mathutil.h"

static byte_t *ba_zeros, *ba_ones, *ba_odd, *ba_even, *ba_rand;
static size_t ba_size;

static void reset_arrays()
{
    //printf("reset_arrays\n");
    size_t i, j, nbytes;
    nbytes =  (size_t)ceil((double)ba_size/sizeof(byte_t));
    for (i=0; i<nbytes; i++) {
        ba_zeros[i] = 0x0;
        ba_ones[i] = ~(0x0);
        ba_odd[i] = 0x0;
        ba_even[i] = 0x0;
        for (j=0; j<BYTESIZE; j+=2) {
            ba_odd[i] |= ((0x1)<<j);
            ba_even[i] |= ((0x2)<<j);
        }
        ba_rand[i] = (byte_t)rand();
    }    
}


void bitarray_test_setup(CuTest *tc) 
{
    //printf("bitarray_test_setup\n");
    size_t nbytes;
    ba_size = 1000;
    nbytes =  (size_t)ceil((double)ba_size/sizeof(byte_t));
    ba_zeros = malloc(nbytes);
    ba_ones = malloc(nbytes);
    ba_odd = malloc(nbytes);
    ba_even = malloc(nbytes);
    ba_rand = malloc(nbytes);
    reset_arrays();
    //ba_print(ba_zeros, ba_size, 4);
    //ba_print(ba_ones, ba_size, 4);
    //ba_print(ba_odd, ba_size, 4);
    //ba_print(ba_even, ba_size, 4);
    //ba_print(ba_rand, ba_size, 4);
}

void bitarray_test_teardown(CuTest *tc) 
{
    //printf("bitarray_test_teardown\n");
    free(ba_zeros);
    free(ba_ones);
    free(ba_odd);
    free(ba_even);
}

void test_bitarr_new_from_str(CuTest *tc)
{
    //printf("test_bitarr_new_from_str\n");
    size_t max_len = 1024;
    char *str= cstr_new(max_len);
    byte_t *ba;
    for (size_t len = 0; len<max_len; len++) {
        for (size_t b=0; b<len; str[b++]=(rand()%2)?'1':'0');
        ba  = bitarr_new_from_str(str, len);
        char *dec = cstr_new(len);
        for (size_t i=0; i<len; i++) {
            dec[i] = bitarr_get_bit(ba, i)?'1':'0'; 
        }
        ////printf("str=%s\ndec=%s\n", str, dec);
        CuAssertStrEquals(tc, str, dec);
        free(ba);
        free(dec);
    }
    free(str);
}

void test_bitarr_get_bit(CuTest *tc)
{
    //printf("test_bitarr_get_bit\n");
    size_t i;
    reset_arrays();
    //byte_t bit;
    for (i=0; i<ba_size; i++) {
        ////printf("i=%zu\n",i);
        ////printf("zero\n");
        CuAssertTrue(tc, !bitarr_get_bit(ba_zeros, i));
        ////printf("one\n");
        CuAssertTrue(tc, bitarr_get_bit(ba_ones, i));
        if (!(i%2)) { // i is EVEN
            ////printf("odd\n");
            //bit = ba_get_bit(ba_odd, i);
            CuAssertTrue(tc, !bitarr_get_bit(ba_odd, i));
            ////printf("even\n");
            //bit = ba_get_bit(ba_even, i);
            CuAssertTrue(tc, bitarr_get_bit(ba_even, i));            
        } else { // i is ODD
            ////printf("odd\n");
            //bit = ba_get_bit(ba_odd, i);
            CuAssertTrue(tc, bitarr_get_bit(ba_odd, i));
            ////printf("even\n");
            //bit = ba_get_bit(ba_even, i);
            CuAssertTrue(tc, !bitarr_get_bit(ba_even, i));            
        }
    }
}

void test_bitarr_set_bit(CuTest *tc)
{    
    //printf("test_bitarr_set_bit\n");
    size_t i;
    reset_arrays();
        
    //  turn ba_zeros into ba_even
    //  turn ba_ones into ba_odd
    //  turn ba_odd into itself
    //  turn ba_even into itself
    
    for (i=0; i<ba_size; i++) {
        bitarr_set_bit(ba_zeros, i, 1);
        bitarr_set_bit(ba_ones, i, 0);
        CuAssertTrue(tc, bitarr_get_bit(ba_zeros, i));
        CuAssertTrue(tc, !bitarr_get_bit(ba_ones, i));
        if (!(i%2)) { // i is EVEN
            bitarr_set_bit(ba_odd, i, 0);
            bitarr_set_bit(ba_even, i, 1);
            CuAssertTrue(tc, !bitarr_get_bit(ba_odd, i));
            CuAssertTrue(tc, bitarr_get_bit(ba_even, i));            
        } else { // i is ODD
            bitarr_set_bit(ba_odd, i, 1);
            bitarr_set_bit(ba_even, i, 0);
            CuAssertTrue(tc, bitarr_get_bit(ba_odd, i));
            CuAssertTrue(tc, !bitarr_get_bit(ba_even, i));            
        }
    }
    //ba_print(ba_zeros, ba_size, 4);
    //ba_print(ba_ones, ba_size, 4);
    //ba_print(ba_odd, ba_size, 4);
    //ba_print(ba_even, ba_size, 4);
}


void test_bitarr_and(CuTest *tc)
{
    //printf("test_bitarr_and\n");
    reset_arrays();
    size_t n = MIN(ba_size, 27);
    bitarr_and(ba_odd, ba_even, n);
    //bitarr_print(ba_odd, ba_size, 4);
    for (size_t i=0; i<n; i++)
        CuAssertIntEquals(tc, 0, (int)bitarr_get_bit(ba_odd, i));
    bitarr_and(ba_even, ba_ones, n);
    //bitarr_print(ba_even, ba_size, 4);
    for (size_t i=0; i<n; i++)
        CuAssertIntEquals(tc, 1-i%2, (int)bitarr_get_bit(ba_even, i));
    reset_arrays();
}

void test_bitarr_or(CuTest *tc)
{
    //printf("test_bitarr_or\n");
    reset_arrays();
    size_t n = MIN(ba_size, 27);
    bitarr_or(ba_odd, ba_even, n);
    //bitarr_print(ba_odd, ba_size, 4);
    for (size_t i=1; i<n; i++)
        CuAssertIntEquals(tc, 1, (int)bitarr_get_bit(ba_odd, i));
    bitarr_or(ba_even, ba_zeros, n);
    //bitarr_print(ba_even, ba_size, 4);
    for (size_t i=0; i<n; i++)
        CuAssertIntEquals(tc, 1-i%2, (int)bitarr_get_bit(ba_even, i));
    reset_arrays();
}

void test_bitarr_not(CuTest *tc)
{
    //printf("test_bitarr_not\n");
    reset_arrays();
    size_t n = MIN(ba_size, 27);
    bitarr_not(ba_even, n);
    //bitarr_print(ba_even, ba_size, 4);
    for (size_t i=0; i<n; i++)
        CuAssertIntEquals(tc, i%2, (int)bitarr_get_bit(ba_even, i));
    reset_arrays();
}

void test_bitarr_write_char(CuTest *tc)
{
    //printf("test_bitarr_write_char\n");
    size_t ntests, from_bit, bitscrop, i;
    char written, read;
    ntests = 10000;

    for (i=0; i<ntests; i++) {
        from_bit = rand()%(ba_size-BYTESIZE*sizeof(char));
        written = (char)(rand());
        bitscrop = i%(BYTESIZE*sizeof(char)+1);
        bitarr_write_char(ba_rand, from_bit, written, bitscrop);
        read = bitarr_read_char(ba_rand, from_bit, bitscrop);
        ////printf(">>written = %hhx cropped to %zu. read = %hhx\n", written, bitscrop, read);
        if (bitscrop<(BYTESIZE*sizeof(char))) {
            written &= (char)((1<<bitscrop) - 1);
            written <<= (BYTESIZE*sizeof(char)-bitscrop);
            if (written<0) {
                written = ~0 & written>>(BYTESIZE*sizeof(char)-bitscrop);            
            } else {
                written >>= (BYTESIZE*sizeof(char)-bitscrop);
            }
        }
        CuAssertTrue(tc, written==read);
    }
}

void test_bitarr_write_uchar(CuTest *tc)
{
    //printf("test_bitarr_write_uchar\n");
    size_t ntests, from_bit, bitscrop, i;
    unsigned char written, read;
    ntests = 10000;

    for (i=0; i<ntests; i++) {
        from_bit = rand()%(ba_size-BYTESIZE*sizeof(unsigned char));
        written = (unsigned char)(rand());
        bitscrop = i%(BYTESIZE*sizeof(unsigned char)+1);
        bitarr_write_uchar(ba_rand, from_bit, written, bitscrop);
        read = bitarr_read_uchar(ba_rand, from_bit, bitscrop);
        if (bitscrop<(BYTESIZE*sizeof(char))) {
            written &= (char)((1<<bitscrop) - 1);
        }
        ////printf(">>written = %hhx cropped to %zu. read = %hhx\n", written, bitscrop, read);
        CuAssertTrue(tc, written==read);
    }
}

void test_bitarr_write_short(CuTest *tc)
{
    //printf("test_bitarr_write_short\n");
    size_t ntests, from_bit, bitscrop, i;
    short written, read;
    ntests = 10000;

    for (i=0; i<ntests; i++) {
        from_bit = rand()%(ba_size-BYTESIZE*sizeof(short));
        written = (short)(rand());
        bitscrop = i%(BYTESIZE*sizeof(short)+1);
        bitarr_write_short(ba_rand, from_bit, written, bitscrop);
        read = bitarr_read_short(ba_rand, from_bit, bitscrop);
        if (bitscrop<(BYTESIZE*sizeof(short))) {
            written &= (short)((1<<bitscrop) - 1);
            written <<= (BYTESIZE*sizeof(short)-bitscrop);
            if (written<0) {
                written = ~0 & written>>(BYTESIZE*sizeof(short)-bitscrop);            
            } else {
                written >>= (BYTESIZE*sizeof(short)-bitscrop);
            }
        }
        ////printf("written short = %hx cropped to %zu. read = %hx\n", written, bitscrop, read);
        CuAssertTrue(tc, written==read);
    }
}

void test_bitarr_write_ushort(CuTest *tc)
{
    //printf("test_bitarr_write_ushort\n");
    size_t ntests, from_bit, bitscrop, i;
    unsigned short written, read;
    ntests = 10000;

    for (i=0; i<ntests; i++) {
        from_bit = rand()%(ba_size-BYTESIZE*sizeof(unsigned short));
        written = (unsigned short)(rand());
        bitscrop = i%(BYTESIZE*sizeof(unsigned short)+1);
        bitarr_write_ushort(ba_rand, from_bit, written, bitscrop);
        read = bitarr_read_ushort(ba_rand, from_bit, bitscrop);
        if (bitscrop<(BYTESIZE*sizeof(unsigned short))) {
            written &= (unsigned short)((1<<bitscrop) - 1);
        }
        ////printf(">>written = %hhx cropped to %zu. read = %hhx\n", written, bitscrop, read);
        CuAssertTrue(tc, written==read);
    }
}

void test_bitarr_write_int(CuTest *tc)
{
    //printf("test_bitarr_write_int\n");
    size_t ntests, from_bit, bitscrop, i;
    int written, read;
    ntests = 10000;

    for (i=0; i<ntests; i++) {
        from_bit = rand()%(ba_size-BYTESIZE*sizeof(int));
        written = (int)(rand());
        bitscrop = i%(BYTESIZE*sizeof(int)+1);
        bitarr_write_int(ba_rand, from_bit, written, bitscrop);
        read = bitarr_read_int(ba_rand, from_bit, bitscrop);
        if (bitscrop<(BYTESIZE*sizeof(int))) {
            written &= (int)((1<<bitscrop) - 1);
            written <<= (BYTESIZE*sizeof(int)-bitscrop);
            if (written<0) {
                written = ~0 & written>>(BYTESIZE*sizeof(int)-bitscrop);            
            } else {
                written >>= (BYTESIZE*sizeof(int)-bitscrop);
            }
        }
        CuAssertTrue(tc, written==read);
    }
}

void test_bitarr_write_uint(CuTest *tc)
{
    //printf("test_bitarr_write_uint\n");
    size_t ntests, from_bit, bitscrop, i;
    unsigned int written, read;
    ntests = 10000;

    for (i=0; i<ntests; i++) {
        from_bit = rand()%(ba_size-BYTESIZE*sizeof(unsigned int));
        written = (unsigned int)(rand());
        bitscrop = i%(BYTESIZE*sizeof(unsigned int)+1);
        bitarr_write_uint(ba_rand, from_bit, written, bitscrop);
        read = bitarr_read_uint(ba_rand, from_bit, bitscrop);
        if (bitscrop<(BYTESIZE*sizeof(unsigned int))) {
            written &= (unsigned int)((1<<bitscrop) - 1);
        }
        ////printf(">>written = %hhx cropped to %zu. read = %hhx\n", written, bitscrop, read);
        CuAssertTrue(tc, written==read);
    }
}

void test_bitarr_write_long(CuTest *tc)
{
    //printf("test_bitarr_write_long\n");
    size_t ntests, from_bit, bitscrop, i;
    long written, read;
    ntests = 10000;

    for (i=0; i<ntests; i++) {
        from_bit = rand()%(ba_size-BYTESIZE*sizeof(long));
        written = (long)(rand());
        bitscrop = i%(BYTESIZE*sizeof(long)+1);
        bitarr_write_long(ba_rand, from_bit, written, bitscrop);
        read = bitarr_read_long(ba_rand, from_bit, bitscrop);
        ////printf(">>written = %lx cropped to %zu. read = %lx\n", written, bitscrop, read);
        if (bitscrop<(BYTESIZE*sizeof(long))) {
            written &= (long)((1l<<bitscrop) - 1);
            written <<= (BYTESIZE*sizeof(long)-bitscrop);
            if (written<0) {
                written = ~0l & written>>(BYTESIZE*sizeof(long)-bitscrop);            
            } else {
                written >>= (BYTESIZE*sizeof(long)-bitscrop);
            }
        }
        CuAssertTrue(tc, written==read);
    }
}

void test_bitarr_write_ulong(CuTest *tc)
{
    //printf("test_bitarr_write_ulong\n");
    size_t ntests, from_bit, bitscrop, i;
    unsigned long written, read;
    ntests = 10000;

    for (i=0; i<ntests; i++) {
        from_bit = rand()%(ba_size-BYTESIZE*sizeof(unsigned long));
        written = (unsigned long)(rand());
        bitscrop = i%(BYTESIZE*sizeof(unsigned long)+1);
        bitarr_write_ulong(ba_rand, from_bit, written, bitscrop);
        read = bitarr_read_ulong(ba_rand, from_bit, bitscrop);
        ////printf(">>written = %hhx cropped to %zu. read = %hhx\n", written, bitscrop, read);
        if (bitscrop<(BYTESIZE*sizeof(unsigned long))) {
            written &= (unsigned long)((1ul<<bitscrop) - 1);
        }
        CuAssertTrue(tc, written==read);
    }
}

void test_bitarr_write_longlong(CuTest *tc)
{
    //printf("test_bitarr_write_longlong\n");
    size_t ntests, from_bit, bitscrop, i;
    long long written, read;
    ntests = 10000;

    for (i=0; i<ntests; i++) {
        from_bit = rand()%(ba_size-BYTESIZE*sizeof(long long));
        written = (long long)(rand());
        bitscrop = i%(BYTESIZE*sizeof(long long)+1);
        bitarr_write_longlong(ba_rand, from_bit, written, bitscrop);
        read = bitarr_read_longlong(ba_rand, from_bit, bitscrop);
        ////printf(">>written = %hhx cropped to %zu. read = %hhx\n", written, bitscrop, read);
        if (bitscrop<(BYTESIZE*sizeof(long long))) {
            written &= (long long)((1ll<<bitscrop) - 1);
            written <<= (BYTESIZE*sizeof(long long)-bitscrop);
            if (written<0) {
                written = ~0ll & written>>(BYTESIZE*sizeof(long long)-bitscrop);            
            } else {
                written >>= (BYTESIZE*sizeof(long long)-bitscrop);
            }
        }        
        CuAssertTrue(tc, written==read);
    }
}

void test_bitarr_write_ulonglong(CuTest *tc)
{
    //printf("test_bitarr_write_ulonglong\n");
    size_t ntests, from_bit, bitscrop, i;
    unsigned long long written, read;
    ntests = 10000;

    for (i=0; i<ntests; i++) {
        from_bit = rand()%(ba_size-BYTESIZE*sizeof(unsigned long long));
        written = (unsigned long long)(rand());
        bitscrop = i%(BYTESIZE*sizeof(unsigned long long)+1);
        bitarr_write_ulonglong(ba_rand, from_bit, written, bitscrop);
        read = bitarr_read_ulonglong(ba_rand, from_bit, bitscrop);
        ////printf(">>written = %hhx cropped to %zu. read = %hhx\n", written, bitscrop, read);
        if (bitscrop<(BYTESIZE*sizeof(unsigned long long))) {
            written &= (unsigned long long)((1ull<<bitscrop) - 1);
        }
        CuAssertTrue(tc, written==read);
    }
}

void test_bitarr_write_size_t(CuTest *tc)
{
    //printf("test_bitarr_write_size_t\n");
    size_t ntests, from_bit, bitscrop, i;
    size_t written, read;
    ntests = 10000;

    for (i=0; i<ntests; i++) {
        from_bit = rand()%(ba_size-BYTESIZE*sizeof(size_t));
        written = (size_t)(rand());
        bitscrop = i%(BYTESIZE*sizeof(size_t)+1);
        bitarr_write_size(ba_rand, from_bit, written, bitscrop);
        read = bitarr_read_size(ba_rand, from_bit, bitscrop);
        ////printf(">>written = %hhx cropped to %zu. read = %hhx\n", written, bitscrop, read);
        if (bitscrop<(BYTESIZE*sizeof(size_t))) {
            written &= (size_t)((1l<<bitscrop) - 1);
            written <<= (BYTESIZE*sizeof(size_t)-bitscrop);
            if (written<0) {
                written = ~0 & written>>(BYTESIZE*sizeof(size_t)-bitscrop);            
            } else {
                written >>= (BYTESIZE*sizeof(size_t)-bitscrop);
            }
        }
        CuAssertTrue(tc, written==read);
    }
}

void test_bitarr_write_byte_t(CuTest *tc)
{
    //printf("test_bitarr_write_byte_t\n");
    size_t ntests, from_bit, bitscrop, i;
    byte_t written, read;
    ntests = 10000;

    for (i=0; i<ntests; i++) {
        from_bit = rand()%(ba_size-BYTESIZE*sizeof(byte_t));
        written = (byte_t)(rand());
        bitscrop = i%(BYTESIZE*sizeof(byte_t)+1);
        bitarr_write_byte(ba_rand, from_bit, written, bitscrop);
        read = bitarr_read_byte(ba_rand, from_bit, bitscrop);
        written <<= (BYTESIZE*sizeof(byte_t)-bitscrop);
        written >>= (BYTESIZE*sizeof(byte_t)-bitscrop);
        ////printf(">>written = %hhx cropped to %zu. read = %hhx\n", written, bitscrop, read);
        CuAssertTrue(tc, written==read);
    }
}




CuSuite *bitarray_get_test_suite() 
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, bitarray_test_setup);
    SUITE_ADD_TEST(suite, test_bitarr_new_from_str);
    SUITE_ADD_TEST(suite, test_bitarr_get_bit);
    SUITE_ADD_TEST(suite, test_bitarr_set_bit);
    SUITE_ADD_TEST(suite, test_bitarr_and);
    SUITE_ADD_TEST(suite, test_bitarr_or);
    SUITE_ADD_TEST(suite, test_bitarr_not);
    SUITE_ADD_TEST(suite, test_bitarr_write_char);
    SUITE_ADD_TEST(suite, test_bitarr_write_uchar);
    SUITE_ADD_TEST(suite, test_bitarr_write_short);
    SUITE_ADD_TEST(suite, test_bitarr_write_ushort);
    SUITE_ADD_TEST(suite, test_bitarr_write_int);
    SUITE_ADD_TEST(suite, test_bitarr_write_uint);
    SUITE_ADD_TEST(suite, test_bitarr_write_long);
    SUITE_ADD_TEST(suite, test_bitarr_write_ulong);
    SUITE_ADD_TEST(suite, test_bitarr_write_longlong);
    SUITE_ADD_TEST(suite, test_bitarr_write_ulonglong);
    SUITE_ADD_TEST(suite, test_bitarr_write_size_t);
    SUITE_ADD_TEST(suite, test_bitarr_write_byte_t);
    SUITE_ADD_TEST(suite, bitarray_test_teardown);
    return suite;
}