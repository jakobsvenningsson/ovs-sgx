/*
 * heap.h
 *
 *  Created on: Apr 24, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_LIB_HEAP_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_LIB_HEAP_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* A heap node, to be embedded inside the data structure in the heap. */
struct heap_node {
    size_t idx;
    uint32_t priority;
};

/* A max-heap. */
struct heap {
    struct heap_node **array;   /* Data in elements 1...n, element 0 unused. */
    size_t n;                   /* Number of nodes currently in the heap. */
    size_t allocated;           /* Max 'n' before 'array' must be enlarged. */
};

#define HEAP_FOR_EACH(NODE, MEMBER, HEAP)                           \
    for (((HEAP)->n > 0                                             \
          ? ASSIGN_CONTAINER(NODE, (HEAP)->array[1], MEMBER)        \
          : ((NODE) = NULL, 1));                                    \
         (NODE) != NULL;                                            \
         ((NODE)->MEMBER.idx < (HEAP)->n                            \
          ? ASSIGN_CONTAINER(NODE,                                  \
                             (HEAP)->array[(NODE)->MEMBER.idx + 1], \
                             MEMBER)                                \
          : ((NODE) = NULL, 1)))


/* Returns the index of the node that is the parent of the node with the given
 * 'idx' within a heap. */
static inline size_t
heap_parent__(size_t idx)
{
    return idx / 2;
}
/* Returns the index of the node that is the left child of the node with the
 * given 'idx' within a heap. */
static inline size_t
heap_left__(size_t idx)
{
    return idx * 2;
}

/* Returns the index of the node that is the right child of the node with the
 * given 'idx' within a heap. */
static inline size_t
heap_right__(size_t idx)
{
    return idx * 2 + 1;
}

/* Returns true if 'idx' is the index of a leaf node in 'heap', false
 * otherwise. */
static inline bool
heap_is_leaf__(const struct heap *heap, size_t idx)
{
    return heap_left__(idx) > heap->n;
}
/* Returns the number of elements in 'heap'. */
static inline size_t
heap_count(const struct heap *heap)
{
    return heap->n;
}

static inline struct heap_node *
heap_pop(struct heap *heap)
{
    return heap->array[heap->n--];
}


/* Insertion and deletion. */
void heap_init_ovs(struct heap *);
void heap_insert(struct heap *, struct heap_node *, uint32_t priority);
void heap_remove(struct heap *, struct heap_node *);
void heap_destroy(struct heap *);
void heap_raw_remove(struct heap *, struct heap_node *);
void heap_change(struct heap *, struct heap_node *, uint32_t priority);
static inline void heap_raw_change(struct heap_node *, uint32_t priority);
void heap_raw_insert(struct heap *, struct heap_node *, uint32_t priority);
static inline size_t heap_count(const struct heap *);
static inline bool heap_is_empty(const struct heap *);
/* Returns true if 'heap' is empty, false if it contains at least one
 * element. */
static inline bool
heap_is_empty(const struct heap *heap)
{
    return heap->n == 0;
}


static inline void
heap_raw_change(struct heap_node *node, uint32_t priority)
{
    node->priority = priority;
}


#endif /* ENCLAVE_MYENCLAVE_TRUSTED_LIB_HEAP_H_ */
