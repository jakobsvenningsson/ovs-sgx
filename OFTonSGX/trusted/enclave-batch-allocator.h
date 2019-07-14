#ifndef _ENCLAVE_BATCH_ALLOCATOR_H
#define _ENCLAVE_BATCH_ALLOCATOR_H
#include <stdint.h>
#include "common.h"


struct bblock {
    struct list list_node;
    uint8_t *ptr;
};

struct batch_allocator {
    unsigned int block_sz;
    unsigned int n_blocks;
    unsigned int n_allocated;
    uint8_t *bytes[200];
    struct list free_list;
};


#define BATCH_SIZE 1024

static inline void
batch_allocator_add_block(struct batch_allocator * ba) {
    ba->bytes[ba->n_blocks] = malloc(ba->block_sz * BATCH_SIZE + sizeof(struct bblock) * BATCH_SIZE);
    if(ba->bytes[ba->n_blocks] == NULL) {
        //printf("Failed to malloc\n");
    }
    //memset(ba->bytes[ba->n_blocks], 0, ba->block_sz * BATCH_SIZE + sizeof(struct bblock) * BATCH_SIZE);

    struct bblock *b;
    for(size_t i = 0; i < BATCH_SIZE; ++i) {
        b = (struct bblock *) ((char *) ba->bytes[ba->n_blocks] + (ba->block_sz * BATCH_SIZE + i * sizeof(struct bblock)));
        b->ptr = ((char *) ba->bytes[ba->n_blocks] + i * ba->block_sz);
        list_push_front(&ba->free_list, &b->list_node);
    }

    ba->n_blocks++;
}

static inline struct bblock *
batch_allocator_get_block(struct batch_allocator * ba) {
    if(list_is_empty(&ba->free_list)) {
        batch_allocator_add_block(ba);
    }
    struct list *free_list_node = list_pop_front(&ba->free_list);
    return CONTAINER_OF(free_list_node, struct bblock, list_node);
}


static inline struct bblock *
batch_allocator_get_block1(struct batch_allocator * ba) {
    if(list_is_empty(&ba->free_list)) {
        batch_allocator_add_block(ba);
    }
    struct list *free_list_node = list_pop_front(&ba->free_list);
    return CONTAINER_OF(free_list_node, struct bblock, list_node);
}

/*
struct bblock *
batch_allocator_get_block(struct batch_allocator * ba);*/

static inline void
batch_allocator_free_block(struct batch_allocator *ba, struct list *list_node) {
    list_push_front(&ba->free_list, list_node);
}


static inline void
batch_allocator_init(struct batch_allocator *ba, unsigned int block_sz) {
    ba->n_blocks = 0;
    ba->n_allocated = 0;
    ba->block_sz = block_sz;
    list_init(&ba->free_list);
}



/*
void
batch_allocator_add_block(struct batch_allocator * ba);
struct bblock *
batch_allocator_get_block(struct batch_allocator * ba);
void
batch_allocator_free_block(struct batch_allocator * ba, struct list *list_node);
void
batch_allocator_init(struct batch_allocator *ba, unsigned int block_sz);
*/
/*type struct {
    unsigned int sz;
    uint8_t *bytes;
    struct list list_node;
} block;*/

/*ype struct {
    unsigned int block_sz;
    unsigned int n_blocks;
    uint8_t *batch_blocks[20];
    struct list free_list;
} block_supervisor;

type struct {
    unsigned int n_supervisors;
    struct block_supervisor block_supervisors[5];
} batch_allocator;*/



#endif
