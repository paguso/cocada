#include "arrutil.h"
#include "mathutil.h"
#include "new.h"
#include "segtree.h"
#include "vec.h"


struct __segtree {
    size_t range;
    size_t typesize;
    merge_func merge;
    void *init_val;
    vec *tree;
};


static void segtree_reset(segtree *self)
{
    for(size_t i=0, l=2*self->range; i < l ; i++)
    {
        vec_push(self->tree, self->init_val);
    }
}



segtree *segtree_new(size_t range, size_t typesize, merge_func merge, void *init_val)
{
    segtree *ret = NEW(segtree);
    ret->range = range;
    ret->typesize = typesize;
    ret->merge = merge;
    ret->init_val = malloc(sizeof(typesize));
    memcpy(ret->init_val, init_val, typesize);
    ret->tree = vec_new_with_capacity(typesize, 2*range);
    segtree_reset(ret);
    return ret;
}


void segtree_upd(segtree *self, size_t pos, void *val)
{
    pos += self->range;    
    vec_set(self->tree, pos, val);
    for (pos /= 2 ; pos > 1; pos /= 2) {
        self->merge( vec_get(self->tree, 2 * pos), vec_get(self->tree, 2 * pos + 1), 
                     vec_get_mut(self->tree, pos) );
    }
}


void segtree_qry(segtree *self, size_t pos, void *dest)
{

}


void segtree_range_qry(segtree *self, size_t left, size_t right, void *dest)
{
    for (;left < right; left/=2, right/=2) {
        if (IS_ODD(left)) self->merge(vec_get(self->tree, left++), (const void *)dest, dest);
        if (IS_ODD(right)) self->merge((const void *)dest, vec_get(self->tree, --right), dest);
    }
}
