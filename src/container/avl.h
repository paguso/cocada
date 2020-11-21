#ifndef AVL_H
#define AVL_H

#include <stdio.h> 

#include "coretype.h" 
#include "new.h"
#include "order.h"


typedef struct _avl avl;

avl *avl_new(size_t typesize, cmp_func cmp);

void avl_dtor(void *ptr, const dtor *dt);

const void *avl_get(avl *self, void *key);

void avl_push(avl *self, void *key);

void avl_print(const avl *self, FILE *stream, void (*prt_val)(FILE *, const void *));


#endif
