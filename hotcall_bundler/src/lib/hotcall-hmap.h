/*
 * Copyright (c) 2008, 2009, 2010, 2012 Nicira, Inc.
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

#ifndef _HMAP_HOTCALL_TRUSTED_H
#define _HMAP_HOTCALL_TRUSTED_H 1

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "hotcall-util.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* A hash map node, to be embedded inside the data structure being mapped. */
struct hcall_hmap_node {
    size_t hash;                /* Hash value. */
    struct hcall_hmap_node *next;     /* Next in linked list. */
};

/* Returns the hash value embedded in 'node'. */
static inline size_t hcall_hmap_node_hash(const struct hcall_hmap_node *node)
{
    return node->hash;
}

#define HCALL_HMAP_NODE_NULL ((struct hcall_hmap_node *) 1)
#define HCALL_HMAP_NODE_NULL_INITIALIZER { 0, HCALL_HMAP_NODE_NULL }

/* Returns true if 'node' has been set to null by hmap_node_nullify() and has
 * not been un-nullified by being inserted into an hmap. */
static inline bool
hcall_hmap_node_is_null(const struct hcall_hmap_node *node)
{
    return node->next == HCALL_HMAP_NODE_NULL;
}

/* Marks 'node' with a distinctive value that can be tested with
 * hmap_node_is_null().  */
static inline void
hcall_hmap_node_nullify(struct hcall_hmap_node *node)
{
    node->next = HCALL_HMAP_NODE_NULL;
}

/* A hash map. */
struct hcall_hmap {
    struct hcall_hmap_node **buckets; /* Must point to 'one' iff 'mask' == 0. */
    struct hcall_hmap_node *one;
    size_t mask;
    size_t n;
};

/* Initializer for an empty hash map. */
#define HCALL_HMAP_INITIALIZER(HMAP) { &(HMAP)->one, NULL, 0, 0 }

/* Initialization. */
void hcall_hmap_init(struct hcall_hmap *);
void hcall_hmap_destroy(struct hcall_hmap *);
void hcall_hmap_clear(struct hcall_hmap *);
void hcall_hmap_swap(struct hcall_hmap *a, struct hcall_hmap *b);
void hcall_hmap_moved(struct hcall_hmap *hmap);
static inline size_t hcall_hmap_count(const struct hcall_hmap *);
static inline bool hcall_hmap_is_empty(const struct hcall_hmap *);

/* Adjusting capacity. */
void hcall_hmap_expand(struct hcall_hmap *);
void hcall_hmap_shrink(struct hcall_hmap *);
void hcall_hmap_reserve(struct hcall_hmap *, size_t capacity);

/* Insertion and deletion. */
static inline void hcall_hmap_insert_fast(struct hcall_hmap *,
                                    struct hcall_hmap_node *, size_t hash);
static inline void hcall_hmap_insert(struct hcall_hmap *, struct hcall_hmap_node *, size_t hash);
static inline void hcall_hmap_remove(struct hcall_hmap *, struct hcall_hmap_node *);

void hcall_hmap_node_moved(struct hcall_hmap *, struct hcall_hmap_node *, struct hcall_hmap_node *);
static inline void hcall_hmap_replace(struct hcall_hmap *, const struct hcall_hmap_node *old,
                                struct hcall_hmap_node *new_node);


/* Search.
 *
 * HMAP_FOR_EACH_WITH_HASH iterates NODE over all of the nodes in HMAP that
 * have hash value equal to HASH.  HMAP_FOR_EACH_IN_BUCKET iterates NODE over
 * all of the nodes in HMAP that would fall in the same bucket as HASH.  MEMBER
 * must be the name of the 'struct hcall_hmap_node' member within NODE.
 *
 * These macros may be used interchangeably to search for a particular value in
 * an hmap, see, e.g. shash_find() for an example.  Usually, using
 * HMAP_FOR_EACH_WITH_HASH provides an optimization, because comparing a hash
 * value is usually cheaper than comparing an entire hash map key.  But for
 * simple hash map keys, it makes sense to use HMAP_FOR_EACH_IN_BUCKET because
 * it avoids doing two comparisons when a single simple comparison suffices.
 *
 * The loop should not change NODE to point to a different node or insert or
 * delete nodes in HMAP (unless it "break"s out of the loop to terminate
 * iteration).
 *
 * HASH is only evaluated once.
 */
#define HCALL_HMAP_FOR_EACH_WITH_HASH(NODE, MEMBER, HASH, HMAP)               \
    for (ASSIGN_CONTAINER(NODE, hcall_hmap_first_with_hash(HMAP, HASH), MEMBER); \
         &(NODE)->MEMBER != NULL;                                       \
         ASSIGN_CONTAINER(NODE, hcall_hmap_next_with_hash(&(NODE)->MEMBER),   \
                          MEMBER))
