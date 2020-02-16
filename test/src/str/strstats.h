#ifndef STRSTATS_H
#define STRSTATS_H

#include <stddef.h>

#include "alphabet.h"
#include "strstream.h"
#include "xchar.h"
#include "xstring.h"

/**
 * @brief Returns individual letter counts of a given string.
 *
 * @deprecated Too specific. Should move out of here.
 */
size_t *char_count(char *str, size_t slen, alphabet *ab);


size_t *xchar_count(xstring *xstr, alphabet *ab);


/**
 * @brief Compute cumulative char frequencies
 * cumul_count(T, c) = Sum for a<c count(T,a)
 *
 * Example:
 * ```
 *        T =abracadabra   AB={a,b,c,d,r}
 *
 *           i    :   0    1    2    3    4    5
 *     alphabet   :   a    b    c    d    r
 *      chr_cnt   :   5    2    1    1    2
 *  cumul_freqs   :   0    5    7    8    9   11
 * ```
 *
 * Therefore, if the alphabet is A=<a[0], ..., a[s-1]>, then
 * the number of chars in the interval a[i]...a[j-1] is given by
 * cumul_count[j] - cumul_count[i]
 *
 * @deprecated Too specific. Should move out of here.
 */
size_t  *char_cumul_count(char *str, size_t slen, alphabet *ab);


/**
 * @brief Returns individual letter counts of a given stream.
 * @see ab_count
 * @deprecated Too specific. Should move out of here.
 */
size_t *char_count_stream(strstream *sst, alphabet *ab);


/**
 * @brief Compute cumulative char frequencies of a given stream
 * @deprecated Too specific. Should move out of here.
 */
size_t  *char_cumul_count_stream(strstream *sst, alphabet *ab);


#endif // !STRSTATS_H