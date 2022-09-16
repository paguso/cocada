#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "bitarr.h"
#include "bitvec.h"
#include "mathutil.h"
#include "new.h"
#include "order.h"
#include "roaring.h"
#include "vec.h"

#define MAX_ARRAY_SIZE 4096
#define BITVEC_SIZE (1<<16)

#define MSB(u32_) ((u32_) >> 16)
#define LSB(u32_) ((u32_) & 0x0000FFFF)


typedef enum
{
    EMPTY = 0,
    ARRAY_TYPE = 1,
    BITVEC_TYPE = 2
} ctnr_type;


typedef struct
{
    ctnr_type type;
    size_t card;
    void *data;
} ctnr_t;


struct _roaringbitvec
{
    size_t size;
    size_t ncntrs;
    size_t last_ctnr_sz;
    ctnr_t *ctnrs;
};


size_t roaringbitvec_memsize(roaringbitvec *self) 
{
    size_t ret = sizeof(struct _roaringbitvec);
    ret += self->ncntrs * sizeof(ctnr_t);
    for (size_t i = 0; i < self->ncntrs; i++) {
        ctnr_t *ctnr = self->ctnrs + i;
        switch (ctnr->type)
        {
        case EMPTY:
            break;
        case ARRAY_TYPE:
            ret += vec_memsize(ctnr->data);
            break; 
        case BITVEC_TYPE:
            ret += bitvec_memsize(ctnr->data);
            break;
        default:
            break;
        }
    }
    return ret; 
}



void arrctnr_init(ctnr_t *ctnr)
{
    ctnr->type = ARRAY_TYPE;
    ctnr->data = vec_new_uint16_t();
}


size_t arrctnr_succ(ctnr_t *ctnr, uint16_t val)
{
    if (vec_len(ctnr->data) == 0 || val <= vec_first_uint16_t(ctnr->data)) {
        return 0;
    }
    else if (vec_last_uint16_t(ctnr->data) < val) {
        return vec_len(ctnr->data);
    }
    size_t l = 0, r = vec_len(ctnr->data); // successor in (l,r]
    while ((r - l) > 1) {
        size_t m = MEAN(l, r);
        if (val <= vec_get_uint16_t(ctnr->data, m)) {
            r = m;
        } else {
            l = m;
        }
    }
    return r;
}


bool arrctnr_get(ctnr_t *ctnr, uint16_t index)
{
    return vec_bsearch(ctnr->data, &index, cmp_uint16_t) < vec_len(ctnr->data);
}


void arrctnr_set(ctnr_t *ctnr, uint16_t index, bool val)
{
    size_t pos = arrctnr_succ(ctnr, index);
    bool ins;
    if ((pos == vec_len(ctnr->data)) || (vec_get_uint16_t(ctnr->data, pos) != index)) {
        if (val) {
            vec_ins_uint16_t(ctnr->data, pos, index);
            ctnr->card++;
        }
    } else { // ctnr->data[pos] == val
        if (!val) {
            vec_del(ctnr->data, pos);
            ctnr->card--;
        }
    }
}


void convert_arr_to_bitvec_ctnr(ctnr_t *ctnr) 
{
    vec *v = ctnr->data;
    bitvec *b = bitvec_new_with_capacity(BITVEC_SIZE);
    for (size_t i = 0, l = vec_len(v); i < l; i++) {
        bitvec_set_bit(b, vec_get_uint16_t(v, i), 1);
    }
    ctnr->data = b;
    ctnr->type = BITVEC_TYPE;
    DESTROY_FLAT(v, vec);
}


void convert_bitvec_to_arr_ctnr(ctnr_t *ctnr) 
{
    vec *v = vec_new_with_capacity(sizeof(uint16_t), ctnr->card);
    bitvec *b = ctnr->data;
    for (uint16_t i = 0; i < BITVEC_SIZE; i++) {
        if (bitvec_get_bit(b, i)) {
            vec_push_uint16_t(v, i);
        }
    }
    ctnr->data = v;
    ctnr->type = ARRAY_TYPE;
    bitvec_free(b);
}


void bitvecctnr_init(ctnr_t *ctnr)
{
    ctnr->type = BITVEC_TYPE;
    ctnr->data = bitvec_new_with_capacity(BITVEC_SIZE);
}


bool bitvecctnr_get(ctnr_t *ctnr, uint16_t index)
{
    return bitvec_get_bit((const bitvec *)ctnr->data, index);
}


