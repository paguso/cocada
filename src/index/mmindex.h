#ifndef MMINDEX_H
#define MMINDEX_H


#include "alphabet.h"
#include "strread.h"
#include "xstring.h"
#include "vec.h"

typedef struct _mmindex mmindex;

/**
 * @param ab (move)
 * @param n
 * @param w (move)
 * @param k (move)
 */
mmindex *mmindex_new(alphabet *ab, size_t n, size_t *w, size_t *k);

void mmindex_index(mmindex *mmidx, strread *str);

const vec *mmindex_get(mmindex *mmidx, xstring *kmer);


#endif