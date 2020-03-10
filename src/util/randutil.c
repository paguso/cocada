#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>

#include "bitsandbytes.h"
#include "randutil.h"
#include "string.h"

static bool _randinit = false;

static inline void randinit()
{
	if (!_randinit) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		int usec = tv.tv_usec;
		srand48(usec);
		_randinit = true;
	}
}

void shuffle_arr(void *arr, size_t n, size_t typesize)
{
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

RAND_RANGE_IMPL(short);
RAND_RANGE_IMPL(int);
RAND_RANGE_IMPL(long);
RAND_RANGE_IMPL(size_t);
RAND_RANGE_IMPL(int8_t);
RAND_RANGE_IMPL(int16_t);
RAND_RANGE_IMPL(int32_t);
RAND_RANGE_IMPL(int64_t);
RAND_RANGE_IMPL(uint8_t);
RAND_RANGE_IMPL(uint16_t);
RAND_RANGE_IMPL(uint32_t);
RAND_RANGE_IMPL(uint64_t);