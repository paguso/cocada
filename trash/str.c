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

#include <string.h>

#include "cstrutil.h"
#include "str.h"


struct str {
	const char *s;
	size_t len;
};


str str_new(const char *src)
{
	return str_new_len(src, strlen(src));
}


str str_new_len(const char *src, size_t len)
{
	str ret = {};
	ret.len = len;
	ret.s = cstr_clone_len(src, len);
	return ret;
}


str cstr_to_str(char *src)
{
	return cstr_to_str_len(src, strlen(src));
}


str cstr_to_str_len(char *src, size_t len)
{
	return (str) {
		.s = src, .len = len
	};
}


size_t str_len(str s)
{
	return s.len;
}


const char *str_as_cstr(str s)
{
	return s.s;
}


void str_finalise(void *ptr, const finaliser *fnr)
{
	FREE((void *)(((str *)ptr)->s));
}