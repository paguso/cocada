#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "order.h"

#define CMP_IMPL( TYPE )\
int cmp_##TYPE(const void *pl, const void *pr) {\
    TYPE l = *((TYPE *)pl);\
    TYPE r = *((TYPE *)pr);\
    if (l == r) return 0;\
    else if (l < r) return -1;\
    else return +1;}

#define EQ_IMPL( TYPE )\
int eq_##TYPE(const void *pl, const void *pr)\
    { return   *((TYPE *)pl) == *((TYPE *)pr); }


#define CMPEQ_IMPL( TYPE )\
CMP_IMPL( TYPE )\
EQ_IMPL( TYPE )

CMPEQ_IMPL(int)
CMPEQ_IMPL(long)
CMPEQ_IMPL(float)
CMPEQ_IMPL(double)
CMPEQ_IMPL(size_t)
CMPEQ_IMPL(char)
CMPEQ_IMPL(int8_t)
CMPEQ_IMPL(int16_t)
CMPEQ_IMPL(int32_t)
CMPEQ_IMPL(int64_t)
CMPEQ_IMPL(uint8_t)
CMPEQ_IMPL(uint16_t)
CMPEQ_IMPL(uint32_t)
CMPEQ_IMPL(uint64_t)