#define HCALL_HMAP_FOR_EACH_IN_BUCKET(NODE, MEMBER, HASH, HMAP)               \
    for (ASSIGN_CONTAINER(NODE, hcall_hmap_first_in_bucket(HMAP, HASH), MEMBER); \
         &(NODE)->MEMBER != NULL;                                       \
         ASSIGN_CONTAINER(NODE, hcall_hmap_next_in_bucket(&(NODE)->MEMBER), MEMBER))

static inline struct hcall_hmap_node *hcall_hmap_first_with_hash(const struct hcall_hmap *,
                                                     size_t hash);
static inline struct hcall_hmap_node *hcall_hmap_next_with_hash(const struct hcall_hmap_node *);
static inline struct hcall_hmap_node *hcall_hmap_first_in_bucket(const struct hcall_hmap *,
                                                     size_t hash);
static inline struct hcall_hmap_node *hcall_hmap_next_in_bucket(const struct hcall_hmap_node *);


/* Iteration. */

/* Iterates through every node in HMAP. */
#define HCALL_HMAP_FOR_EACH(NODE, MEMBER, HMAP)                               \
    for (ASSIGN_CONTAINER(NODE, hcall_hmap_first(HMAP), MEMBER);              \
         &(NODE)->MEMBER != NULL;                                       \
         ASSIGN_CONTAINER(NODE, hcall_hmap_next(HMAP, &(NODE)->MEMBER), MEMBER))

/* Safe when NODE may be freed (not needed when NODE may be removed from the
 * hash map but its members remain accessible and intact). */
#define HCALL_HMAP_FOR_EACH_SAFE(NODE, NEXT, MEMBER, HMAP)                    \
    for (ASSIGN_CONTAINER(NODE, hcall_hmap_first(HMAP), MEMBER);              \
         (&(NODE)->MEMBER != NULL                                       \
          ? ASSIGN_CONTAINER(NEXT, hcall_hmap_next(HMAP, &(NODE)->MEMBER), MEMBER) \
          : 0);                                                         \
         (NODE) = (NEXT))

/* Continues an iteration from just after NODE. */
#define HCALL_HMAP_FOR_EACH_CONTINUE(NODE, MEMBER, HMAP)                      \
    for (ASSIGN_CONTAINER(NODE, hcall_hmap_next(HMAP, &(NODE)->MEMBER), MEMBER); \
         &(NODE)->MEMBER != NULL;                                       \
         ASSIGN_CONTAINER(NODE, hcall_hmap_next(HMAP, &(NODE)->MEMBER), MEMBER))

static inline struct hcall_hmap_node *hcall_hmap_first(const struct hcall_hmap *);
static inline struct hcall_hmap_node *hcall_hmap_next(const struct hcall_hmap *,
                                          const struct hcall_hmap_node *);

struct hcall_hmap_node *hcall_hmap_at_position(const struct hcall_hmap *,
                                   uint32_t *bucket, uint32_t *offset);

/* Returns the number of nodes currently in 'hmap'. */
static inline size_t
hcall_hmap_count(const struct hcall_hmap *hmap)
{
    return hmap->n;
}

/* Returns the maximum number of nodes that 'hmap' may hold before it should be
 * rehashed. */
static inline size_t
hcall_hmap_capacity(const struct hcall_hmap *hmap)
{
    return hmap->mask * 2 + 1;
}

/* Returns true if 'hmap' currently contains no nodes,
 * false otherwise. */
static inline bool
hcall_hmap_is_empty(const struct hcall_hmap *hmap)
{
    return hmap->n == 0;
}

/* Inserts 'node', with the given 'hash', into 'hmap'.  'hmap' is never
 * expanded automatically. */
static inline void
hcall_hmap_insert_fast(struct hcall_hmap *hmap, struct hcall_hmap_node *node, size_t hash)
{
    struct hcall_hmap_node **bucket = &hmap->buckets[hash & hmap->mask];
    node->hash = hash;
    node->next = *bucket;
    *bucket = node;
    hmap->n++;
}

/* Inserts 'node', with the given 'hash', into 'hmap', and expands 'hmap' if
 * necessary to optimize search performance. */
static inline void
hcall_hmap_insert(struct hcall_hmap *hmap, struct hcall_hmap_node *node, size_t hash)
{
    hcall_hmap_insert_fast(hmap, node, hash);
    if (hmap->n / 2 > hmap->mask) {
        hcall_hmap_expand(hmap);
    }
}

/* Removes 'node' from 'hmap'.  Does not shrink the hash table; call
 * hmap_shrink() directly if desired. */
static inline void
hcall_hmap_remove(struct hcall_hmap *hmap, struct hcall_hmap_node *node)
{
    struct hcall_hmap_node **bucket = &hmap->buckets[node->hash & hmap->mask];
    while (*bucket != node) {
        bucket = &(*bucket)->next;
    }
    *bucket = node->next;
    hmap->n--;
}

