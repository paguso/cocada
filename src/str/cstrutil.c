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
#include <ctype.h>

#include "new.h"
#include "cstrutil.h"


char *cstr_new(size_t len)
{
	char *ret;
	ret = (char *) malloc((len+1)*sizeof(char));
	memset(ret, '\0', (len+1)*sizeof(char));
	return ret;
}


char *cstr_clone(const char *src)
{
	char *ret = cstr_new(strlen(src));
	strcpy(ret, src);
	return ret;
}


char *cstr_clone_len(const char *src, size_t len)
{
	char *ret = cstr_new(len);
	strcpy(ret, src);
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


char *cstr_crop(char *str, size_t from,  size_t to)
{
	str = memmove(str, str+from, to-from);
	str = realloc(str, to-from+1);
	str[to-from] = '\0';
	return str;
}

char *cstr_crop_len(char *str, size_t len)
{
	return cstr_crop(str, 0, len);
}


void cstr_trim(char *str, size_t len, char *unwanted, size_t unw_len)
{
	size_t end = len;
	for (size_t j=0; j<unw_len; j++) {
		while(end && str[end-1]==unwanted[j]) end--;
	}
	str[end] = '\0';
	if (end) {
		size_t begin = 0;
		for (size_t j=0; j<unw_len; j++) {
			while(str[begin]==unwanted[j]) begin++;
		}
		if (begin) {
			memmove(str, str + (begin*sizeof(char)), (end-begin)*sizeof(char));
			str[end-begin] = '\0';
		}
	}
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


bool cstr_equals(const char *left, const char *right)
{
	if ((left==NULL) ^ (right==NULL)) return false;
	else if ((left==NULL) && (right==NULL)) return true;
	else return strcmp(left, right)==0;
}


bool cstr_equals_ignore_case(const char *left, const char *right)
{
	if ((left==NULL) ^ (right==NULL)) return false;
	else if ((left==NULL) && (right==NULL)) return true;
	size_t i, l = strlen(left);
	if ( l != strlen(right) )
		return false;
	for (i = 0; i < l && tolower(left[i]) == tolower(right[i]); i++);
	return i==l;
}