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
#include <stdint.h>

#include "coretype.h"
#include "mathutil.h"



#define POW2CEIL_IMPL( TYPE, ... )\
TYPE pow2ceil_##TYPE( TYPE val ) {\
    TYPE pow = 1;\
    while (pow < val) pow *= 2;\
    return pow;\
}

XX_UNSIGNED_INT(POW2CEIL_IMPL)


uint64_t mod_sum(uint64_t a, uint64_t b, uint64_t m)
{
	a %= m;
	b %= m;
	if ( a <= UINT64_MAX - b) {
		return (a + b) % m;
	} else {
		return b - ( m - a);
	}

}

uint64_t mod_mult(uint64_t a, uint64_t b, uint64_t m)
{
	uint64_t res = 0;
	a = a % m;
	while (b > 0) {
		if ( b & 1 ) { // b is odd
			res = mod_sum(res, a, m); // res = res + a mod m
		}
		a = mod_sum(a, a, m); // a = 2*a mod m
		b /= 2;
	}
	return res % m;
}


uint64_t mod_pow(uint64_t b, uint64_t e, uint64_t m)
{
	b = b % m;
	if (b == 0) return 0;
	uint64_t res = 1;
	while (e) {
		if (e & 1)
			res = mod_mult(res, b, m); // res = (res*b) % m;
		e >>= 1;
		b = mod_mult(b, b, m); // b = b^2 % m;
	}
	return res;
}


bool naive_is_prime(uint64_t val)
{
	if (val < 2) return false;
	for (uint64_t q=2; q*q <=val; q++) {
		if (val % q == 0) {
			return false;
		}
	}
	return true;
}


bool is_prime(uint64_t n)
{
	uint64_t a[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
	uint64_t limits[] = {2046, 1373652, 25326000, 3215031750, 2152302898746, 3474749660382, 341550071728320, 3825123056546413050, 18446744073709551615};
	size_t nwitness[] = {1, 2, 3, 4, 5, 6, 7, 9, 12};
	if (n < 2) return false;
	if (n == 2) return true;
	if (IS_EVEN(n)) return false;

	size_t nwit = 1;
	for (size_t j = 0; j < 9 && n >= limits[j]; nwit = nwitness[++j]);

	uint64_t d = n-1;
	uint64_t r = 0;
	while (IS_EVEN(d)) {
		d >>= 1;
		r += 1;
	}

	bool prime = true;
	for (size_t i = 0; prime && i < nwit; i++) {
		prime = false;
		uint64_t x = mod_pow(a[i], d, n); // (a[i]^d) mod n
		if ( x == 1 || x == (n-1) ) {
			prime = true;
			continue;
		}
		for (size_t _j=0; _j<r-1; _j++) {
			x = mod_mult(x, x, n); // (x*x) % n;
			if ( x == n-1 ) {
				prime = true;
				break;
			}
		}
	}
	return prime;
}


uint64_t prime_succ(uint64_t n)
{
	uint64_t firstfew[] = {2,2,2,3,5,5,7};
	if (n<=6) return firstfew[n];
	uint64_t k = (uint64_t) DIVCEIL(n, 6);
	assert (n <= k*6);
	uint64_t ret = 6 * k - 1;
	bool pm = true;
	if (ret < n) {
		ret += 2;
		pm = false;
	}
	while ( !is_prime(ret) ) {
		ret += (pm)?2:4;
		pm = !pm;
	}
	return ret;
}