#include <stdlib.h>
#include <stdio.h>

#include "arrays.h"
#include "errlog.h"
#include "hash.h"
#include "hashmap.h"
#include "hashset.h"
#include "mathutil.h"
#include "new.h"
#include "serialise.h"
#include "deque.h"


static som *_som_new(som_t type, size_t size, get_som_func get_som)
{
    som *ret = NEW(som);
    ret->type = type;
    ret->get_som = get_som;
    ret->size = size;
    ret->nchd = 0;
    ret->chd = NEW_ARR(sub_som, 1) ;
    return ret;
}


#define IMPL_GET_SER_TYPE(TYPE, ...)\
\
static som* _som_##TYPE = NULL;\
\
som *get_som_##TYPE() {\
    if (_som_##TYPE==NULL) {\
        _som_##TYPE = _som_new(som_##TYPE, sizeof(TYPE), get_som_##TYPE);\
    }\
    return _som_##TYPE;\
}

XX_PRIMITIVES(IMPL_GET_SER_TYPE)




som *som_arr_new() 
{
    return _som_new(som_arr, 0, NULL);
}


som *som_ptr_new()
{
    return _som_new(som_rawptr, sizeof(rawptr), NULL);
}


som *som_struct_new(size_t size, get_som_func get_som)
{
    return _som_new(som_struct, size, get_som);    
}


som *som_proxy_new(get_som_func get_som)
{
    return _som_new(som_proxy, 0, get_som);
}


som *som_cons(som *par, size_t off,  som *chd)
{
    if (IS_POW2(par->nchd)) {
	    par->chd = (sub_som *) realloc(par->chd, ( 2 * par->nchd) * sizeof(sub_som));
    }
    sub_som ss = {.off = off, .chd=chd};
	par->chd[par->nchd++] = ss;
	return par;
}


size_t som_nchd(som *self)
{
    return self->nchd;
}


sub_som som_chd(som *self, size_t i)
{
    return self->chd[i];
}



typedef struct {
    void *start;
    size_t size;
} mem_chunk;


static bool is_written(vec *written, void *addr)
{
    for (size_t i=0, l=vec_len(written); i<l; i++) {
        mem_chunk *chk = (mem_chunk *)vec_get(written, i);
        if ( (size_t)chk->start <= (size_t)addr  &&
             (size_t)addr < (size_t)chk->start + (size_t)chk->size ) { 
            return true;
        }
    }
    return false;
}



static void write_type(som_t typ, FILE *stream) 
{
    byte_t btyp = (byte_t) typ; 
    fwrite(&btyp, 1, 1, stream);
}


static void write_addr(void *ptr, FILE *stream)
{
    size_t addr = (size_t) ptr;
    fwrite(&addr, sizeof(size_t), 1, stream);
}


#define WRITE_PRIM(TYPE, ...)\
static void write_##TYPE(void *obj, FILE *stream, vec *written)\
{\
    write_type(som_##TYPE, stream);\
    write_addr(obj, stream);\
    fwrite(obj, sizeof(TYPE), 1, stream);\
    mem_chunk ck = {.start = obj, .size=sizeof(TYPE)};\
    vec_push(written, &ck);\
}

XX_PRIMITIVES(WRITE_PRIM)
WRITE_PRIM(rawptr)


#define CASE_WRITE_PRIM(TYPE, ...)\
case som_##TYPE:\
    write_##TYPE(obj, stream, written);\
    break;\


typedef struct {
    void *obj;
    som *model;
} obj_model;


static void write_obj(som *model, void *obj, FILE *stream, vec *written) 
{
    deque *dq = deque_new(sizeof(obj_model));
    obj_model om = {.obj=obj, .model=model}; 
    deque_push_back(dq, &om);
    while (!deque_empty(dq)) {
        deque_pop_front(dq, &om);
        model = om.model;
        while (model->type == som_proxy) {
            model = model->get_som();
        }
        obj = om.obj;
        if (is_written(written, obj)) {
            continue;
        }
        switch (model->type) {
            XX_PRIMITIVES(CASE_WRITE_PRIM)
            case som_rawptr:
                write_rawptr(obj, stream, written);
                if (som_nchd(model) > 0) {
                    sub_som chd = som_chd(model, 0);
                    om.model = chd.chd;
                    om.obj = *((rawptr *)obj);
                    deque_push_back(dq, &om);
                }
            case som_arr:
                write_type(som_arr, stream);
                write_addr(obj, stream);
                
                break;
            case som_struct:
                ;
                byte_t typ = (byte_t)som_struct;
                fwrite(&typ, 1, 1, stream);
                //fwrite(&root->size, sizeof(size_t), 1, stream);
                size_t addr = (size_t) obj;
                fwrite(&addr, sizeof(size_t), 1, stream);
                void *str_obj = obj;
                for (size_t i=0; i<som_nchd(model); i++) {
                    sub_som chd = som_chd(model, i);
                    obj = str_obj + chd.off;
                    som *field_som = chd.chd;
                    while (field_som->type == som_proxy) {
                        field_som = field_som->get_som();
                    }
                    switch (field_som->type) {
                    XX_PRIMITIVES(CASE_WRITE_PRIM)
                    case som_rawptr/* constant-expression */:
                        /* code */
                        break;
                    
                    default:
                        break;
                    }
                }
                break;
            default:
                ERROR("Unrecognised somialisable object type.")
                break;
        }
    }
}



