/*
 * hash.h
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_LIB_HASH_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_LIB_HASH_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif


static inline uint32_t
hash_rot(uint32_t x, int k)
{
    return (x << k) | (x >> (32 - k));
}

uint32_t hash_words(const uint32_t data[], size_t n_words, uint32_t basis);
uint32_t hash_bytes(const void *, size_t n_bytes, uint32_t basis);

static inline uint32_t hash_int(uint32_t x, uint32_t basis);
static inline uint32_t hash_2words(uint32_t, uint32_t);
uint32_t hash_3words(uint32_t, uint32_t, uint32_t);

static inline uint32_t hash_boolean(bool x, uint32_t basis);
uint32_t hash_double(double, uint32_t basis);

static inline uint32_t hash_pointer(const void *, uint32_t basis);
static inline uint32_t hash_string(const char *, uint32_t basis);

/* Murmurhash by Austin Appleby,
 * from http://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp.
 *
 * The upstream license there says:
 *
 * // MurmurHash3 was written by Austin Appleby, and is placed in the public
 * // domain. The author hereby disclaims copyright to this source code.
 *
 * See hash_words() for sample usage. */

static inline uint32_t mhash_add__(uint32_t hash, uint32_t data)
{
    data *= 0xcc9e2d51;
    data = hash_rot(data, 15);
    data *= 0x1b873593;
    return hash ^ data;
}

static inline uint32_t mhash_add(uint32_t hash, uint32_t data)
{
    hash = mhash_add__(hash, data);
    hash = hash_rot(hash, 13);
    return hash * 5 + 0xe6546b64;
}

static inline uint32_t mhash_finish(uint32_t hash, size_t n_bytes)
{
    hash ^= n_bytes;
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;
    return hash;
}

static inline uint32_t hash_int(uint32_t x, uint32_t basis)
{
    return hash_2words(x, basis);
}

static inline uint32_t hash_2words(uint32_t x, uint32_t y)
{
    return mhash_finish(mhash_add(mhash_add(x, 0), y), 4);
}

static inline uint32_t hash_pointer(const void *p, uint32_t basis)
{
    /* Often pointers are hashed simply by casting to integer type, but that
     * has pitfalls since the lower bits of a pointer are often all 0 for
     * alignment reasons.  It's hard to guess where the entropy really is, so
     * we give up here and just use a high-quality hash function.
     *
     * The double cast suppresses a warning on 64-bit systems about casting to
     * an integer to different size.  That's OK in this case, since most of the
     * entropy in the pointer is almost certainly in the lower 32 bits. */
    return hash_int((uint32_t) (uintptr_t) p, basis);
}


#endif /* ENCLAVE_MYENCLAVE_TRUSTED_LIB_HASH_H_ */
