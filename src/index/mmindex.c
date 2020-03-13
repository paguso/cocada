#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "alphabet.h"
#include "arrutil.h"
#include "new.h"
#include "order.h"
#include "minqueue.h"
#include "new.h"
#include "hashmap.h"
#include "vec.h"
#include "xstring.h"
#include "xstrhash.h"

#include "mmindex.h"

struct _mmindex {
	alphabet *ab;
	xstrhash *hasher;
	size_t nidx;
	size_t *w;
	size_t *k;
	size_t max_wlen;
	hashmap **tbls;
	size_t nstr;
	vec *offs;
	size_t nseq;
};


mmindex *mmindex_new(alphabet * ab, size_t n, size_t * w, size_t * k)
{
	mmindex *ret = NEW(mmindex);
	ret->ab = ab;
	ret->hasher = xstrhash_new(alphabet_clone(ab));
	ret->nidx = n;
	ret->w = w;
	ret->k = k;
	ret->max_wlen = 0;
	for (size_t i = 0; i < n; i++)
		ret->max_wlen =
		    (w[i] + k[i] > ret->max_wlen) ? w[i] + k[i] : ret->max_wlen;
	ret->tbls = NEW_ARR(hashmap *, ret->nidx);
	FILL_ARR(ret->tbls, 0, ret->nidx,
	         hashmap_new(sizeof(uint64_t), sizeof(vec *),
	                     ident_hash_uint64_t, eq_uint64_t));
	ret->nstr = 0;
	ret->offs = vec_new(sizeof(size_t));
	vec_push_size_t(ret->offs, 0);
	ret->nseq = 0;
	return ret;
}

void mmindex_dispose(void *ptr, const dtor * dt)
{
	mmindex *mm = (mmindex *) ptr;
	alphabet_free(mm->ab);
	FREE(mm->w);
	FREE(mm->k);
	dtor *hmdt = dtor_cons(dtor_cons(DTOR(hashmap), empty_dtor()),
	                       dtor_cons(ptr_dtor(), DTOR(vec)));
	for (size_t i = 0; i < mm->nidx; i++) {
		FINALISE(mm->tbls[i], hmdt);
		FREE(mm->tbls[i]);
	}
	FREE(mm->tbls);
	dtor_free(hmdt);
	FREE(mm->offs, vec);
}


typedef struct _rankpos {
	uint64_t rank;
	size_t pos;
} rankpos;


int cmp_rankpos(const void *pl, const void *pr)
{
	return cmp_uint64_t(pl, pr);
}


static inline void _insert(hashmap * tbl, uint64_t rank, size_t pos)
{
	vec *v;
	if (hashmap_has_key(tbl, (const void *) &rank)) {
		v = hashmap_get_mut(tbl, (const void *) &rank);
	} else {
		v = vec_new(sizeof(size_t));
		hashmap_set(tbl, (const void *) &rank, &v);
	}
	vec_push_size_t(v, pos);
}


void mmindex_index(mmindex * self, strstream * sst)
{
	size_t nidx = self->nidx;
	size_t offset = vec_last_size_t(self->offs);
	xstring *window = xstring_new_with_capacity(strstream_sizeof_char(sst),
	                  self->max_wlen);
	minqueue **win_rks = NEW_ARR(minqueue *, nidx);
	FILL_ARR(win_rks, 0, nidx, minqueue_new(sizeof(rankpos), cmp_rankpos));
	uint64_t *prev_mm_rk = NEW_ARR(uint64_t, nidx);	// rank of previous window minimiser
	FILL_ARR(prev_mm_rk, 0, nidx, 0);
	uint64_t *prev_right_rk = NEW_ARR(uint64_t, nidx);	// rank of previous window rightmost kmer
	FILL_ARR(prev_right_rk, 0, nidx, 0);

	size_t pos = 0;
	rankpos rp = { 0, 0 };
	for (xchar_t c; (c = strstream_getc(sst)) != EOF;) {
		// prepare window
		if (pos >= self->max_wlen) {
			xstr_rot_left(window, 1);
			xstr_set(window, self->max_wlen - 1, c);
		} else {
			//read in first window
			xstr_push(window, c);
		}
		pos += 1;
		for (size_t i = 0; i < nidx; i++) {
			if (pos == self->k[i]) {
				uint64_t kmer_rk = xstrhash_lex_sub(self->hasher, window,
				                                    xstr_len(window) -
				                                    self->k[i],
				                                    xstr_len(window));
				prev_right_rk[i] = kmer_rk;
				prev_mm_rk[i] = kmer_rk;
				rp.rank = kmer_rk;
				rp.pos = pos - self->k[i];
				minqueue_push(win_rks[i], &rp);
				// initial end minimisers are all indexed
				_insert(self->tbls[i], kmer_rk, offset + pos - self->k[i]);
			} else if (pos > self->k[i]) {
				// get previous windows minimiser
				// let (last_mm_rk, _last_mm_pos) = win_rks[i].xtr().unwrap().clone();
				// compute new last kmer rank and add it to the new window
				uint64_t kmer_rk =
				    xstrhash_roll_lex_sub(self->hasher, window,
				                          xstr_len(window) - self->k[i] -
				                          1,
				                          xstr_len(window) - 1,
				                          prev_right_rk[i], c);
				size_t kmer_pos = pos - self->k[i];
				prev_right_rk[i] = kmer_rk;
				// dequeue the first kmer of previous window if it is full
				if (pos > self->w[i] + self->k[i] - 1) {
					minqueue_remv(win_rks[i]);
				}
				// and add new kmer
				rp.rank = kmer_rk;
				rp.pos = kmer_pos;
				minqueue_push(win_rks[i], &rp);
				// then get current window miminiser
				uint64_t cur_mm_rk =
				    ((rankpos *) minqueue_min(win_rks[i]))->rank;
				if (self->w[i] == 1 || prev_mm_rk[i] != cur_mm_rk) {
					// new minimiser. add all its occurrences
					for (minqueue_iter it = minqueue_all_min(win_rks[i]);
					        minqueue_iter_has_next(&it);) {
						const rankpos *mm = minqueue_iter_next(&it);
						_insert(self->tbls[i], mm->rank, offset + mm->pos);
					}
					prev_mm_rk[i] = cur_mm_rk;
				} else if (cur_mm_rk == kmer_rk) {
					// last kmer is a new occ of same old mm
					_insert(self->tbls[i], kmer_rk, offset + kmer_pos);
				}
			}
		}
	}
	// index end minimisers
	bool still_indexing = true;
	while (still_indexing) {
		still_indexing = false;
		for (size_t i = 0; i < nidx; i++) {
			if (minqueue_len(win_rks[i]) > 1) {
				still_indexing = true;
				rankpos last_mm_rp;
				minqueue_min_cpy(win_rks[i], &last_mm_rp);
				minqueue_remv(win_rks[i]);
				rankpos cur_mm_rp;
				minqueue_min_cpy(win_rks[i], &cur_mm_rp);
				if (last_mm_rp.rank != cur_mm_rp.rank) {
					// new minimiser
					for (minqueue_iter it = minqueue_all_min(win_rks[i]);
					        minqueue_iter_has_next(&it);) {
						const rankpos *mm = minqueue_iter_next(&it);
						_insert(self->tbls[i], mm->rank, offset + mm->pos);
					}
				}
			}
		}
	}
	vec_push_size_t(self->offs, pos);
	self->nseq += 1;
}
