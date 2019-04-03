/*
 * flow.c
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#include "flow.h"
#include <stdlib.h>
#include <stdbool.h>
#include "packets.h"


//lines 641
/* Perform a bitwise OR of miniflow 'src' flow data with the equivalent
 * fields in 'dst', storing the result in 'dst'. */
static void
flow_union_with_miniflow(struct flow *dst, const struct miniflow *src)
{
    uint32_t *dst_u32 = (uint32_t *) dst;
    int ofs;
    int i;

    ofs = 0;
    for (i = 0; i < MINI_N_MAPS; i++) {
        uint32_t map;

        for (map = src->map[i]; map; map = zero_rightmost_1bit(map)) {
            dst_u32[raw_ctz(map) + i * 32] |= src->values[ofs++];
        }
    }
}

/* Fold minimask 'mask''s wildcard mask into 'wc's wildcard mask. */
void
flow_wildcards_fold_minimask(struct flow_wildcards *wc,
                             const struct minimask *mask)
{
    flow_union_with_miniflow(&wc->masks, &mask->masks);
}
//667




//1039
static int
miniflow_n_values(const struct miniflow *flow)
{
    int n, i;

    n = 0;
    for (i = 0; i < MINI_N_MAPS; i++) {
        n += popcount(flow->map[i]);
    }
    return n;
}

static uint32_t *
miniflow_alloc_values(struct miniflow *flow, int n)
{
    if (n <= MINI_N_INLINE) {
        return flow->inline_values;
    } else {
        //COVERAGE_INC(miniflow_malloc);
        return xmalloc(n * sizeof *flow->values);
    }
}

/* Initializes 'dst' as a copy of 'src'.  The caller must eventually free 'dst'
 * with miniflow_destroy(). */

void
miniflow_init(struct miniflow *dst, const struct flow *src)
{
    const uint32_t *src_u32 = (const uint32_t *) src;
    unsigned int ofs;
    unsigned int i;
    int n;

    /* Initialize dst->map, counting the number of nonzero elements. */
    n = 0;
    memset(dst->map, 0, sizeof dst->map);
    for (i = 0; i < FLOW_U32S; i++) {
        if (src_u32[i]) {
            dst->map[i / 32] |= 1u << (i % 32);
            n++;
        }
    }

    /* Initialize dst->values. */
    dst->values = miniflow_alloc_values(dst, n);
    ofs = 0;
    for (i = 0; i < MINI_N_MAPS; i++) {
        uint32_t map;

        for (map = dst->map[i]; map; map = zero_rightmost_1bit(map)) {
            dst->values[ofs++] = src_u32[raw_ctz(map) + i * 32];
        }
    }
}

/* Initializes 'dst' as a copy of 'src'.  The caller must eventually free 'dst'
 * with miniflow_destroy(). */
void
miniflow_clone(struct miniflow *dst, const struct miniflow *src)
{
    int n = miniflow_n_values(src);
    memcpy(dst->map, src->map, sizeof dst->map);
    dst->values = miniflow_alloc_values(dst, n);
    memcpy(dst->values, src->values, n * sizeof *dst->values);
}
//lines 1106



//lines 1107
/* Frees any memory owned by 'flow'.  Does not free the storage in which 'flow'
 * itself resides; the caller is responsible for that. */
void
miniflow_destroy(struct miniflow *flow)
{
    if (flow->values != flow->inline_values) {
        free(flow->values);
    }
}

//1116
/* Initializes 'dst' as a copy of 'src'. */
void
miniflow_expand(const struct miniflow *src, struct flow *dst)
{
    memset(dst, 0, sizeof *dst);
    flow_union_with_miniflow(dst, src);
}

//lines 1125
static const uint32_t *
miniflow_get__(const struct miniflow *flow, unsigned int u32_ofs)
{
    if (!(flow->map[u32_ofs / 32] & (1u << (u32_ofs % 32)))) {
        static const uint32_t zero = 0;
        return &zero;
    } else {
        const uint32_t *p = flow->values;

        BUILD_ASSERT(MINI_N_MAPS == 2);
        if (u32_ofs < 32) {
            p += popcount(flow->map[0] & ((1u << u32_ofs) - 1));
        } else {
            p += popcount(flow->map[0]);
            p += popcount(flow->map[1] & ((1u << (u32_ofs - 32)) - 1));
        }
        return p;
    }
}


/* Initializes 'dst' as a copy of 'src'. */
void
minimask_expand(const struct minimask *mask, struct flow_wildcards *wc)
{
    miniflow_expand(&mask->masks, &wc->masks);
}


