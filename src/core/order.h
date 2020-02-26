#ifndef ORDER_H
#define ORDER_H


#include <stdbool.h>

typedef int (*cmp_func)(const void *, const void *);

typedef bool (*eq_func)(const void *, const void *);


#endif