#ifndef ORDER_H
#define ORDER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int (*cmp_func)(const void *, const void *);

typedef bool (*eq_func)(const void *, const void *);

#define CMP_DECL( TYPE )\
int cmp_##TYPE(const void *, const void *);

#define EQ_DECL( TYPE )\
bool eq_##TYPE(const void *, const void *);


#define CMPEQ_DECL( TYPE )\
CMP_DECL( TYPE )\
EQ_DECL( TYPE )

CMPEQ_DECL(int)
CMPEQ_DECL(long)
CMPEQ_DECL(float)
CMPEQ_DECL(double)
CMPEQ_DECL(size_t)
CMPEQ_DECL(char)
CMPEQ_DECL(int8_t)
CMPEQ_DECL(int16_t)
CMPEQ_DECL(int32_t)
CMPEQ_DECL(int64_t)
CMPEQ_DECL(uint8_t)
CMPEQ_DECL(uint16_t)
CMPEQ_DECL(uint32_t)
CMPEQ_DECL(uint64_t)




#endif