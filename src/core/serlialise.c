#include <stdlib.h>
#include <stdio.h>

#include "arrays.h"
#include "hash.h"
#include "hashmap.h"
#include "hashset.h"
#include "mathutil.h"
#include "new.h"
#include "serialise.h"


static ser _ser_null = {.type=ser_null, .addr=NULL, .size=0, .nchd=0, .chd=NULL};


ser *ser_null_new(memmap *mm)
{
    if (!memmap_has_ser(mm, NULL)) {
        memmap_add_ser(mm, &_ser_null);
    }
    return &_ser_null;
}


ser *ser_new(ser_t type, void *addr, size_t size, memmap *mm)
{
    if (addr == NULL) {
        if (!memmap_has_ser(mm, NULL)) {
            memmap_add_ser(mm, &_ser_null);
        }
        return &_ser_null;
    }
    if (memmap_has_ser(mm, addr)) {
        return memmap_get_ser(mm, addr);
    }
    ser *ret = NEW(ser);
    ret->type = type;
    ret->addr = addr;
    ret->size = size;
    ret->nchd = 0;
    ret->chd = NEW_ARR(ser *, 1) ;
    return ret;
}


ser *ser_cons(ser *par, const ser *chd)
{
    if (IS_POW2(par->nchd)) {
	    par->chd = (ser **) realloc(par->chd, ( 2 * par->nchd) * sizeof(ser *));
    }
	par->chd[par->nchd++] = (ser *)chd;
	return par;
}


size_t ser_nchd(ser *self)
{
    return self->nchd;
}

const ser *ser_chd(ser *self, size_t i)
{
    return self->chd[i];
}


ser *serialisable_get_ser(serialisable *trait_obj, memmap *mm)
{
    return trait_obj->vt->get_ser(trait_obj, mm);
}



struct _memmap {
    hashmap *map_ser;
};

memmap *memmap_new() {
    memmap *ret = NEW(memmap);
    return ret;
}

bool memmap_has_ser(memmap *self, void *addr)
{
    if (!self->map_ser) return false;
    return hashmap_has_key(self->map_ser, &addr);
}

ser *memmap_get_ser(memmap *self, void *addr)
{
    if (!self->map_ser) return NULL;
    return (ser *)hashmap_get_rawptr(self->map_ser, &addr);
}

void memmap_add_ser(memmap *self, ser *st)
{
    if (!self->map_ser) {
        self->map_ser = hashmap_new(sizeof(size_t), sizeof(rawptr), ident_hash_size_t, eq_size_t);
    }
    hashmap_set(self->map_ser, &st->addr, &st);
}




void _write(ser *root, FILE *stream) 
{
    byte_t type = (byte_t) root->type;
    byte_t bytesize = (byte_t) root->size;
    size_t addr = (size_t) root->addr;
    fwrite(&type, 1, 1, stream);
    fwrite(&addr, sizeof(size_t), 1, stream);
    switch(root->type) {
        case ser_arr:
        case ser_struct:
            fwrite(&root->size, sizeof(size_t), 1, stream);
            break;
        default:
            fwrite(&bytesize, 1, 1, stream);
            break;
    }
    fwrite(root->addr, root->size, 1, stream);
}


void _visit(ser *root, FILE *stream, hashset *done)
{
    if (hashset_contains_size_t(done, (size_t)(root->addr))) {
        return;
    }
    hashset_add_size_t(done, (size_t)(root->addr));
    FOREACH_IN_ARR(chd, ser*, root->chd, root->nchd) {
        _visit(chd, stream, done);
    }
    _write(root, stream);
}


void serialise(ser *st, FILE *stream) {
    hashset *done = hashset_new(sizeof(size_t), ident_hash_size_t, eq_size_t);
    _visit(st, stream, done);
    FREE(done, hashset);
}


