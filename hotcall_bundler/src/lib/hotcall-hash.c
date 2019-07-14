/*
 * Copyright (c) 2008, 2009, 2010, 2012, 2013 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "hotcall-hash.h"
#include "hotcall-util.h"
#include <string.h>

/* Returns the hash of 'a', 'b', and 'c'. */
uint32_t
hcall_hash_3words(uint32_t a, uint32_t b, uint32_t c)
{
    return hcall_mhash_finish(hcall_mhash_add(hcall_mhash_add(hcall_mhash_add(a, 0), b), c), 12);
}


uint32_t
hcall_hash_bytes(const void *p_, size_t n, uint32_t basis)
{
    const uint8_t *p = p_;
    size_t orig_n = n;
    uint32_t hash;

    hash = basis;
    while (n >= 4) {
        hash = hcall_mhash_add(hash, hcall_get_unaligned_u32((const uint32_t *) p));
        n -= 4;
        p += 4;
    }

    if (n) {
        uint32_t tmp = 0;

        memcpy(&tmp, p, n);
        hash = hcall_mhash_add__(hash, tmp);
    }

    return hcall_mhash_finish(hash, orig_n);
}

uint32_t
hcall_hash_double(double x, uint32_t basis)
{
    uint32_t value[2];

    memcpy(value, &x, sizeof value);
    return hcall_hash_3words(value[0], value[1], basis);
}
