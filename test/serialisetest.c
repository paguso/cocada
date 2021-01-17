#include "CuTest.h"

#include "arrays.h"
#include "new.h"
#include "trait.h"
#include "serialise.h"

#define STR_SEQ_INFO(STRUCT)\
static const STRUCT __dummy_##STRUCT;\

#define STR_OFFSET(STRUCT, FIELD)\
((size_t)(&__dummy_##STRUCT.FIELD) - (size_t)(&(__dummy_##STRUCT))) 


typedef struct _node {
    int val;
    int *arr;
    struct _node *next;
} node;

som *_node_som = NULL;
STR_SEQ_INFO(node)

som *get_node_som () {
    if (_node_som==NULL) {
        _node_som = som_struct_new(sizeof(node), get_node_som);
    }
    som_cons(_node_som, STR_OFFSET(node, val), get_som_int());
    som_cons(_node_som, STR_OFFSET(node, arr), 
                    som_cons(som_ptr_new(), 0,
                             som_cons(som_arr_new(), 0, get_som_int())));
    som_cons(_node_som, STR_OFFSET(node, next), 
                    som_cons(som_ptr_new(), 0, 
                             som_struct_new(sizeof(node), get_node_som)));
    return _node_som;
}



node *head = NULL;

void test_setup()
{
    int n = 3;
    node **cur = &head;
    node *tail;
    for (int i=1; i<=n; i++) {
        tail = NEW(node);
        tail->val = i;
        tail->arr = NEW_ARR(int, i);
        FILL_ARR(tail->arr, 0, i, i);
        tail->next = NULL;
        (*cur) = tail;
        cur = &tail->next;              
    }    
}


void test_get_som(CuTest *tc) 
{
    test_setup();
    FILE *stream = fopen("serialised_node.out", "wb");
    som *st = get_node_som();
    //somialise(st, stream);
    fclose(stream);
}


CuSuite *serialise_get_test_suite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_get_som);
    return suite;
}