/* Returns the uint32_t that would be at byte offset '4 * u32_ofs' if 'flow'
 * were expanded into a "struct flow". */
uint32_t
miniflow_get(const struct miniflow *flow, unsigned int u32_ofs)
{
    return *miniflow_get__(flow, u32_ofs);
}

//1151
static ovs_be16
miniflow_get_be16(const struct miniflow *flow, unsigned int u8_ofs)
{
    const uint32_t *u32p = miniflow_get__(flow, u8_ofs / 4);
    const ovs_be16 *be16p = (const ovs_be16 *) u32p;
    return be16p[u8_ofs % 4 != 0];
}

/* Returns the VID within the vlan_tci member of the "struct flow" represented
 * by 'flow'. */
uint16_t
miniflow_get_vid(const struct miniflow *flow)
{
    ovs_be16 tci = miniflow_get_be16(flow, offsetof(struct flow, vlan_tci));
    return vlan_tci_to_vid(tci);
}





//lines 1172
/* Returns true if 'a' and 'b' are the same flow, false otherwise.  */
bool
miniflow_equal(const struct miniflow *a, const struct miniflow *b)
{
    int i;

    for (i = 0; i < MINI_N_MAPS; i++) {
        if (a->map[i] != b->map[i]) {
            return false;
        }
    }
    return !memcmp(a->values, b->values,
                       miniflow_n_values(a) * sizeof *a->values);
}

/* Returns true if 'a' and 'b' are equal at the places where there are 1-bits
 * in 'mask', false if they differ. */
bool
miniflow_equal_in_minimask(const struct miniflow *a, const struct miniflow *b,
                           const struct minimask *mask)
{
    const uint32_t *p;
    int i;

    p = mask->masks.values;
    for (i = 0; i < MINI_N_MAPS; i++) {
        uint32_t map;

        for (map = mask->masks.map[i]; map; map = zero_rightmost_1bit(map)) {
            int ofs = raw_ctz(map) + i * 32;

            if ((miniflow_get(a, ofs) ^ miniflow_get(b, ofs)) & *p) {
                return false;
            }
            p++;
        }
    }

    return true;
}

/* Returns true if 'a' and 'b' are equal at the places where there are 1-bits
 * in 'mask', false if they differ. */
bool
miniflow_equal_flow_in_minimask(const struct miniflow *a, const struct flow *b,
                                const struct minimask *mask)
{
    const uint32_t *b_u32 = (const uint32_t *) b;
    const uint32_t *p;
    int i;

    p = mask->masks.values;
    for (i = 0; i < MINI_N_MAPS; i++) {
        uint32_t map;

        for (map = mask->masks.map[i]; map; map = zero_rightmost_1bit(map)) {
            int ofs = raw_ctz(map) + i * 32;

            if ((miniflow_get(a, ofs) ^ b_u32[ofs]) & *p) {
                return false;
            }
            p++;
        }
    }

    return true;
}


/* Returns a hash value for 'flow', given 'basis'. */
uint32_t
miniflow_hash(const struct miniflow *flow, uint32_t basis)
{
    BUILD_ASSERT_DECL(MINI_N_MAPS == 2);
    return hash_3words(flow->map[0], flow->map[1],
                       hash_words(flow->values, miniflow_n_values(flow),
                                  basis));
}

/* Returns a hash value for the bits of 'flow' where there are 1-bits in
 * 'mask', given 'basis'.
 *
 * The hash values returned by this function are the same as those returned by
 * flow_hash_in_minimask(), only the form of the arguments differ. */
uint32_t
miniflow_hash_in_minimask(const struct miniflow *flow,
                          const struct minimask *mask, uint32_t basis)
{
    const uint32_t *p = mask->masks.values;
    uint32_t hash;
    int i;

    hash = basis;
    for (i = 0; i < MINI_N_MAPS; i++) {
        uint32_t map;

        for (map = mask->masks.map[i]; map; map = zero_rightmost_1bit(map)) {
            int ofs = raw_ctz(map) + i * 32;

            hash = mhash_add(hash, miniflow_get(flow, ofs) & *p);
            p++;
        }
    }

    return mhash_finish(hash, (p - mask->masks.values) * 4);
}

/* Returns a hash value for the bits of 'flow' where there are 1-bits in
 * 'mask', given 'basis'.
 *
 * The hash values returned by this function are the same as those returned by
 * miniflow_hash_in_minimask(), only the form of the arguments differ. */
