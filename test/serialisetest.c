#include "CuTest.h"

#include "arrays.h"
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

ser *_get_ser(serialisable *trait_obj, memmap *mm) {
    node *self = trait_obj->impltor;
    if (memmap_has_ser(mm, self)) {
        return memmap_get_ser(mm, self);
    }
    ser *ret = ser_new(ser_struct, self, sizeof(node), mm);
    ser_cons(ret, ser_new(ser_int, &self->val, sizeof(int), mm));
    ser_cons(ret, ser_cons(ser_new(ser_ptr, &self->arr, sizeof(rawptr), mm), 
                            (self->arr) ? 
                            ser_new(ser_arr, self->arr, self->val*sizeof(int), mm):
                            ser_null_new(mm) ));
    ser_cons(ret, ser_cons( ser_new(ser_ptr, &self->next, sizeof(rawptr), mm),
                            (self->next) ? 
                            serialisable_get_ser(node_as_serialisable(self->next), mm):
                            ser_null_new(mm) ));
    return ret;
}

serialisable_vt node_ser_vt = {
    .get_ser = _get_ser    
};

IMPL_TRAIT(node, serialisable);


node *head = NULL;

void test_setup()
{
    int n = 1;
    node **cur = &head;
    node *tail;
    for (int i=1; i<=n; i++) {
        tail = NEW(node);
        tail->_t_serialisable.impltor = tail;
        tail->_t_serialisable.vt = &node_ser_vt;
        tail->val = i;
        tail->arr = NEW_ARR(int, i);
        FILL_ARR(tail->arr, 0, i, i);
        tail->next = NULL;
        (*cur) = tail;
        cur = &tail->next;              
    }    
}


void test_get_ser(CuTest *tc) 
{
    test_setup();
    memmap *mm = memmap_new();
    ser *st = serialisable_get_ser(node_as_serialisable(head), mm);
    FILE *stream = fopen("serialised_node.out", "wb");
    serialise(st, stream);
    fclose(stream);
}


CuSuite *serialise_get_test_suite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_get_ser);
    return suite;
}
