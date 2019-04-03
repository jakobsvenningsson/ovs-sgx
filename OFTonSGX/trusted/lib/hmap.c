/*
 * hmap.c
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */
#include "hmap.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"

/* Initializes 'hmap' as an empty hash table. */
void
hmap_init(struct hmap *hmap)
{
    hmap->buckets = &hmap->one;
    hmap->one = NULL;
    hmap->mask = 0;
    hmap->n = 0;
}

/* Frees memory reserved by 'hmap'.  It is the client's responsibility to free
 * the nodes themselves, if necessary. */
void
hmap_destroy(struct hmap *hmap)
{
    if (hmap && hmap->buckets != &hmap->one) {
        free(hmap->buckets);
    }
}

/* Removes all node from 'hmap', leaving it ready to accept more nodes.  Does
 * not free memory allocated for 'hmap'.
 *
 * This function is appropriate when 'hmap' will soon have about as many
 * elements as it before.  If 'hmap' will likely have fewer elements than
 * before, use hmap_destroy() followed by hmap_clear() to save memory and
 * iteration time. */
void
hmap_clear(struct hmap *hmap)
{
    if (hmap->n > 0) {
        hmap->n = 0;
        memset(hmap->buckets, 0, (hmap->mask + 1) * sizeof *hmap->buckets);
    }
}

/* Exchanges hash maps 'a' and 'b'. */
void
hmap_swap(struct hmap *a, struct hmap *b)
{
    struct hmap tmp = *a;
    *a = *b;
    *b = tmp;
    hmap_moved(a);
    hmap_moved(b);
}

/* Adjusts 'hmap' to compensate for having moved position in memory (e.g. due
 * to realloc()). */
void
hmap_moved(struct hmap *hmap)
{
    if (!hmap->mask) {
        hmap->buckets = &hmap->one;
    }
}

static void
resize(struct hmap *hmap, size_t new_mask)
{
    struct hmap tmp;
    size_t i;

    ovs_assert(!(new_mask & (new_mask + 1)));
    ovs_assert(new_mask != SIZE_MAX);

    hmap_init(&tmp);
    if (new_mask) {
        tmp.buckets = xmalloc(sizeof *tmp.buckets * (new_mask + 1));
        tmp.mask = new_mask;
        for (i = 0; i <= tmp.mask; i++) {
            tmp.buckets[i] = NULL;
        }
    }
    for (i = 0; i <= hmap->mask; i++) {
        struct hmap_node *node, *next;
        int count = 0;
        for (node = hmap->buckets[i]; node; node = next) {
            next = node->next;
            hmap_insert_fast(&tmp, node, node->hash);
            count++;
        }
        if (count > 5) {
            //COVERAGE_INC(hmap_pathological);
        }
    }
    hmap_swap(hmap, &tmp);
    hmap_destroy(&tmp);
}



static size_t
calc_mask(size_t capacity)
{
    size_t mask = capacity / 2;
    mask |= mask >> 1;
    mask |= mask >> 2;
    mask |= mask >> 4;
    mask |= mask >> 8;
    mask |= mask >> 16;
#if SIZE_MAX > UINT32_MAX
    mask |= mask >> 32;
#endif

    /* If we need to dynamically allocate buckets we might as well allocate at
     * least 4 of them. */
    mask |= (mask & 1) << 1;

    return mask;
}
/* Expands 'hmap', if necessary, to optimize the performance of searches. */
void
hmap_expand(struct hmap *hmap)
{
    size_t new_mask = calc_mask(hmap->n);
    if (new_mask > hmap->mask) {
        //COVERAGE_INC(hmap_expand);
        resize(hmap, new_mask);
    }
}
