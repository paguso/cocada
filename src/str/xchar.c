#include <byteswap.h>
#include <stdint.h>

#include "xchar.h"

void xchar_flip_bytes(xchar_t *c) {
    #if XCHAR_BYTESIZE==2
        *c = bswap_16(*c);
    #elif XCHAR_BYTESIZE==4
        *c = bswap_32(*c);
    #elif XCHAR_BYTESIZE==8
        *c = bswap_64(*c);
    #endif
}