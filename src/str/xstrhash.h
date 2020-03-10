#ifndef __XSTRHASH_H__
#define __XSTRHASH_H__

#include <stddef.h>
#include <stdint.h>

#include "alphabet.h"
#include "new.h"
#include "xstring.h"

typedef struct _xstrhash xstrhash;


/**
 * @param ab (no transfer) alphabet
 */
uint64_t xstrhash_new(const alphabet *ab);

void xstrhash_dispose(void *ptr, const dtor *dt);

uint64_t xstrhash_lex(const xstrhash *self, const xstring *s);

uint64_t xstrhash_roll_lex(const xstrhash *self, const xstring *s, uint64_t hash, xchar_t c);


#endif