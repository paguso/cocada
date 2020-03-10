#include <stddef.h>

#include "alphabet.h"
#include "arrutil.h"
#include "strstats.h"
#include "strstream.h"
#include "xchar.h"
#include "xstring.h"


size_t *char_count(char *str, size_t slen, alphabet *ab)
{
	size_t absz = ab_size(ab);
	size_t *counts = NEW_ARR(size_t, absz);
	FILL_ARR(counts, 0, absz, 0);
	for (size_t i=0; i<slen; counts[ab_rank(ab, str[i++])]++);
	return counts;
}


size_t *xchar_count(xstring *xstr, alphabet *ab)
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


size_t  *xchar_cumul_count(xstring *xstr, alphabet *ab)
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