/* Puts 'new_node' in the position in 'hmap' currently occupied by 'old_node'.
 * The 'new_node' must hash to the same value as 'old_node'.  The client is
 * responsible for ensuring that the replacement does not violate any
 * client-imposed invariants (e.g. uniqueness of keys within a map).
 *
 * Afterward, 'old_node' is not part of 'hmap', and the client is responsible
 * for freeing it (if this is desirable). */
static inline void
hcall_hmap_replace(struct hcall_hmap *hmap,
             const struct hcall_hmap_node *old_node, struct hcall_hmap_node *new_node)
{
    struct hcall_hmap_node **bucket = &hmap->buckets[old_node->hash & hmap->mask];
    while (*bucket != old_node) {
        bucket = &(*bucket)->next;
    }
    *bucket = new_node;
    new_node->hash = old_node->hash;
    new_node->next = old_node->next;
}

static inline struct hcall_hmap_node *
hcall_hmap_next_with_hash__(const struct hcall_hmap_node *node, size_t hash)
{
    while (node != NULL && node->hash != hash) {
        node = node->next;
    }
    return CONST_CAST(struct hcall_hmap_node *, node);
}

/* Returns the first node in 'hmap' with the given 'hash', or a null pointer if
 * no nodes have that hash value. */
static inline struct hcall_hmap_node *
hcall_hmap_first_with_hash(const struct hcall_hmap *hmap, size_t hash)
{
    return hcall_hmap_next_with_hash__(hmap->buckets[hash & hmap->mask], hash);
}

/* Returns the first node in 'hmap' in the bucket in which the given 'hash'
 * would land, or a null pointer if that bucket is empty. */
static inline struct hcall_hmap_node *
hcall_hmap_first_in_bucket(const struct hcall_hmap *hmap, size_t hash)
{
    return hmap->buckets[hash & hmap->mask];
}

/* Returns the next node in the same bucket as 'node', or a null pointer if
 * there are no more nodes in that bucket.
 *
 * If the hash map has been reallocated since 'node' was visited, some nodes
 * may be skipped; if new nodes with the same hash value have been added, they
 * will be skipped.  (Removing 'node' from the hash map does not prevent
 * calling this function, since node->next is preserved, although freeing
 * 'node' of course does.) */
static inline struct hcall_hmap_node *
hcall_hmap_next_in_bucket(const struct hcall_hmap_node *node)
{
    return node->next;
}

/* Returns the next node in the same hash map as 'node' with the same hash
 * value, or a null pointer if no more nodes have that hash value.
 *
 * If the hash map has been reallocated since 'node' was visited, some nodes
 * may be skipped; if new nodes with the same hash value have been added, they
 * will be skipped.  (Removing 'node' from the hash map does not prevent
 * calling this function, since node->next is preserved, although freeing
 * 'node' of course does.) */
static inline struct hcall_hmap_node *
hcall_hmap_next_with_hash(const struct hcall_hmap_node *node)
{
    return hcall_hmap_next_with_hash__(node->next, node->hash);
}

static inline struct hcall_hmap_node *
hcall_hmap_next__(const struct hcall_hmap *hmap, size_t start)
{
    size_t i;
    for (i = start; i <= hmap->mask; i++) {
        struct hcall_hmap_node *node = hmap->buckets[i];
        if (node) {
            return node;
        }
    }
    return NULL;
}

/* Returns the first node in 'hmap', in arbitrary order, or a null pointer if
 * 'hmap' is empty. */
static inline struct hcall_hmap_node *
hcall_hmap_first(const struct hcall_hmap *hmap)
{
    return hcall_hmap_next__(hmap, 0);
}

/* Returns the next node in 'hmap' following 'node', in arbitrary order, or a
 * null pointer if 'node' is the last node in 'hmap'.
 *
 * If the hash map has been reallocated since 'node' was visited, some nodes
 * may be skipped or visited twice.  (Removing 'node' from the hash map does
 * not prevent calling this function, since node->next is preserved, although
 * freeing 'node' of course does.) */
static inline struct hcall_hmap_node *
hcall_hmap_next(const struct hcall_hmap *hmap, const struct hcall_hmap_node *node)
{
    return (node->next
            ? node->next
            : hcall_hmap_next__(hmap, (node->hash & hmap->mask) + 1));
}


/* Returns true if 'node' is in 'hmap', false otherwise. */
static inline bool
hcall_hmap_contains(const struct hcall_hmap *hmap, const struct hcall_hmap_node *node)
{
    struct hcall_hmap_node *p;

    for (p = hcall_hmap_first_in_bucket(hmap, node->hash); p; p = p->next) {
        if (p == node) {
            return true;
        }
    }

    return false;
}

#ifdef  __cplusplus
}
#endif

#endif /* hmap.h */
