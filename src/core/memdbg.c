/*
 * COCADA - COCADA Collection of Algorithms and DAta Structures
 *
 * Copyright (C) 2016  Paulo G S Fonseca
 *
 * This program is free software; you can redistribute it and/or mody
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>


#define  MEM_DEBUG_OFF
#include "memdbg.h"

//  Implement the tally as a linear probing closed hashtable
//  with Fibonacci hashing

static size_t alloc_no = 0;

typedef enum {
	FREE, // must be zero
	ACTIVE,
	DELETED
} flag_t;

typedef struct {
	size_t alloc_no;
	void *addr;
	size_t size;
	flag_t flag;
} memchunk;

typedef struct {
	size_t nact;
	size_t ndel;
	size_t cap;
	size_t total;
	memchunk *data;
} memtable;

static const size_t MIN_CAP = 128;
static const double GROW_BY = 1.62;
static const double MAX_LOAD = 0.66;
static const double MIN_LOAD = 0.25;

static memtable tally = {.nact = 0, .ndel = 0, .cap = 0, .data = NULL};


static void memtable_init(memtable *tally)
{
	if (tally->cap) return;
	tally->nact = 0;
	tally->ndel = 0;
	tally->cap = MIN_CAP;
	tally->data = calloc(tally->cap, sizeof(memchunk));
	tally->total = 0;
}


static size_t hash(void *addr, size_t capacity)
{
	size_t h = (size_t)(addr);
	h *= 11400714819323198485llu;
	return h % capacity;
}

/* Ok but unused
static memchunk memtable_get(memtable *tally, void *addr)
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
*/

static void memtable_check_and_resize(memtable *tally)
{
	double load = (double)( tally->nact + tally->ndel ) / (double)(tally->cap);
	size_t newcap = tally->cap;
	if (load < MIN_LOAD) {
		newcap = tally->cap / GROW_BY;
		newcap = (newcap > MIN_CAP) ? newcap : MIN_CAP;
	}
	else if (load > MAX_LOAD) {
		newcap = tally->cap * GROW_BY;
	}
	if (newcap != tally->cap) {
		memchunk *newdata = (memchunk *)calloc(newcap, sizeof(memchunk));
		for (size_t i = 0; i < tally->cap; i++) {
			if (tally->data[i].flag == ACTIVE) {
				size_t pos = hash(tally->data[i].addr, newcap);
				for (; newdata[pos].flag != FREE; pos = (pos + 1) % newcap);
				newdata[pos] = tally->data[i];
			}
		}
		free(tally->data);
		tally->data = newdata;
		tally->ndel = 0;
		tally->cap = newcap;
	}
}


static void memtable_set(memtable *tally, void *addr, size_t size)
{
#ifndef MEM_DEBUG
	return;
#endif
	memtable_init(tally);
	size_t pos = hash(addr, tally->cap);
	while (tally->data[pos].flag != FREE) {
		if (tally->data[pos].flag == ACTIVE &&
		        tally->data[pos].addr == addr) {
			tally->total += (size - tally->data[pos].size);
			tally->data[pos].size = size;
			return;
		}
		pos = (pos + 1) % tally->cap;
	}
	tally->data[pos].alloc_no = alloc_no++;
	tally->data[pos].addr = addr;
	tally->data[pos].size = size;
	tally->data[pos].flag = ACTIVE;
	tally->total += size;
	tally->nact++;
	memtable_check_and_resize(tally);
}


static void memtable_unset(memtable *tally, void *addr)
{
#ifndef MEM_DEBUG
	return;
#endif
	memtable_init(tally);
	size_t pos = hash(addr, tally->cap);
	while (tally->data[pos].flag != FREE) {
		if (tally->data[pos].flag == ACTIVE &&
		        tally->data[pos].addr == addr) {
			tally->data[pos].flag = DELETED;
			tally->ndel++;
			tally->nact--;
			tally->total -= tally->data[pos].size;
			break;
		}
		pos = (pos + 1) % tally->cap;
	}
	memtable_check_and_resize(tally);
}


static const char *prefixes[7] = {"", "Kilo", "Mega", "Giga", "Tera", "Peta", "Exa"};

typedef struct {
	double size;
	const char *prefix;
} hr_t;

static hr_t human_readable(size_t size)
{
	double total = (double) size;
	size_t i = 0;
	while (i < 6 && size >> 10) {
		size >>= 10;
		i++;
	}
	hr_t ret = {.size = total / (double)(1 << (i * 10)), .prefix = prefixes[i]};
	return ret;
}


