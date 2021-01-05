#include "fastaidx.h"
#include "cstrutil.h"
#include "new.h"
#include "vec.h"


struct _fastaidx
{
    char *path;
    vec *dscs;
    vec *seqs;
};


fastaidx *fastaidx_new(const char *src_path)
{
    fastaidx *ret = NEW(fastaidx);
    ret->path = cstr_clone(src_path);
    ret->dscs = vec_new(sizeof(size_t));
    ret->seqs = vec_new(sizeof(size_t)); 
    return ret; 
}


void fastaidx_dtor(void *ptr, const dtor *dt) {
    fastaidx *self = (fastaidx *)ptr;
    FREE(self->path);
    FREE(self->dscs, vec);
    FREE(self->seqs, vec);
    FREE(self);
}


size_t fastaidx_size(fastaidx *self)
{
    return vec_len(self->dscs);
}


void fastaidx_add(fastaidx *self, size_t dsc_offset, size_t seq_offset)
{
    vec_push_size_t(self->dscs, dsc_offset);
    vec_push_size_t(self->seqs, seq_offset);
}


fasta_rec_offs fastaidx_get(fastaidx *self, size_t rec_no)
{
    fasta_rec_offs ret;
    ret.descr_off = vec_get_size_t(self->dscs, rec_no);
    ret.seq_off = vec_get_size_t(self->seqs, rec_no);
    return ret;
}