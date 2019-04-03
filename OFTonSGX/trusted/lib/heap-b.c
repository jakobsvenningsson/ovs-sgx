/*
 * heap.c
 *
 *  Created on: May 7, 2018
 *      Author: jorge
 */

#include "heap.h"
#include <stdlib.h>

static void put_node(struct heap *, struct heap_node *, size_t i);
static bool float_up(struct heap *, size_t i);
static void swap_nodes(struct heap *, size_t i, size_t j);
static void float_down(struct heap *, size_t i);
static void float_up_or_down(struct heap *, size_t i);

void
heap_init_ovs(struct heap *heap)
{
    heap->array = NULL;
    heap->n = 0;
    heap->allocated = 0;
}

/* Frees memory owned internally by 'heap'.  The caller is responsible for
 * freeing 'heap' itself, if necessary. */
void
heap_destroy(struct heap *heap)
{
    if (heap) {
        free(heap->array);
    }
}


static void
put_node(struct heap *heap, struct heap_node *node, size_t i)
{
    heap->array[i] = node;
    node->idx = i;
}

static void
swap_nodes(struct heap *heap, size_t i, size_t j)
{
    struct heap_node *old_i = heap->array[i];
    struct heap_node *old_j = heap->array[j];

    put_node(heap, old_j, i);
    put_node(heap, old_i, j);
}



static bool
float_up(struct heap *heap, size_t i)
{
    bool moved = false;
    size_t parent;

    for (; i > 1; i = parent) {
        parent = heap_parent__(i);
        if (heap->array[parent]->priority >= heap->array[i]->priority) {
            break;
        }
        swap_nodes(heap, parent, i);
        moved = true;
    }
    return moved;
}

void
heap_raw_remove(struct heap *heap, struct heap_node *node)
{
    size_t i = node->idx;
    if (i < heap->n) {
        put_node(heap, heap->array[heap->n], i);
    }
    heap->n--;
}




void
heap_remove(struct heap *heap, struct heap_node *node)
{
    size_t i = node->idx;

    heap_raw_remove(heap, node);
    if (i <= heap->n) {
        float_up_or_down(heap, i);
    }
}



void
heap_change(struct heap *heap, struct heap_node *node, uint32_t priority)
{
    heap_raw_change(node, priority);
    float_up_or_down(heap, node->idx);
}


void
heap_raw_insert(struct heap *heap, struct heap_node *node, uint32_t priority)
{
    if (heap->n >= heap->allocated) {
        heap->allocated = heap->n == 0 ? 1 : 2 * heap->n;
        heap->array =realloc(heap->array,(heap->allocated + 1) * sizeof *heap->array);
        if(!heap->array){
        	abort();
        }
    }

    put_node(heap, node, ++heap->n);
    node->priority = priority;
}


/* Inserts 'node' into 'heap' with the specified 'priority'.
 *
 * This takes time O(lg n). */
void
heap_insert(struct heap *heap, struct heap_node *node, uint32_t priority)
{
    heap_raw_insert(heap, node, priority);
    float_up(heap, node->idx);
}

static void
float_down(struct heap *heap, size_t i)
{
    while (!heap_is_leaf__(heap, i)) {
        size_t left = heap_left__(i);
        size_t right = heap_right__(i);
        size_t max = i;

        if (heap->array[left]->priority > heap->array[max]->priority) {
            max = left;
        }
        if (right <= heap->n
            && heap->array[right]->priority > heap->array[max]->priority) {
            max = right;
        }
        if (max == i) {
            break;
        }

        swap_nodes(heap, max, i);
        i = max;
    }
}

static void
float_up_or_down(struct heap *heap, size_t i)
{
    if (!float_up(heap, i)) {
        float_down(heap, i);
    }
}
