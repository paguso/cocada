#ifndef SEGTREE_H
#define SEGTREE_H


typedef struct __segtree segtree;

typedef void (*merge_func)(const void  *left, const void *right, void *dest);


segtree *segtree_new(size_t range, size_t typesize, merge_func merge, void *init_val);

void segtree_upd(segtree *self, size_t pos, void *val);

void *segtree_qry(segtree *self, size_t pos);

void segtree_range_qry(segtree *self, size_t left, size_t right, void *dest)

#endif