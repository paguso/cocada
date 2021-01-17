#ifndef SERIALISE_H
#define SERIALISE_H

#include <stdio.h>

#include "coretype.h"
#include "hashset.h"
#include "vec.h"



#define SOM_T(TYPE,...) som_##TYPE,

typedef enum {
    XX_PRIMITIVES(SOM_T)
    SOM_T(rawptr)
    SOM_T(arr)
    SOM_T(struct)
    SOM_T(proxy)
}
som_t;



typedef struct _som som;

typedef som* (*get_som_func) ();

typedef struct _sub_som {
    size_t off;
    som *chd;
} sub_som;


struct _som {
    som_t  type;
    get_som_func get_som;
    size_t size;
    size_t nchd;
    sub_som *chd;
};


#define DECL_GET_SOM_TYPE(TYPE, ...) \
som* get_som_##TYPE();

XX_PRIMITIVES(DECL_GET_SOM_TYPE)

som *som_arr_new();

som *som_ptr_new();

som *som_struct_new(size_t size, get_som_func get_som);

som *som_proxy_new(get_som_func get_som);

som *som_cons(som *par, size_t off, som *chd);

size_t som_nchd(som *self);

sub_som som_chd(som *self, size_t i);

void serialise(void *obj, som *model, FILE *stream);

#endif