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
#include <string.h>

#include "coretype.h"
#include "errlog.h"
#include "sort.h"
#include "randutil.h"
#include "range.h"

#define ELT(arr, i, typesize) (((byte_t *)(arr)) + ((i) * (typesize)))
#define SWP(a, b, swp, size) memcpy(swp, a, size); memcpy(a, b, size); memcpy(b, swp, size)
#define IDX_SWP(a, b, swp) swp = a; a = b; b = swp

size_t part(byte_t *arr, size_t typesize, cmp_func cmp, size_t l, size_t r)
{
	byte_t *left = arr + (l * typesize);
	byte_t *right = arr + ((r-1) * typesize);
	byte_t *swp = (byte_t *)malloc(sizeof(typesize));

	byte_t *p = left;
	if (r >= l + 3) {
		byte_t *mid = arr + (typesize * ((l + r) / 2));
		if ((cmp(left, mid)>0) ^ (cmp(left,right)>0)) {
			p = left;
		}
		else if ((cmp(mid, left)<0) ^ (cmp(mid, right)<0)) {
			p = mid;
		}
		else {
			p = right;
		}
	}
	if (p != left) {
		SWP(left, p, swp, typesize);
	}

	byte_t *i = left;
	byte_t *j = right;
	while (i <= j) {
		while (i <= right && cmp(i, left) <= 0) {
			i += typesize;
		}
		while (cmp(left, j) < 0) {
			j -= typesize;
		}
		if (i < j) {
			SWP(i, j, swp, typesize);
		}
	}
	SWP(left, j, swp, typesize);
	free(swp);
	return (j - arr)/typesize;
}

static void qs(void *arr, size_t typesize, cmp_func cmp, size_t l, size_t r)
{
	if (r <= l + 1)
		return;
	size_t p = part((byte_t *)arr, typesize, cmp, l, r);
	qs(arr, typesize, cmp, l, p);
	qs(arr, typesize, cmp, p + 1, r);
}

void quicksort(void *arr, size_t n, size_t typesize, cmp_func cmp)
{
	qs(arr, typesize, cmp, 0, n);
}


size_t idx_part(size_t *idx, byte_t *arr, size_t arr_tsz, cmp_func cmp,
                size_t l, size_t r)
{
	size_t i = l;
	size_t j = r - 1;
	size_t swp;

	size_t p = l;
	if (r >= l + 3) {
		size_t m = (l + r) / 2;
		if ((cmp(ELT(arr, idx[l], arr_tsz), ELT(arr, idx[m], arr_tsz)) > 0) ^
		        (cmp(ELT(arr, idx[l], arr_tsz), ELT(arr, idx[r-1], arr_tsz)) > 0)) {
			p = l;
		}
		else if ((cmp(ELT(arr, idx[m], arr_tsz), ELT(arr, idx[l], arr_tsz)) < 0) ^
		         (cmp(ELT(arr, idx[m], arr_tsz), ELT(arr, idx[r-1], arr_tsz)) < 0)) {
			p = m;
		}
		else {
			p = r - 1;
		}
	}
	if (p != l) {
		IDX_SWP(idx[l], idx[p], swp);
	}

	while (i <= j) {
		while (i < r
		        && cmp(ELT(arr, idx[i], arr_tsz), ELT(arr, idx[l], arr_tsz)) <= 0) {
			i++;
		}
		while (cmp(ELT(arr, idx[j], arr_tsz), ELT(arr, idx[l], arr_tsz)) > 0) {
			j--;
		}
		if (i < j) {
			IDX_SWP(idx[i], idx[j], swp);
		}
	}
	IDX_SWP(idx[l], idx[j], swp);
	return j;
}

void idx_qs(size_t *idx, byte_t *arr, size_t arr_tsz, cmp_func cmp, size_t l,
            size_t r)
{
	if (r <= l + 1)
		return;
	size_t p = idx_part(idx, arr, arr_tsz, cmp, l, r);
	idx_qs(idx, arr, arr_tsz, cmp, l, p);
	idx_qs(idx, arr, arr_tsz, cmp, p + 1, r);
}


size_t *index_quicksort(void *arr, size_t n, size_t typesize, cmp_func cmp)
{
	size_t *idx = range_arr_new_size_t(0, n, 1).arr;
	idx_qs(idx, arr, typesize, cmp, 0, n);
	return idx;
}


size_t succ(void *sorted_arr, size_t n, size_t typesize, cmp_func cmp, void *val)
{
    if (cmp(ELT(sorted_arr, n - 1, typesize), val) < 0) {
        return n;
    } else if (cmp(ELT(sorted_arr, 0, typesize), val) >= 0) {
        return 0;
    } else {
        size_t l = 0;
        size_t r = n - 1;
        size_t m;
        while ( r - l > 1) { // sucessor in (l,r]
            m = (l + r) / 2;
            if (cmp(ELT(sorted_arr, m, typesize), val) >= 0) {
                r = m;
            } else {
                l = m;
            }
        }
        return r;
    }
}

size_t strict_succ(void *sorted_arr, size_t n, size_t typesize, cmp_func cmp, void *val)
{
    if (cmp(ELT(sorted_arr, n - 1, typesize), val) <= 0) {
        return n;
    } else if (cmp(ELT(sorted_arr, 0, typesize), val) > 0) {
        return 0;
    } else {
        size_t l = 0;
        size_t r = n - 1;
        size_t m;
        while ( r - l > 1) { // sucessor in (l,r]
            m = (l + r) / 2;
            if (cmp(ELT(sorted_arr, m, typesize), val) > 0) {
                r = m;
            } else {
                l = m;
            }
        }
        return r;
    }
}


size_t pred(void *sorted_arr, size_t n, size_t typesize, cmp_func cmp, void *val)
{
    if (cmp(ELT(sorted_arr, n - 1, typesize), val) <= 0) {
        return n - 1;
    } else if (cmp(ELT(sorted_arr, 0, typesize), val) > 0) {
        return n;
    } else {
        size_t l = 0;
        size_t r = n - 1;
        size_t m;
        while ( r - l > 1) { // predecessor in [l, r)
            m = (l + r) / 2;
            if (cmp(ELT(sorted_arr, m, typesize), val) > 0) {
                r = m;
            } else {
                l = m;
            }
        }
        return l;
    }
}


size_t strict_pred(void *sorted_arr, size_t n, size_t typesize, cmp_func cmp, void *val)
{
    if (cmp(ELT(sorted_arr, n - 1, typesize), val) < 0) {
        return n - 1;
    } else if (cmp(ELT(sorted_arr, 0, typesize), val) >= 0) {
        return n;
    } else {
        size_t l = 0;
        size_t r = n - 1;
        size_t m;
        while ( r - l > 1) { // predecessor in [l, r)
            m = (l + r) / 2;
            if (cmp(ELT(sorted_arr, m, typesize), val) >= 0) {
                r = m;
            } else {
                l = m;
            }
        }
        return l;
    }
}