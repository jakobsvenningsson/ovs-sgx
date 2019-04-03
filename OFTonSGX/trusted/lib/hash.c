/*
 * hash.c
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#include "hash.h"
#include <string.h>

/* Returns the hash of 'a', 'b', and 'c'. */
uint32_t
hash_3words(uint32_t a, uint32_t b, uint32_t c)
{
    return mhash_finish(mhash_add(mhash_add(mhash_add(a, 0), b), c), 12);
}

/* Returns the hash of the 'n' 32-bit words at 'p', starting from 'basis'.
 * 'p' must be properly aligned. */
uint32_t
hash_words(const uint32_t p[], size_t n_words, uint32_t basis)
{
    uint32_t hash;
    size_t i;

    hash = basis;
    for (i = 0; i < n_words; i++) {
        hash = mhash_add(hash, p[i]);
    }
    return mhash_finish(hash, n_words * 4);
}

/* Returns the hash of the 'n' bytes at 'p', starting from 'basis'. */
uint32_t
hash_bytes(const void *p_, size_t n, uint32_t basis)
{
    const uint8_t *p = p_;
    size_t orig_n = n;
    uint32_t hash;

    hash = basis;
    while (n >= 4) {
        hash = mhash_add(hash, get_unaligned_u32((const uint32_t *) p));
        n -= 4;
        p += 4;
    }

    if (n) {
        uint32_t tmp = 0;

        memcpy(&tmp, p, n);
        hash = mhash_add__(hash, tmp);
    }

    return mhash_finish(hash, orig_n);
}
