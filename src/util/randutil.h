#ifndef RANDUTIL_H
#define RANDUTIL_H

/**
 * Shuffle in-place the elements of array @p arr containing @p n elements 
 * of size @p typesize
 */
void shuffle_arr(void *arr, size_t n, size_t typesize);


#define RAND_RANGE_DECL( TYPE )\
TYPE rand_range_##TYPE(TYPE l, TYPE r);

RAND_RANGE_DECL(int);
RAND_RANGE_DECL(size_t);

#endif