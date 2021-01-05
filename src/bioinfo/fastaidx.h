#ifndef FASTAIDX_H
#define FASTAIDX_H

#include <stddef.h>

#include "new.h"

typedef struct _fastaidx fastaidx;

fastaidx *fastaidx_new(const char *src_path);

void fastaidx_dtor(void *ptr, const dtor *dt);

size_t fastaidx_size(fastaidx *self);

void fastaidx_add(fastaidx *self, size_t descr_offset, size_t seq_offset);

typedef struct {
    size_t descr_off;
    size_t seq_off;
} fasta_rec_offs;

fasta_rec_offs fastaidx_get(fastaidx *self, size_t rec_no);

#endif