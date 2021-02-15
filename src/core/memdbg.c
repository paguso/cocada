#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>

#define  DEBUG_MEM_OFF
#include "memdbg.h"


/*---------------------------------------------*/

typedef enum {
    FREE, // must be zero
    ACTIVE,
    DELETED
} flag_t;

typedef struct {
    void *addr;
    size_t size;
    flag_t flag;
} memchunk;

typedef struct {
    size_t nact;
    size_t ndel;
    size_t cap;
    memchunk *data;
} memtable;

static const size_t MIN_CAP = 128;
static const double GROW_BY = 1.62;
static const double MAX_LOAD = 0.66;
static const double MIN_LOAD = 0.25;



memtable tally = {.nact = 0, .ndel = 0, .cap = 0, .data = NULL};

void memtable_init(memtable *tally) {
    if (tally->cap) return;
    tally->nact = 0;
    tally->ndel = 0;
    tally->cap = MIN_CAP;
    tally->data = calloc(tally->cap, sizeof(memchunk));        
}


size_t hash(void *addr, size_t capacity) {
    size_t h = (size_t)(addr);
    h *= 11400714819323198485llu;
    return h % capacity;
}


memchunk memtable_get(memtable *tally, void *addr) 
{
    memtable_init(tally);
    size_t pos = hash(addr, tally->cap);
    while (tally->data[pos].flag != FREE) {
        if (tally->data[pos].flag == ACTIVE && 
            tally->data[pos].addr == addr) {
            return tally->data[pos];
        }
        pos = (pos + 1) % tally->cap;
    }
    return tally->data[pos];
}


void memtable_check_and_resize(memtable *tally)
{
    double load = (double)( tally->nact + tally->ndel ) / (double)(tally->cap);
    size_t newcap;
    if (load < MIN_LOAD) {
        newcap = tally->cap / GROW_BY;
        newcap = (newcap > MIN_CAP) ? newcap : MIN_CAP;        
    } else if (load > MAX_LOAD) {
        newcap = tally->cap * GROW_BY;
    }
    if (newcap != tally->cap) {
        memchunk *newdata = calloc(newcap, sizeof(memchunk));
        for (size_t i = 0; i < tally->cap; i++) {
            if (tally->data[i].flag == ACTIVE) {
                size_t pos = hash(tally->data[i].addr, newcap); 
                for(; newdata[pos].flag != FREE; pos = (pos + 1) % newcap);
                newdata[pos] = tally->data[i];
            }
        }
        free(tally->data);
        tally->data = newdata;
        tally->ndel = 0;
        tally->cap = newcap;        
    }
}


void memtable_set(memtable *tally, void *addr, size_t size) 
{
    memtable_init(tally);
    size_t pos = hash(addr, tally->cap);
    while (tally->data[pos].flag != FREE) {
        if (tally->data[pos].flag == ACTIVE &&
            tally->data[pos].addr == addr) {
            tally->data[pos].size = size;
            return;
        }
        pos = (pos + 1) % tally->cap;
    }
    tally->data[pos].addr = addr;
    tally->data[pos].size = size;
    tally->data[pos].flag = ACTIVE;
    tally->nact++;
    memtable_check_and_resize(tally);
}
    

void memtable_unset(memtable *tally, void *addr) 
{
    memtable_init(tally);
    size_t pos = hash(addr, tally->cap);
    while (tally->data[pos].flag != FREE) {
        if (tally->data[pos].flag == ACTIVE &&
            tally->data[pos].addr == addr) {
            tally->data[pos].flag = DELETED;
            tally->ndel++;
            break;
        }
        pos = (pos + 1) % tally->cap;
    }
    memtable_check_and_resize(tally);
}



void *ccd_malloc(size_t size, char *file, int line)
{
    void *ret = malloc(size);
    printf("malloc [%s line %d]:  %zu bytes @%p\n", file, line, size, ret);
    memtable_set(&tally, ret, size);    
    return ret;
}


void *ccd_calloc(size_t nmemb, size_t size, char *file, int line) 
{
    void *ret = calloc(nmemb, size);
    printf("calloc [%s line %d]:  %zu bytes @%p\n", file, line, nmemb * size, ret);
    memtable_set(&tally, ret, nmemb * size);    
    return ret;
}


void *ccd_realloc(void *ptr, size_t size, char *file, int line)
{
    void *ret = realloc(ptr, size);
    printf("realloc [%s line %d]:  %zu bytes @%p\n", file, line, size, ret);
    if (ret != ptr) {
        memtable_unset(&tally, ptr);
    }
    memtable_set(&tally, ret, size);    
    return ret;
}


void ccd_free(void *ptr, char *file, int line) {
    free(ptr);
    printf("free [%s line %d]: @%p\n", file, line, ptr);
    memtable_unset(&tally, ptr);
}

#undef DEBUG_MEM_OFF