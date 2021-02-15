#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>


#define  DEBUG_MEM_OFF
#include "memdbg.h"

void *ccd_malloc(size_t size){//}, char *file, int line) {
    void *ret = malloc(size);
    //printf("malloc [%s,%d]:  %zu bytes @%p\n", file, line, size, ret);
    printf("malloc:  %zu bytes @%p\n",  size, ret);
    return ret;
}

void ccd_free(void *ptr){//}, char *file, int line) {
    free(ptr);
    //printf("free [%s, %d]: @%p\n", file, line, ptr);
    printf("free: @%p\n", ptr);
}

#undef DEBUG_MEM_OFF