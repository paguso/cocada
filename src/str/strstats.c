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

#include <stddef.h>

#include "alphabet.h"
#include "arrutil.h"
#include "strstats.h"
#include "strstream.h"
#include "xchar.h"
#include "xstr.h"


size_t *char_count(char *str, size_t slen, alphabet *ab)
{
	size_t absz = ab_size(ab);
	size_t *counts = NEW_ARR(size_t, absz);
	FILL_ARR(counts, 0, absz, 0);
	for (size_t i=0; i<slen; counts[ab_rank(ab, str[i++])]++);
	return counts;
}


size_t *xchar_count(xstr *xstr, alphabet *ab)
{
	size_t absz = ab_size(ab);
	size_t *counts = NEW_ARR(size_t, absz);
	FILL_ARR(counts, 0, absz, 0);
	for ( size_t i=0, l=xstr_len(xstr); i<l;
	        counts[ab_rank(ab, xstr_get(xstr, i++))]++ );
	return counts;
}


size_t  *char_cumul_count(char *str, size_t slen, alphabet *ab)
{
	size_t absz = ab_size(ab);
	size_t *counts;
	counts = NEW_ARR(size_t, absz+1);
	FILL_ARR(counts, 0, absz+1, 0);
	for (size_t i=0; i<slen; i++)
		counts[ab_rank(ab, str[i])+1]++;
	for (size_t i=1; i<=absz; i++)
		counts[i] += counts[i-1];
	return counts;
}


size_t  *xchar_cumul_count(xstr *xstr, alphabet *ab)
{
	size_t absz = ab_size(ab);
	size_t *counts = NEW_ARR(size_t, absz+1);
	FILL_ARR(counts, 0, absz, 0);
	for ( size_t i=0, l=xstr_len(xstr); i<l;
	        counts[ab_rank(ab, xstr_get(xstr, i++))+1]++ );
	for (size_t i=1; i<=absz; i++)
		counts[i] += counts[i-1];
	return counts;
}


size_t *char_count_stream(strstream *sst, alphabet *ab)
{
	size_t absz = ab_size(ab);
	size_t *counts = NEW_ARR(size_t, absz);
	FILL_ARR(counts, 0, absz, 0);
	strstream_reset(sst);
	for (xchar_t c; (c=strstream_getc(sst))!=EOF; counts[ab_rank(ab, c)]++);
	return counts;
}


size_t *char_cumul_count_stream(strstream *sst, alphabet *ab)
{
	size_t absz = ab_size(ab);
	size_t *counts;
	counts = NEW_ARR(size_t, absz+1);
	FILL_ARR(counts, 0, absz+1, 0);
	strstream_reset(sst);
	for ( xchar_t c; (c=strstream_getc(sst))!=EOF;
	        counts[ab_rank(ab, c)+1]++ );
	for (size_t i=1; i<=absz; i++)
		counts[i] += counts[i-1];
	return counts;
}


