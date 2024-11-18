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


#define  MEM_DEBUG_OFF
#include "coretype.h"
#include "errlog.h"
#include "memdbg.h"

//  Implement the tally as a linear probing closed hashtable
//  with Fibonacci hashing

// hashtable entry status type
typedef enum {
	FREE, // must be zero
	ACTIVE,
	DELETED
} Status;

// memory chunk table entry
typedef struct {
	usize alloc_no;	// entry sequential id
	void *addr;			// block start address
	usize size;		// block size (in bytes)
	Status status;		// entry status
} MemChunk;

// implement a linear probing hashtable
// with memory addresses as keys
typedef struct {
	usize count;       //
	usize nact;		// nb of active entries
	usize ndel;		// nb of deleted entries
	usize cap;			// table capacity
	usize total;		// total memory size (in bytes) accounted for
	MemChunk *data;		// memory chunks table
} MemTable;

static const usize MIN_CAP = 128;
static const double GROW_BY = 1.62;
static const double MAX_LOAD = 0.66;
static const double MIN_LOAD = 0.25;


static MemTable tally = {.count = 0, .nact = 0, .ndel = 0, .cap = 0, .data = NULL};

static void memtable_init(MemTable *tally)
{
	if (tally->cap) return;
	tally->count = 0;
	tally->nact = 0;
	tally->ndel = 0;
	tally->cap = MIN_CAP;
	tally->data = calloc(tally->cap, sizeof(MemChunk));
	tally->total = 0;
}


static usize hash(const void *addr, usize capacity)
{
	usize h = (usize)(addr);
	h *= 11400714819323198485llu;
	return h % capacity;
}