void bitvecctnr_set(ctnr_t *ctnr, uint16_t index, bool val)
{
    if (bitvec_get_bit(ctnr->data, index) != val)
    {
        bitvec_set_bit(ctnr->data, index, val);
        ctnr->card += ((val) ? 1 : -1);
    }
}

roaringbitvec *roaringbitvec_new(size_t n)
{
    roaringbitvec *ret = NEW(roaringbitvec);
    ret->size = n;
    ret->ncntrs = (size_t)DIVCEIL(n, BITVEC_SIZE);
    ret->last_ctnr_sz = ret->size % BITVEC_SIZE;
    ret->ctnrs = calloc(ret->ncntrs, sizeof(ctnr_t));
    for (size_t i = 0; i < ret->ncntrs; i++)
    {
        ret->ctnrs[i] = (ctnr_t){.type = EMPTY, .card = 0, .data = NULL};
    }
    return ret;
}


roaringbitvec *roaringbitvec_new_from_bitarr(byte_t *b, size_t n)
{
    setbuf(stdout, NULL);
    roaringbitvec *ret = roaringbitvec_new(n);
    for (size_t i = 0; i < n; i++) {
        if (bitarr_get_bit(b, i)) {
            //printf("set rbr[%zu]\n",i);
            roaringbitvec_set(ret, i, 1);
        }
    }
    return ret;
}


void roaringbitvec_free(roaringbitvec *self)
{
    for (size_t i = 0; i < self->ncntrs; i++) {
        switch (self->ctnrs[i].type) {
        case ARRAY_TYPE:
            DESTROY_FLAT(self->ctnrs[i].data, vec);
            break;
        case BITVEC_TYPE:
            bitvec_free(self->ctnrs[i].data);
            break;
        default:
            break;
        }
    }
    FREE(self->ctnrs);
    FREE(self);
}


size_t roaringbitvec_card(roaringbitvec *self)
{
    size_t ret = 0;
    for (size_t i = 0; i<self->ncntrs; i++) {
        ret += self->ctnrs[i].card;
    }
    return ret;
}


void roaringbitvec_set(roaringbitvec *self, uint32_t pos, bool val)
{
    assert(pos < self->size);
    uint16_t bucket = MSB(pos);
    uint16_t index = LSB(pos);
    ctnr_t *ctnr = self->ctnrs + bucket;
    switch (ctnr->type) {
    case EMPTY:
        if (val) {
            arrctnr_init(ctnr);
        }
    case ARRAY_TYPE:
        arrctnr_set(ctnr, index, val);
        if (ctnr->card > MAX_ARRAY_SIZE) {
            convert_arr_to_bitvec_ctnr(ctnr);
        }
        break;
    case BITVEC_TYPE:
        bitvecctnr_set(ctnr, index, val);
        if (ctnr->card <= MAX_ARRAY_SIZE) {
            convert_bitvec_to_arr_ctnr(ctnr);
        }
        break;
    default:
        break;
    }
}

bool roaringbitvec_get(roaringbitvec *self, uint32_t pos)
{
    assert(pos < self->size);
    uint16_t bucket = MSB(pos);
    uint16_t index = LSB(pos);
    ctnr_t *ctnr = self->ctnrs + bucket;
    switch (ctnr->type) {
    case EMPTY:
        return 0;
        break;
    case ARRAY_TYPE:
        return arrctnr_get(ctnr, index);
        break;
    case BITVEC_TYPE:
        return bitvecctnr_get(ctnr, index);
    default:
        break;
    }
}


void roaringbitvec_fit(roaringbitvec *self)
{
    for (size_t i = 0; i < self->ncntrs; i++) {
        ctnr_t *ctnr = self->ctnrs + i;
        switch (ctnr->type)
        {
        case EMPTY:
            break;
        case ARRAY_TYPE:
            vec_fit(ctnr->data);
            break; 
        case BITVEC_TYPE:
            bitvec_fit(ctnr->data);
            break;
        default:
            break;
        }
    }
}


void roaringbitvec_fprint(FILE *stream, roaringbitvec *self)
{
    char *types[3] = {"EMPTY", "ARRAY", "BITVEC"};
    fprintf(stream, "roaringbitvec@%p {\n", self);
    fprintf(stream, "   size=%zu\n", self->size);
    for (size_t b = 0; b < self->ncntrs; b++)
    {
        fprintf(stream, "   [%zu] type=%s card=%d\n", b, types[self->ctnrs[b].type], self->ctnrs[b].card);
    }
    fprintf(stream, "}\n");
}
