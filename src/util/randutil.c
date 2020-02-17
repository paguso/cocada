#include <stdlib.h>

#include "bitsandbytes.h"
#include "randutil.h"
#include "string.h"

static bool __randinit = false;

static void randinit() {    
    if (!__randinit) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        int usec = tv.tv_usec;
        srand48(usec);
        __randinit = true;
    }
}

void shuffle_arr(void *arr, size_t n, size_t typesize) {
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