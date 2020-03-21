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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "new.h"
#include "cstringutil.h"


char *cstr_new(size_t len)
{
	char *ret;
	ret = (char *) malloc((len+1)*sizeof(char));
	memset(ret, '\0', (len+1)*sizeof(char));
	return ret;
}

void cstr_fill(char *str, size_t from, size_t to, char c)
{
	for (size_t i=from; i<to; i++) {
		str[i] = c;
	}
}

void cstr_clear(char *str, size_t len)
{
	memset(str, '\0', (len+1)*sizeof(char));
}


char *cstr_substr(char *str, size_t from,  size_t to)
{
	char *ret = cstr_new(to-from);
	memcpy(ret, str+from, to-from);
	return ret;
}


char *cstr_trim(char *str, size_t from,  size_t to)
{
	str = memmove(str, str+from, to-from);
	str = realloc(str, to-from+1);
	str[to-from] = '\0';
	return str;
}

char *cstr_trim_to_len(char *str, size_t len)
{
	return cstr_trim(str, 0, len);
}


char *cstr_resize(char *str, size_t len)
{
	size_t l = strlen(str);
	str = realloc(str, len+1);
	if (l < len) {
		memset(str+l, '\0', (len-l));
	}
	str[len] = '\0';
	return str;
}




void cstr_revert(char *str, size_t len)
{
	size_t i=0, j=len-1;
	char c;
	while (i<j) {
		c = str[i];
		str[i] = str[j];
		str[j] = c;
		i++;
		j--;
	}
}


static const char *DIGITS = "0123456789abcdef";


void uint_to_cstr(char *dest, uintmax_t val, char base)
{
	if (val==0) {
		strcpy(dest, "0");
		return;
	}
	size_t b, l;
	switch (base) {
	case 'h':
		b = 16;
		l = (size_t)ceil(log2(val+1)/4.0);
		break;
	case 'o':
		b = 8;
		l = (size_t)ceil(log2(val+1)/3.0);
		break;
	case 'b':
		b = 2;
		l = (size_t)ceil(log2(val+1));
		break;
	default:
		b = 10;
		l = (size_t)ceil(log10(val+1));
		break;
	}
	memset(dest, '0', l);
	dest[l]='\0';
	while (val) {
		dest[--l] = DIGITS[val%b];
		val /= b;
	}
}
