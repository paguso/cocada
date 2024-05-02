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

#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "CuTest.h"

#include "bitbyte.h"
#include "errlog.h"
#include "cstrutil.h"

void test_byte_reverse(CuTest *tc)
{
	for (byte b = 0; b < BYTE_MAX; b++) {
		byte bc = b;
		byte_reverse(&bc);
		byte_reverse(&bc);
		CuAssertIntEquals(tc, b, bc);
	}
}


void test_byteo_str(CuTest *tc)
{
	CuAssert(tc, "Tests require BYTESIZE==8", BYTESIZE == 8);
	byte b;
	char str[BYTESIZE + 1];
	b = 0x01;
	byteo_str(b, str);
	CuAssertStrEquals(tc, "00000001", str);
	b = 0x23;
	byteo_str(b, str);
	CuAssertStrEquals(tc, "00100011", str);
	b = 0x45;
	byteo_str(b, str);
	CuAssertStrEquals(tc, "01000101", str);
	b = 0x67;
	byteo_str(b, str);
	CuAssertStrEquals(tc, "01100111", str);
	b = 0x89;
	byteo_str(b, str);
	CuAssertStrEquals(tc, "10001001", str);
	b = 0xab;
	byteo_str(b, str);
	CuAssertStrEquals(tc, "10101011", str);
	b = 0xcd;
	byteo_str(b, str);
	CuAssertStrEquals(tc, "11001101", str);
	b = 0xef;
	byteo_str(b, str);
	CuAssertStrEquals(tc, "11101111", str);

	printf("start converting\n");
	usize n = 1ll << 8;
	for (usize i = 0; i < n; i++) {
		b = (byte) i;
		byteo_str(b, str);
		DEBUG("%"PRIbB" = %s\n", BYTESTRB(b), str);
		byteo_strx(b, str);
		DEBUG("%"PRIbX" = %s\n", BYTESTRX(b), str);
	}

	printf("Done converting %zu bytes\n", n);
}


void test_byte_bitcount(CuTest *tc)
{
	for (byte bit = 0; bit < 2; bit++) {
		for (byte n = 0; n < BYTE_MAX; n++) {
			byte b = n;
			usize actual = byte_bitcount(b, bit);
			usize expected = 0;
			for (usize j = 0; j < BYTESIZE; j++) {
				if ((b % 2) == bit)
					expected++;
				b >>= 1;
			}
			CuAssertSizeTEquals(tc, expected, actual);
		}
	}
}


void test_uint16_bitcount(CuTest *tc)
{
	for (byte bit = 0; bit < 2; bit++) {
		for (uint16 n = 0; n < UINT16_MAX; n++) {
			uint16 b = n;
			usize actual = uint16_bitcount(b, bit);
			usize expected = 0;
			for (usize j = 0; j < 16; j++) {
				if ((b % 2) == bit)
					expected++;
				b >>= 1;
			}
			CuAssertSizeTEquals(tc, expected, actual);
		}
	}
}


void test_uint32_bitcount(CuTest *tc)
{
	for (byte bit = 0; bit < 2; bit++) {
		for (uint32 n = 1; n < UINT32_MAX; n = (n << 1) + 1) {
			uint32 b = n;
			usize actual = uint32_bitcount(b, bit);
			usize expected = 0;
			for (usize j = 0; j < 32; j++) {
				if ((b % 2) == bit)
					expected++;
				b >>= 1;
			}
			CuAssertSizeTEquals(tc, expected, actual);
		}
	}
}


void test_uint64_bitcount(CuTest *tc)
{
	for (byte bit = 0; bit < 2; bit++) {
		for (uint64 n = 1; n < UINT64_MAX; n = (n << 1) + 1) {
			uint64 b = n;
			usize actual = uint64_bitcount(b, bit);
			usize expected = 0;
			for (usize j = 0; j < 64; j++) {
				if ((b % 2) == bit)
					expected++;
				b >>= 1;
			}
			CuAssertSizeTEquals(tc, expected, actual);
		}
	}
}


void test_byte_rank(CuTest *tc)
{
	for (byte bit = 0; bit < 2; bit++) {
		for (byte n = 0; n < BYTE_MAX; n++) {
			for (usize i = 0; i < BYTESIZE; i++) {
				usize rk = byte_rank(n, i, bit);
				byte b = n;
				byte_reverse(&b);
				usize j = 0, cnt = 0;
				while (j < i) {
					if ((b % 2) == bit)
						cnt++;
					b >>= 1;
					j++;
				}
				CuAssertSizeTEquals(tc, cnt, rk);
			}
			usize rk = byte_rank(n, BYTESIZE, bit);
			CuAssertSizeTEquals(tc, byte_bitcount(n, bit), rk);
		}
	}
}



