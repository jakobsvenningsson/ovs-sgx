#ifndef _TLS_ENCLAVE_BATCH_ALLOCATOR_H
#define _TLS_ENCLAVE_BATCH_ALLOCATOR_H
#include <stdint.h>
#include "common.h"

struct bblock {
    struct list list_node;
    //struct hmap_node hmap_node;

    uint8_t *ptr;
};

struct batch_allocator {
    unsigned int block_sz;
    unsigned int n_blocks;
    unsigned int n_allocated;

    uint8_t *bytes[200];
    //uint8_t *free_list[100];

    //struct hmap allocated_blocks;
    struct list free_list;
};

void
batch_allocator_add_block(struct batch_allocator * ba);
struct bblock *
batch_allocator_get_block(struct batch_allocator * ba);
void
batch_allocator_free_block(struct batch_allocator * ba, struct list *list_node);
void
batch_allocator_init(struct batch_allocator *ba, unsigned int block_sz);

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
