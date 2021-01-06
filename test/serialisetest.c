#include "CuTest.h"

#include "new.h"
#include "trait.h"
#include "serialise.h"


typedef struct _node {
    serialisable _t_serialisable;
    int val;
    int *arr;
    struct _node *next;
} node;

DECL_TRAIT(node, serialisable);

ser *_get_ser(serialisable *trait_obj) {
    node *self = trait_obj->impltor;
    ser *ret = ser_new(ser_struct, self, sizeof(node));
    ser_cons(ret, ser_new(ser_int, &self->val, sizeof(int)));
    ser_cons(ret, ser_cons(ser_new(ser_ptr, &self->arr, sizeof(rawptr)), ser_new(ser_arr, self->arr, self->val*sizeof(int))));
    ser_cons(ret, ser_cons( ser_new(ser_ptr, &self->next, sizeof(rawptr)), 
                            get_ser(node_as_serialisable(self->next))
    ));
    return ret;
}

serialisable_vt node_ser_vt = {
    .get_ser = _get_ser    
};

IMPL_TRAIT(node, serialisable);