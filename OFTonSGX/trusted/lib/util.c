/*
 * util.c
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#include "util.h"
#include <stdlib.h>
#include <stdio.h>


void
ovs_assert_failure(const char *where, const char *function,
                   const char *condition)
{
    /* Prevent an infinite loop (or stack overflow) in case VLOG_ABORT happens
     * to trigger an assertion failure of its own. */
    static int reentry = 0;

    switch (reentry++) {
    case 0:
        /*VLOG_ABORT("%s: assertion %s failed in %s()",
                   where, condition, function);*/
        NOT_REACHED();

    case 1:
        /*fprintf(stderr, "%s: assertion %s failed in %s()",
                where, condition, function);*/
        abort();

    default:
        abort();
    }
}



void *
xcalloc(size_t count, size_t size)
{
    void *p = count && size ? calloc(count, size) : malloc(1);
    //COVERAGE_INC(util_xalloc);
    if (p == NULL) {
        abort();
    }
    return p;
}

void *
xzalloc(size_t size)
{
    return xcalloc(1, size);
}

void *
xmalloc(size_t size)
{
    void *p = malloc(size ? size : 1);
    //COVERAGE_INC(util_xalloc);
    if (p == NULL) {
        abort();
    }
    return p;
}

void *
xrealloc(void *p, size_t size)
{
    p = realloc(p, size ? size : 1);
    //COVERAGE_INC(util_xalloc);
    if (p == NULL) {
        abort();
    }
    return p;
}



void *
xmemdup(const void *p_, size_t size)
{
    void *p = xmalloc(size);
    memcpy(p, p_, size);
    return p;
}

char *
xmemdup0(const char *p_, size_t length)
{
    char *p = xmalloc(length + 1);
    memcpy(p, p_, length);
    p[length] = '\0';
    return p;
}


/* Returns the number of trailing 0-bits in 'n'.  Undefined if 'n' == 0. */
#if !defined(UINT_MAX) || !defined(UINT32_MAX)
#error "Someone screwed up the #includes."
#elif __GNUC__ >= 4 && UINT_MAX == UINT32_MAX
/* Defined inline in util.h. */
#else
static int
raw_ctz(uint32_t n)
{
    unsigned int k;
    int count = 31;

#define CTZ_STEP(X)                             \
    k = n << (X);                               \
    if (k) {                                    \
        count -= X;                             \
        n = k;                                  \
    }
    CTZ_STEP(16);
    CTZ_STEP(8);
    CTZ_STEP(4);
    CTZ_STEP(2);
    CTZ_STEP(1);
#undef CTZ_STEP

    return count;
}
#endif




/* Returns the number of 1-bits in 'x', between 0 and 32 inclusive. */
unsigned int
popcount(uint32_t x)
{
    /* In my testing, this implementation is over twice as fast as any other
     * portable implementation that I tried, including GCC 4.4
     * __builtin_popcount(), although nonportable asm("popcnt") was over 50%
     * faster. */
#define INIT1(X)                                \
    ((((X) & (1 << 0)) != 0) +                  \
     (((X) & (1 << 1)) != 0) +                  \
     (((X) & (1 << 2)) != 0) +                  \
     (((X) & (1 << 3)) != 0) +                  \
     (((X) & (1 << 4)) != 0) +                  \
     (((X) & (1 << 5)) != 0) +                  \
     (((X) & (1 << 6)) != 0) +                  \
     (((X) & (1 << 7)) != 0))
#define INIT2(X)   INIT1(X),  INIT1((X) +  1)
#define INIT4(X)   INIT2(X),  INIT2((X) +  2)
#define INIT8(X)   INIT4(X),  INIT4((X) +  4)
#define INIT16(X)  INIT8(X),  INIT8((X) +  8)
#define INIT32(X) INIT16(X), INIT16((X) + 16)
#define INIT64(X) INIT32(X), INIT32((X) + 32)

    static const uint8_t popcount8[256] = {
        INIT64(0), INIT64(64), INIT64(128), INIT64(192)
    };

    return (popcount8[x & 0xff] +
            popcount8[(x >> 8) & 0xff] +
            popcount8[(x >> 16) & 0xff] +
            popcount8[x >> 24]);
}

/* Zeros the 'n_bits' bits starting from bit 'dst_ofs' in 'dst'.  'dst' is
 * 'dst_len' bytes long.
 *
 * If you consider all of 'dst' to be a single unsigned integer in network byte
 * order, then bit N is the bit with value 2**N.  That is, bit 0 is the bit
 * with value 1 in dst[dst_len - 1], bit 1 is the bit with value 2, bit 2 is
 * the bit with value 4, ..., bit 8 is the bit with value 1 in dst[dst_len -
 * 2], and so on.
 *
 * Required invariant:
 *   dst_ofs + n_bits <= dst_len * 8
 */
void
bitwise_zero(void *dst_, unsigned int dst_len, unsigned dst_ofs,
             unsigned int n_bits)
{
    uint8_t *dst = dst_;

    if (!n_bits) {
        return;
    }

    dst += dst_len - (dst_ofs / 8 + 1);
    dst_ofs %= 8;

    if (dst_ofs) {
        unsigned int chunk = MIN(n_bits, 8 - dst_ofs);

        *dst &= ~(((1 << chunk) - 1) << dst_ofs);

        n_bits -= chunk;
        if (!n_bits) {
            return;
        }

        dst--;
    }

    while (n_bits >= 8) {
        *dst-- = 0;
        n_bits -= 8;
    }

    if (n_bits) {
        *dst &= ~((1 << n_bits) - 1);
    }
}
