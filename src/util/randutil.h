#ifndef RANDUTIL_H
#define RANDUTIL_H

#include <stddef.h>
#include <stdint.h>

/**
 * Shuffle in-place the elements of array @p arr containing @p n elements
 * of size @p typesize
 */
void shuffle_arr(void *arr, size_t n, size_t typesize);


#define RAND_RANGE_DECL( TYPE )\
TYPE rand_range_##TYPE(TYPE l, TYPE r);

RAND_RANGE_DECL(short);
RAND_RANGE_DECL(int);
RAND_RANGE_DECL(long);
RAND_RANGE_DECL(size_t);
RAND_RANGE_DECL(int8_t);
RAND_RANGE_DECL(int16_t);
RAND_RANGE_DECL(int32_t);
RAND_RANGE_DECL(int64_t);
RAND_RANGE_DECL(uint8_t);
RAND_RANGE_DECL(uint16_t);
RAND_RANGE_DECL(uint32_t);
RAND_RANGE_DECL(uint64_t);

#endif