void test_byte_select(CuTest *tc)
{
	for (byte bit = 0; bit < 2; bit++) {
		for (byte n = 0; n < BYTE_MAX; n++) {
			usize s = byte_select(n, BYTESIZE, bit);
			CuAssertSizeTEquals(tc, BYTESIZE, s);
			for (usize rk = 0, c = byte_bitcount(n, bit); rk < c; rk++) {
				usize s = byte_select(n, rk, bit);
				byte b = n;
				byte_reverse(&b);
				usize j = 0, cnt = 0;
				while (cnt <= rk) {
					if ((b % 2) == bit)
						cnt++;
					b >>= 1;
					j++;
				}
				CuAssertSizeTEquals(tc, j - 1, s);
			}
		}
	}
}


void test_uint16_lohibit(CuTest *tc)
{
	for (uint16 x = 0 ; x < UINT16_MAX; x++) {
		uint lb = uint16_lobit(x);
		uint hb = uint16_hibit(x);
		int lbbf = -1;
		uint16 y = x;
		while ((y > 0)) {
			lbbf++;
			if (y % 2 == 1) {
				break;
			}
			y >>= 1;
		}
		lbbf = (lbbf >= 0) ? lbbf : 16;
		CuAssertUIntEquals(tc, lbbf, lb);
		y = x;
		int hbbf = -1;
		while (y) {
			hbbf++;
			y >>= 1;
		}
		hbbf = (hbbf >= 0) ? hbbf : 16;
		CuAssertUIntEquals(tc, hbbf, hb);
	}
}


void test_uint32_lohibit(CuTest *tc)
{
	uint32 y, x = 0;
	int hbbf = 0, hb, lbbf = 0, lb;
	for (byte *b = (byte *)&x, *end = (byte *)(&x + 1); b < end; b++) {
		for (int v = 0; v < 256; v++) {
			x = 0;
			*b = (byte)v;
			lb = uint32_lobit(x);
			y = x;
			for (lbbf = 0; lbbf < 32 && (~y & (uint32)1) ; y >>= 1, lbbf++);
			lbbf = lbbf >= 0 ? lbbf : 32;
			CuAssertUIntEquals(tc, lbbf, lb);
			x = 0;
			*b = (byte)v;
			hb = uint32_hibit(x);
			y = x;
			for (hbbf = -1; y ; y >>= 1, hbbf++);
			hbbf = hbbf >= 0 ? hbbf : 32;
			CuAssertUIntEquals(tc, hbbf, hb);
		}
	}
}


void test_uint64_lohibit(CuTest *tc)
{
	uint64 x = 0;
	int hbbf = 0, hb, lbbf = 0, lb;
	for (byte *b = (byte *)&x, *end = (byte *)(&x + 1); b < end; b++) {
		for (int v = 0; v < 256; v++) {
			x = 0;
			*b = (byte)v;
			lb = uint64_lobit(x);
			for (lbbf = 0; lbbf < 64 && (~x & (uint64)1) ; x >>= 1, lbbf++);
			lbbf = lbbf >= 0 ? lbbf : 64;
			CuAssertUIntEquals(tc, lbbf, lb);
			x = 0;
			*b = (byte)v;
			hb = uint64_hibit(x);
			for (hbbf = -1; x ; x >>= 1, hbbf++);
			hbbf = hbbf >= 0 ? hbbf : 64;
			CuAssertUIntEquals(tc, hbbf, hb);
		}
	}
}



CuSuite *bitbyte_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_byte_reverse);
	SUITE_ADD_TEST(suite, test_byteo_str);
	SUITE_ADD_TEST(suite, test_byte_bitcount);
	SUITE_ADD_TEST(suite, test_uint16_bitcount);
	SUITE_ADD_TEST(suite, test_uint32_bitcount);
	SUITE_ADD_TEST(suite, test_uint64_bitcount);
	SUITE_ADD_TEST(suite, test_byte_rank);
	SUITE_ADD_TEST(suite, test_byte_select);
	SUITE_ADD_TEST(suite, test_uint16_lohibit);
	SUITE_ADD_TEST(suite, test_uint32_lohibit);
	SUITE_ADD_TEST(suite, test_uint64_lohibit);
	return suite;
}