uint32_t
flow_hash_in_minimask(const struct flow *flow, const struct minimask *mask,
                      uint32_t basis)
{
    const uint32_t *flow_u32 = (const uint32_t *) flow;
    const uint32_t *p = mask->masks.values;
    uint32_t hash;
    int i;
    hash = basis;
    for (i = 0; i < MINI_N_MAPS; i++) {
        uint32_t map;

        for (map = mask->masks.map[i]; map; map = zero_rightmost_1bit(map)) {
            int ofs = raw_ctz(map) + i * 32;
            hash = mhash_add(hash, flow_u32[ofs] & *p);
            p++;
        }
    }

    return mhash_finish(hash, (p - mask->masks.values) * 4);
}

/* Initializes 'dst' as a copy of 'src'.  The caller must eventually free 'dst'
 * with minimask_destroy(). */
void
minimask_init(struct minimask *mask, const struct flow_wildcards *wc)
{
    miniflow_init(&mask->masks, &wc->masks);
}

/* Initializes 'dst' as a copy of 'src'.  The caller must eventually free 'dst'
 * with minimask_destroy(). */
void
minimask_clone(struct minimask *dst, const struct minimask *src)
{
    miniflow_clone(&dst->masks, &src->masks);
}

/* Initializes 'dst_' as the bit-wise "and" of 'a_' and 'b_'.
 *
 * The caller must provide room for FLOW_U32S "uint32_t"s in 'storage', for use
 * by 'dst_'.  The caller must *not* free 'dst_' with minimask_destroy(). */
void
minimask_combine(struct minimask *dst_,
                 const struct minimask *a_, const struct minimask *b_,
                 uint32_t storage[FLOW_U32S])
{
    struct miniflow *dst = &dst_->masks;
    const struct miniflow *a = &a_->masks;
    const struct miniflow *b = &b_->masks;
    int i, n;

    n = 0;
    dst->values = storage;
    for (i = 0; i < MINI_N_MAPS; i++) {
        uint32_t map;

        dst->map[i] = 0;
        for (map = a->map[i] & b->map[i]; map;
             map = zero_rightmost_1bit(map)) {
            int ofs = raw_ctz(map) + i * 32;
            uint32_t mask = miniflow_get(a, ofs) & miniflow_get(b, ofs);

            if (mask) {
                dst->map[i] |= rightmost_1bit(map);
                dst->values[n++] = mask;
            }
        }
    }
}

////1356

uint16_t
minimask_get_vid_mask(const struct minimask *mask)
{
    return miniflow_get_vid(&mask->masks);
}






//lines1388
    /* Returns true if 'a' and 'b' are the same flow mask, false otherwise.  */
    bool
    minimask_equal(const struct minimask *a, const struct minimask *b)
    {
        return miniflow_equal(&a->masks, &b->masks);
    }

/* Returns a hash value for 'mask', given 'basis'. */
uint32_t
minimask_hash(const struct minimask *mask, uint32_t basis)
{
        return miniflow_hash(&mask->masks, basis);
}

/* Returns true if at least one bit is wildcarded in 'a_' but not in 'b_',
 * false otherwise. */
bool
minimask_has_extra(const struct minimask *a_, const struct minimask *b_)
{
    const struct miniflow *a = &a_->masks;
    const struct miniflow *b = &b_->masks;
    int i;

    for (i = 0; i < MINI_N_MAPS; i++) {
        uint32_t map;

        for (map = a->map[i] | b->map[i]; map;
             map = zero_rightmost_1bit(map)) {
            int ofs = raw_ctz(map) + i * 32;
            uint32_t a_u32 = miniflow_get(a, ofs);
            uint32_t b_u32 = miniflow_get(b, ofs);

            if ((a_u32 & b_u32) != b_u32) {
                return true;
            }
        }
    }

    return false;
}
///line 1428


/* Frees any memory owned by 'mask'.  Does not free the storage in which 'mask'
 * itself resides; the caller is responsible for that. */
void
minimask_destroy(struct minimask *mask)
{
    miniflow_destroy(&mask->masks);
}

/* Returns true if 'mask' matches every packet, false if 'mask' fixes any bits
 * or fields. */
bool
minimask_is_catchall(const struct minimask *mask_)
{    const struct miniflow *mask = &mask_->masks;

    BUILD_ASSERT(MINI_N_MAPS == 2);

    return !(mask->map[0] | mask->map[1]);
}
