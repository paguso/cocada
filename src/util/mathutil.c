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
    if ( a < UINT64_MAX - b) {
        return (a + b) % m;
    }  
    else {
        return b - ( m - a);
    }

}

uint64_t mod_mult(uint64_t a, uint64_t b, uint64_t m) 
{ 
    uint64_t res = 0;
    a = a % m; 
    while (b > 0) {
        // If b is odd, add 'a' to result 
        if ( b & 1 ) {
            res = mod_sum(res, a, m);
            /*
            if (res < UINT64_MAX - a ) {
                res = (res + a) % m; 
            } 
            else { // res + a overflows
                //assert (m-res <= a);
                res = a - (m - res);
                //assert (res <= m);
            }
            */
        }
        // Multiply 'a' with 2 
        a = mod_sum(a, a, m);
        /*
        if (a <= 0x7FFFFFFFFFFFFFFF) {
            a = (a * 2) % m; 
        }
        else { // 2*a overflows
            //assert(a<=m);
            //assert(m-a <= a);
            a = a - (m - a);
            //assert(a<=m);
        }
        */
        // Divide b by 2 
        b /= 2; 
    } 
    return res % m; 
} 


uint64_t mod_pow(uint64_t b, uint64_t e, uint64_t m) 
{
    b = b % m;
    if (b == 0) return 0;
    uint64_t r = 1;
    while (e) {
        if (e & 1)
            r = mod_mult(r, b, m);//(r*b) % m;
        e >>= 1;
        b = mod_mult(b, b, m);//(b*b) % m;
    }
    return r;
}


bool naive_is_prime(uint64_t val) 
{
    if (val<2) return false;
    for (uint64_t q=2; q*q <=val; q++) {
        if (val%q == 0) {
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
    if (n==2) return true;
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
