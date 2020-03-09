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

#include "mmindex.h"

struct _mmindex {
    alphabet *ab;
    size_t nparam;
    size_t *w;
    size_t *k;
    size_t max_wlen;
    hashmap **tbls;
    size_t nstr;
    vec *offs;
};


mmindex *mmindex_new(alphabet *ab, size_t n, size_t *w, size_t *k)
{
    mmindex *ret = NEW(mmindex);
    ret->ab = ab;
    ret->nparam = n;
    ret->w = w;
    ret->k = k;
    ret->max_wlen = 0;
    for (size_t i =0; i<n; i++)
        ret->max_wlen = (w[i]+k[i] > ret->max_wlen) ? w[i]+k[i] : ret->max_wlen;
    ret->tbls = NEW_ARR(hashmap*, ret->nparam);
    for (size_t i=0; i<ret->nparam; i++) {
        ret->tbls[i] = hashmap_new( sizeof(uint64_t), vec_sizeof(), 
                                    ident_hash_uint64_t, eq_uint64_t );
    }
    ret->nstr = 0;
    ret->offs = vec_new(sizeof(size_t));
    return ret;
}

void mmindex_dispose(void *ptr, const dtor *dt)
{
    mmindex *mm = (mmindex *)ptr;
    alphabet_free(mm->ab);
    FREE(mm->w);
    FREE(mm->k);
    dtor *hmdt = dtor_cons( dtor_cons( DTOR(hashmap), empty_dtor() ), DTOR(vec) );
    for (size_t i=0; i < mm->nparam; i++) {
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

int cmp_rankpos(const void *pl, const void *pr) {
    return cmp_uint64_t(pl, pr);
}


void mmindex_index(mmindex *mmidx, strstream *str)
{
    size_t nidx = mmidx->nparam;
    size_t offset = vec_last_size_t(mmidx->offs);

    size_t max_win_len = mmidx->max_wlen;
    
    xstring *window = xstring_new_with_capacity(strstream_sizeof_char(str), max_win_len);

    vec *win_rks = vec_new_with_capacity(sizeof(minqueue *), nidx);
    VEC_FILL( win_rks, minqueue_new(sizeof(rankpos), cmp_rankpos), nidx ); //window kmers ranks 
    vec *prev_mm_rk = vec_new_with_capacity(sizeof(size_t), nidx); // rank of previous window minimiser
    VEC_FILL( prev_mm_rk, 0, )
    /* 
    let mut win_rks: Vec<MQueue<(TMmRank, usize)>> = vec![MQueue::new_min(); nidx]; // window kmers ranks
    let mut prev_mm_rk = vec![0 as TMmRank; nidx]; // rank of previous window minimiser
    let mut prev_right_rk = vec![0 as TMmRank; nidx]; // rank of previous window rightmost kmer

    let mut pos = 0;
    while let Some(c) = s.get()? {
        // perpare window
        if pos >= max_win_len {
            window.rotate_left(1);
            window[max_win_len - 1] = c;
        } else {
            //read in first window
            window.push(c);
        }
        pos += 1;
        for i in 0..nidx {
            if pos == self.k[i] {
                let kmer_rk = self.hasher[i].hash(&window[window.len() - self.k[i]..]);
                prev_right_rk[i] = kmer_rk;
                prev_mm_rk[i] = kmer_rk; 
                win_rks[i].push((kmer_rk, pos - self.k[i]));
                // initial end minimisers are all indexed
                self.insert(i, kmer_rk, offset + pos - self.k[i]);
            } else if pos > self.k[i] {
                // get previous windows minimiser
                // let (last_mm_rk, _last_mm_pos) = win_rks[i].xtr().unwrap().clone();
                // compute new last kmer rank and add it to the new window
                let kmer_rk = self.hasher[i].roll_hash(
                    &window[window.len() - self.k[i] - 1..window.len() - 1],
                    prev_right_rk[i],
                    c,
                );
                let kmer_pos = pos - self.k[i];
                prev_right_rk[i] = kmer_rk;
                // dequeue the first kmer of previous window if it is full
                if pos > self.w[i] + self.k[i] - 1 {
                    win_rks[i].pop();
                }
                // and add new kmer
                win_rks[i].push((kmer_rk, kmer_pos));
                // then get current window miminiser
                let cur_mm_rk = win_rks[i].xtr().unwrap().0;
                if self.w[i] == 1 || prev_mm_rk[i] != cur_mm_rk {
                    // new minimiser. add all its occurrences
                    for &(rk, p) in win_rks[i].xtr_iter() {
                        self.insert(i, rk, offset + p);
                    }
                    prev_mm_rk[i] = cur_mm_rk;
                } else if cur_mm_rk == kmer_rk {
                    // last kmer is a new occ of same old mm
                    self.insert(i, kmer_rk, offset + kmer_pos);
                }
            }
        }
    }
    // index end minimisers
    let mut still_indexing = true;
    while still_indexing {
        still_indexing = false;
        for i in 0..nidx {
            if win_rks[i].len() > 1 {
                still_indexing = true;
                let (last_mm_rk, _last_mm_pos) = win_rks[i].xtr().unwrap().clone();
                win_rks[i].pop();
                let (cur_mm_rk, _cur_mm_pos) = win_rks[i].xtr().unwrap().clone();
                if last_mm_rk != cur_mm_rk {
                    // new minimiser
                    for &(rk, p) in win_rks[i].xtr_iter() {
                        self.insert(i, rk, offset + p);
                    }
                }
            }
        }
    }
    self.offs.push(pos);
    self.nseq += 1;
    //println!("nseq={0} offs={1:?}", self.nseq, self.offs);
    Ok(())
    */
}