typedef struct {
	size_t no, pos;
} pair;

static int cmp_pair(const void *l, const void *r)
{
	if ( ((pair *)l)->no == ((pair *)r)->no )
		return 0;
	else if ( ((pair *)l)->no < ((pair *)r)->no )
		return -1;
	else
		return +1;
}

static void memtable_print_stats(FILE *stream, memtable *tally,
                                 bool print_chunks)
{
	fprintf(stream,
	        "================================================================================\n");
	fprintf(stream, "Heap memory info\n");
	if (print_chunks) {
		fprintf(stream,
		        "--------------------------------------------------------------------------------\n");
		fprintf(stream, "Chunks in chronological order of allocation\n\n");
		size_t n = tally->nact, k=0;
		pair *pairs = (pair *)malloc(n * sizeof(pair));
		for (size_t i=0; i < tally->cap; i++) {
			if (tally->data[i].flag == ACTIVE) {
				pairs[k].no = tally->data[i].alloc_no;
				pairs[k].pos = i;
				k++;
			}
		}
		assert(k==n);
		qsort(pairs, n, sizeof(pair), cmp_pair);
		for (size_t i=0; i < n; i++) {
			size_t pos = pairs[i].pos;
			fprintf(stream, "#%zu:  %zu bytes @%p\n",
			        pairs[i].no, tally->data[pos].size, tally->data[pos].addr);
		}
		fprintf(stream,
		        "--------------------------------------------------------------------------------\n");
		free(pairs);
	}
	hr_t hrsize = human_readable(tally->total);
	fprintf(stream, "Heap memory usage summary\n\n");
	fprintf(stream, "Total memory  : %zu bytes (%.3lf %sbytes)\n",
	        tally->total, hrsize.size, hrsize.prefix);
	fprintf(stream, "Nb. of chunks : %zu\n", tally->nact);

	fprintf(stream,
	        "================================================================================\n");
}


void memdbg_print_stats(FILE *stream, bool print_chunks)
{
	memtable_print_stats(stream, &tally, print_chunks);
}


void memdbg_reset()
{
	tally.cap = 0;
	tally.nact = 0;
	tally.total = 0;
	tally.ndel = 0;
	free(tally.data);
	tally.data = NULL;
	alloc_no = 0;
}


size_t memdbg_total()
{
	return tally.total;
}


size_t memdbg_nchunks()
{
	return tally.nact;
}


bool memdbg_is_empty()
{
	return (tally.total == 0 && tally.nact == 0);
}


void *memdbg_malloc(size_t size, char *file, int line)
{
	void *ret = malloc(size);
	memtable_set(&tally, ret, size);
#ifdef MEM_DEBUG_PRINT_ALL
	hr_t hrsize = human_readable(tally.total);
	printf("malloc [%s:%d]  %zu bytes @%p (total: %.3lf %sbytes)\n",
	        file, line, size, ret, hrsize.size, hrsize.prefix);
#endif
	return ret;
}


void *memdbg_calloc(size_t nmemb, size_t size, char *file, int line)
{
	void *ret = calloc(nmemb, size);
	memtable_set(&tally, ret, nmemb * size);
#ifdef MEM_DEBUG_PRINT_ALL
	hr_t hrsize = human_readable(tally.total);
	printf("calloc [%s:%d]  %zu bytes @%p (total: %.3lf %sbytes)\n",
	       file, line, nmemb * size, ret, hrsize.size, hrsize.prefix);
#endif
	return ret;
}


void *memdbg_realloc(void *ptr, size_t size, char *file, int line)
{
	void *ret = realloc(ptr, size);
	if (ret != ptr) {
		memtable_unset(&tally, ptr);
	}
	memtable_set(&tally, ret, size);
#ifdef MEM_DEBUG_PRINT_ALL
	hr_t hrsize = human_readable(tally.total);
	printf("realloc [%s:%d]  %zu bytes @%p (total: %.3lf %sbytes)\n",
	       file, line, size, ret, hrsize.size, hrsize.prefix);
#endif
	return ret;
}


void memdbg_free(void *ptr, char *file, int line)
{
	free(ptr);
#ifdef MEM_DEBUG_PRINT_ALL
	hr_t hrsize = human_readable(tally.total);
	printf("free [%s:%d] @%p (total: %.3lf %sbytes)\n",
	       file, line, ptr, hrsize.size, hrsize.prefix);
#endif
	memtable_unset(&tally, ptr);
}

#undef MEM_DEBUG_OFF