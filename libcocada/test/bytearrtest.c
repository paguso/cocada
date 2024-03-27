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


#include "CuTest.h"

#include "bitbyte.h"
#include "bytearr.h"
#include "memdbg.h"

static byte_t *ba_zeros, *ba_ones, *ba_odd, *ba_even, *ba_rand;
static size_t ba_size;

static void reset_arrays()
{
	size_t i, j;
	for (i = 0; i < ba_size; i++) {
		ba_zeros[i] = 0x0;
		ba_ones[i] = ~(0x0);
		ba_odd[i] = 0x0;
		ba_even[i] = 0x0;
		for (j = 0; j < BYTESIZE; j += 2) {
			ba_odd[i] |= ((0x1) << j);
			ba_even[i] |= ((0x2) << j);
		}
		ba_rand[i] = (byte_t)rand();
	}
}


void bytearray_test_setup(CuTest *tc)
{
	ba_size = 100;
	ba_zeros = malloc(ba_size);
	ba_ones = malloc(ba_size);
	ba_odd = malloc(ba_size);
	ba_even = malloc(ba_size);
	ba_rand = malloc(ba_size);
	reset_arrays();
	//printf("ba_zeros:\n");
	//bytearr_print(ba_zeros, ba_size, 4, "");
	//printf("ba_ones\n");
	//bytearr_print(ba_ones, ba_size, 4, "");
	//printf("ba_odd:\n");
	//bytearr_print(ba_odd, ba_size, 4, "");
	//printf("ba_even:\n");
	//bytearr_print(ba_even, ba_size, 4, "");
	//printf("ba_rand:\n");
	//bytearr_print(ba_rand, ba_size, 4, "");
}

void bytearray_test_teardown(CuTest *tc)
{
	free(ba_zeros);
	free(ba_ones);
	free(ba_odd);
	free(ba_even);
}

void test_revert_bytes(CuTest *tc)
{
	size_t i;
	uint32_t a, ainv;
	byte_t *ap, *ainvp;
	a = 0xccddeeff;
	ap = (byte_t *) &a;
	ainv = a;
	ainvp = (byte_t *) &ainv;
	bytearr_reverse((byte_t *)&ainv, sizeof(uint32_t));
	for (i = 0; i < sizeof(uint32_t); i++) {
		//printf("a[%zu]=%x ainv[%zu]=%x\n", i, ap[i], sizeof(uint32_t)-i-1, ainvp[sizeof(uint32_t)-i-1]);
		CuAssertTrue(tc, ap[i] == ainvp[sizeof(uint32_t) - i - 1] );
	}
	//printf("inverting bytes of %x gives %x", a, ainv);
}

void test_bytearr_write_int(CuTest *tc)
{
	size_t ntests, from_byte, i, bytecrop;
	int written, read;
	ntests = 10000;
	for (i = 0; i < ntests; i++) {
		from_byte = rand() % (ba_size - sizeof(unsigned int));
		written = (int)(rand());
		bytecrop = i % (sizeof(unsigned int) +1);
		bytearr_write_int(ba_zeros, from_byte, written, bytecrop);
		read = bytearr_read_int(ba_zeros, from_byte, bytecrop);
		if (bytecrop < sizeof(unsigned int)) {
			written &= (int)((((int)0x1) << (bytecrop * BYTESIZE)) - 1);
			written <<= (BYTESIZE * sizeof(unsigned int) - (bytecrop * BYTESIZE));
			if (written < 0) {
				written = ~0 & written >> ((BYTESIZE * sizeof(unsigned int)) -
				                           (bytecrop * BYTESIZE));
			}
			else {
				written >>= ((BYTESIZE * sizeof(unsigned int)) - (bytecrop * BYTESIZE));
			}
		}
		//printf(">> written = %x. read = %x\n", written, read);
		//printf(">> written = %d. read = %d\n", written, read);
		//bytearr_print(ba_zeros, ba_size, 4);
		CuAssertIntEquals(tc, written, read);
	}
}


void test_bytearr_write_uint(CuTest *tc)
{
	size_t ntests, from_byte, i, bytecrop;
	unsigned int written, read;
	ntests = 1000;
	for (i = 0; i < ntests; i++) {
		from_byte = rand() % (ba_size - sizeof(unsigned int));
		written = (unsigned int)(rand());
		bytecrop = i % (sizeof(unsigned int) +1);
		bytearr_write_uint(ba_zeros, from_byte, written, bytecrop);
		read = bytearr_read_uint(ba_zeros, from_byte, bytecrop);
		if (bytecrop < sizeof(unsigned int)) {
			written &= (size_t)((((unsigned int)0x1) << (bytecrop * BYTESIZE)) - 1);
			written <<= (BYTESIZE * sizeof(unsigned int) - (bytecrop * BYTESIZE));
			if (written < 0) {
				written = ~0 & written >> ((BYTESIZE * sizeof(unsigned int)) -
				                           (bytecrop * BYTESIZE));
			}
			else {
				written >>= ((BYTESIZE * sizeof(unsigned int)) - (bytecrop * BYTESIZE));
			}
		}
		//printf(">> written = %x. read = %x\n", written, read);
		//bytearr_print(ba_zeros, ba_size, 4);
		CuAssertIntEquals(tc, written, read);
	}
}


void test_bytearr_write_size_t(CuTest *tc)
{
	size_t ntests, from_byte, i, bytecrop;
	size_t written, read;
	ntests = 1000;
	for (i = 0; i < ntests; i++) {
		from_byte = rand() % (ba_size - sizeof(size_t));
		written = (size_t)(rand());
		bytecrop = i % (sizeof(size_t) +1);
		bytearr_write_size_t(ba_zeros, from_byte, written, bytecrop);
		read = bytearr_read_size_t(ba_zeros, from_byte, bytecrop);
		if (bytecrop < sizeof(size_t)) {
			written &= (size_t)((((size_t)0x1) << (bytecrop * BYTESIZE)) - 1);
			written <<= (BYTESIZE * sizeof(size_t) - (bytecrop * BYTESIZE));
			if (written < 0) {
				written = ~0 & written >> ((BYTESIZE * sizeof(size_t)) - (bytecrop * BYTESIZE));
			}
			else {
				written >>= ((BYTESIZE * sizeof(size_t)) - (bytecrop * BYTESIZE));
			}
		}
		//printf(">> written = %x. read = %x\n", written, read);
		//bytearr_print(ba_zeros, ba_size, 4);
		CuAssertIntEquals(tc, written, read);
	}
}

CuSuite *bytearray_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, bytearray_test_setup);
	SUITE_ADD_TEST(suite, test_bytearr_write_int);
	SUITE_ADD_TEST(suite, test_bytearr_write_uint);
	SUITE_ADD_TEST(suite, test_bytearr_write_size_t);
	SUITE_ADD_TEST(suite, bytearray_test_teardown);
	return suite;
}