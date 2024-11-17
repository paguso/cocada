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
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cstrutil.h"
#include "mathutil.h"
#include "memdbg.h"
#include "new.h"


char *cstr_new(usize len)
{
	char *ret;
	ret = (char *) malloc((len + 1) * sizeof(char));
	memset(ret, '\0', (len + 1)*sizeof(char));
	return ret;
}


char *cstr_clone(const char *src)
{
	if (src == NULL) {
		return NULL;
	}
	char *ret = cstr_new(strlen(src));
	strcpy(ret, src);
	return ret;
}


char *cstr_clone_len(const char *src, usize len)
{
	char *ret = cstr_new(len);
	memcpy(ret, src, len * sizeof(char));
	return ret;
}


char *cstr_reassign(char *dest, const char *src)
{
	const usize n = strlen(src);
	char *ret = (char *) realloc(dest, (n + 1) * sizeof(char));
	strncpy(ret, src, n);
	ret[n] = '\0';
	return ret;
}


void cstr_fill(char *str, usize from, usize to, char c)
{
	memset(str + from, c, from - to);
}


void cstr_clear(char *str, usize len)
{
	memset(str, '\0', (len + 1)*sizeof(char));
}


char *cstr_substr(char *str, usize from,  usize to)
{
	char *ret = cstr_new(to - from);
	memcpy(ret, str + from, to - from);
	return ret;
}


char *cstr_ncpy(char *dest, char *src, usize n)
{
	usize m = strlen(src);
	m = MIN(m, n);
	dest[m] = '\0';
	return strncpy(dest, src, m);
}


void cstr_crop(char *str, usize from,  usize to)
{
	str = memmove(str, str + from, to - from);
	str[to - from] = '\0';
}


void cstr_trim(char *str, usize len, char *unwanted, usize unw_len)
{
	usize end = len;
	for (usize j = 0; j < unw_len; j++) {
		while (end && str[end - 1] == unwanted[j]) end--;
	}
	str[end] = '\0';
	if (end) {
		usize begin = 0;
		for (usize j = 0; j < unw_len; j++) {
			while (str[begin] == unwanted[j]) begin++;
		}
		if (begin) {
			memmove(str, str + (begin * sizeof(char)), (end - begin)*sizeof(char));
			str[end - begin] = '\0';
		}
	}
}


char *cstr_resize(char *str, usize len)
{
	usize l = strlen(str);
	str = realloc(str, len + 1);
	if (l < len) {
		memset(str + l, '\0', (len - l));
	}
	str[len] = '\0';
	return str;
}


char *cstr_fit(char *str)
{
	return cstr_resize(str, strlen(str));
}


char *cstr_cut(char *str, usize from, usize to)
{
	if (from >= to) return str;
	strcpy(&(str[from]), &(str[to]));
	return str;
}



void cstr_revert(char *str, usize len)
{
	usize i = 0, j = len - 1;
	char c;
	while (i < j) {
		c = str[i];
		str[i] = str[j];
		str[j] = c;
		i++;
		j--;
	}
}


static const char DIGITS[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};


char *uint_to_cstr(char *dest, uintmax_t val, char base)
{
	if (val == 0) {
		dest[0] = '0';
		dest[1] = '\0';
		return dest;
	}
	uintmax_t b, l;
	switch (base) {
	case 'h':
		b = 16;
		break;
	case 'o':
		b = 8;
		break;
	case 'b':
		b = 2;
		break;
	default:
		b = 10;
		break;
	}
	l = 0;
	while (val) {
		dest[l++] = DIGITS[val % b];
		val /= b;
	}
	dest[l] = '\0';
	cstr_revert(dest, l);
	return dest;
}


bool cstr_equals(const char *left, const char *right)
{
	if ((left == NULL) ^ (right == NULL)) return false;
	else if ((left == NULL) && (right == NULL)) return true;
	else return strcmp(left, right) == 0;
}


bool cstr_equals_ignore_case(const char *left, const char *right)
{
	if ((left == NULL) ^ (right == NULL)) return false;
	else if ((left == NULL) && (right == NULL)) return true;
	usize i, l = strlen(left);
	if ( l != strlen(right) )
		return false;
	for (i = 0; i < l && tolower(left[i]) == tolower(right[i]); i++);
	return i == l;
}


char *cstr_join(const char *sep, usize n, ...)
{
	va_list valist;
	usize len = 0;
	{
		va_start(valist, n);
		for (usize i = 0; i < n; i++) {
			len += strlen(va_arg(valist, char *));
		}
		va_end(valist);
	}
	len += ( ((n > 0) ? (n - 1) : 0) * strlen(sep));
	char *ret = cstr_new(len);
	{
		va_start(valist, n);
		for (usize i = 0; i < n; i++) {
			strcat(ret, va_arg(valist, char *));
			if (i + 1 < n) {
				strcat(ret, sep);
			}
		}
		va_end(valist);
	}
	ret[len] = '\0';
	return ret;
}