/* Ok but unused
static MemChunk memtable_get(MemTable *tally, void *addr)
{
	memtable_init(tally);
	usize pos = hash(addr, tally->cap);
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

static void memtable_check_and_resize(MemTable *tally)
{
	double load = (double)( tally->nact + tally->ndel ) / (double)(tally->cap);
	usize newcap = tally->cap;
	if (load < MIN_LOAD) {
		newcap = tally->cap / GROW_BY;
		newcap = (newcap > MIN_CAP) ? newcap : MIN_CAP;
	}
	else if (load > MAX_LOAD) {
		newcap = tally->cap * GROW_BY;
	}
	if (newcap != tally->cap) {
		MemChunk *newdata = (MemChunk *)calloc(newcap, sizeof(MemChunk));
		for (usize i = 0; i < tally->cap; i++) {
			if (tally->data[i].status == ACTIVE) {
				usize pos = hash(tally->data[i].addr, newcap);
				for (; newdata[pos].status != FREE; pos = (pos + 1) % newcap);
				newdata[pos] = tally->data[i];
			}
		}
		free(tally->data);
		tally->data = newdata;
		tally->ndel = 0;
		tally->cap = newcap;
	}
}


static usize memtable_set(MemTable *tally, void *addr, usize size)
{
#ifndef MEM_DEBUG
	return 0;
#endif
	memtable_init(tally);
	usize pos = hash(addr, tally->cap);
	while (tally->data[pos].status != FREE) {
		if (tally->data[pos].status == ACTIVE &&
		        tally->data[pos].addr == addr) {
			tally->total += (size - tally->data[pos].size);
			tally->data[pos].size = size;
			return tally->data[pos].alloc_no;
		}
		pos = (pos + 1) % tally->cap;
	}
	usize ret = tally->count;
	tally->data[pos].alloc_no = tally->count;
	tally->count++;
	tally->data[pos].addr = addr;
	tally->data[pos].size = size;
	tally->data[pos].status = ACTIVE;
	tally->total += size;
	tally->nact++;
	memtable_check_and_resize(tally);
	return ret;
}


static void memtable_unset(MemTable *tally, void *addr)
{
#ifndef MEM_DEBUG
	return;
#endif
	memtable_init(tally);
	usize pos = hash(addr, tally->cap);
	while (tally->data[pos].status != FREE) {
		if (tally->data[pos].status == ACTIVE &&
		        tally->data[pos].addr == addr) {
			tally->data[pos].status = DELETED;
			tally->ndel++;
			tally->nact--;
			tally->total -= tally->data[pos].size;
			break;
		}
		pos = (pos + 1) % tally->cap;
	}
	memtable_check_and_resize(tally);
}

static MemDbgQuery memtable_get(MemTable *tally, const void *addr)
{
#ifndef MEM_DEBUG
	return (MemDbgQuery) {
		.active = false, .size = 0
	};
#endif
	usize pos = hash(addr, tally->cap);
	while (tally->data[pos].status != FREE) {
		if (tally->data[pos].status == ACTIVE &&
		        tally->data[pos].addr == addr) {
			return (MemDbgQuery) {
				.active = true, .size = tally->data[pos].size
			};
		}
		pos = (pos + 1) % tally->cap;
	}
	assert(tally->data[pos].status == FREE);
	return (MemDbgQuery) {
		.active = false, .size = 0
	};
}



static const char *prefixes[7] = {"", "Kilo", "Mega", "Giga", "Tera", "Peta", "Exa"};


typedef struct {
	double size;
	const char *prefix;
} hr_t;

static hr_t human_readable(usize size)
{
	double total = (double) size;
	usize i = 0;
	while (i < 6 && size >> 10) {
		size >>= 10;
		i++;
	}
	hr_t ret = {.size = total / (double)(1 << (i * 10)), .prefix = prefixes[i]};
	return ret;
}


typedef struct {
	usize no, pos;
} pair_t;

static int cmp_pair(const void *l, const void *r)
{
	if ( ((pair_t *)l)->no == ((pair_t *)r)->no )
		return 0;
	else if ( ((pair_t *)l)->no < ((pair_t *)r)->no )
		return -1;
	else
		return +1;
}

static void memtable_print_stats(FILE *stream, MemTable *tally,
                                 bool print_chunks)
{
	fprintf(stream,
	        "================================================================================\n");
	fprintf(stream, "Heap memory info\n");
	if (print_chunks) {
		fprintf(stream,
		        "--------------------------------------------------------------------------------\n");
		fprintf(stream, "Chunks in chronological order of allocation\n\n");
		usize n = tally->nact, k = 0;
		pair_t *pairs = (pair_t *)malloc(n * sizeof(pair_t));
		for (usize i = 0; i < tally->cap; i++) {
			if (tally->data[i].status == ACTIVE) {
				pairs[k].no = tally->data[i].alloc_no;
				pairs[k].pos = i;
				k++;
			}
		}
		assert(k == n);
		qsort(pairs, n, sizeof(pair_t), cmp_pair);
		for (usize i = 0; i < n; i++) {
			usize pos = pairs[i].pos;
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
	tally.count = 0;
	tally.cap = 0;
	tally.nact = 0;
	tally.total = 0;
	tally.ndel = 0;
	free(tally.data);
	tally.data = NULL;
}


usize memdbg_total()
{
	return tally.total;
}


usize memdbg_nchunks()
{
	return tally.nact;
}


bool memdbg_is_empty()
{
	return (tally.total == 0 && tally.nact == 0);
}


MemDbgQuery memdbg_query(const void *addr)
{
	return memtable_get(&tally, addr);
}


void *memdbg_malloc(usize size, char *file, int line)
{
	void *ret = malloc(size);
#ifdef MEM_DEBUG_PRINT_ALL
	usize alloc_no = memtable_set(&tally, ret, size);
	hr_t hrsize = human_readable(tally.total);
	printf("malloc #%zu [%s:%d]  %zu bytes @%p (total: %.3lf %sbytes)\n",
	       alloc_no, file, line, size, ret, hrsize.size, hrsize.prefix);
#else
	memtable_set(&tally, ret, size);
#endif
	return ret;
}


void *memdbg_calloc(usize nmemb, usize size, char *file, int line)
{
	void *ret = calloc(nmemb, size);
#ifdef MEM_DEBUG_PRINT_ALL
	usize alloc_no = memtable_set(&tally, ret, nmemb * size);
	hr_t hrsize = human_readable(tally.total);
	printf("calloc #%zu [%s:%d]  %zu bytes @%p (total: %.3lf %sbytes)\n",
	       alloc_no, file, line, nmemb * size, ret, hrsize.size, hrsize.prefix);
#else
	memtable_set(&tally, ret, nmemb * size);
#endif
	return ret;
}


void *memdbg_realloc(void *ptr, usize size, char *file, int line)
{
	void *ret = realloc(ptr, size);
	if (ret != ptr) {
		memtable_unset(&tally, ptr);
	}
#ifdef MEM_DEBUG_PRINT_ALL
	usize alloc_no = memtable_set(&tally, ret, size);
	hr_t hrsize = human_readable(tally.total);
	printf("realloc #%zu [%s:%d]  %zu bytes @%p (total: %.3lf %sbytes)\n",
	       alloc_no, file, line, size, ret, hrsize.size, hrsize.prefix);
#else
	memtable_set(&tally, ret, size);
#endif
	return ret;
}


void memdbg_free(void *ptr, char *file, int line)
{
	MemDbgQuery q = memtable_get(&tally, ptr);
	ERROR_ASSERT(q.active == true,
	             "ERROR: invalid or double free detected @%p [%s:%d]\n",
	             ptr, file, line);
	free(ptr);
#ifdef MEM_DEBUG_PRINT_ALL
	hr_t hrsize = human_readable(tally.total);
	printf("free [%s:%d] @%p (total: %.3lf %sbytes)\n",
	       file, line, ptr, hrsize.size, hrsize.prefix);
#endif
	memtable_unset(&tally, ptr);
}

#undef MEM_DEBUG_OFF
