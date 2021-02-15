#ifndef MEMDBG_H
#define MEMDBG_H

#include <stddef.h>

void *ccd_malloc(size_t size, char *file, int line);

void *ccd_calloc(size_t nmemb, size_t size, char *file, int line);

void *ccd_realloc(void *ptr, size_t size, char *file, int line);

void ccd_free(void *ptr, char *file, int line);


#ifndef DEBUG_MEM_OFF

#warning "Compiling with debug memory on"

#define malloc(size) ccd_malloc(size, __FILE__ , __LINE__)

#define calloc(nmemb, size) ccd_malloc(nmemb, size, __FILE__ , __LINE__)

#define realloc(ptr, size) ccd_malloc(ptr, size, __FILE__ , __LINE__)

#define free(ptr) ccd_free(ptr,  __FILE__ , __LINE__)


#endif


#endif