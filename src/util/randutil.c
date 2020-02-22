#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>

#include "bitsandbytes.h"
#include "randutil.h"
#include "string.h"

static bool __randinit = false;

static inline void randinit() {    
    if (!__randinit) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        int usec = tv.tv_usec;
        srand48(usec);
        __randinit = true;
    }
}

void shuffle_arr(void *arr, size_t n, size_t typesize) {
    randinit();
    byte_t tmp[typesize];
    if (n > 1) {
        for (size_t j, i = n-1; i > 0; i--) {
            j = (size_t)(drand48() * (i+1));
            memcpy(tmp, arr + (j * typesize), typesize);
            memcpy(arr + (j * typesize), arr + (i * typesize), typesize);
            memcpy(arr + (i * typesize), tmp, typesize);
        }
    }
}


#define RAND_RANGE_IMPL( TYPE )\
TYPE rand_range_##TYPE(TYPE l, TYPE r) {\
    randinit();\
    assert(r >= l);\
    assert(r < RAND_MAX);\
    return l + (rand() % (r-l));\
}

RAND_RANGE_IMPL(int)
RAND_RANGE_IMPL(size_t)