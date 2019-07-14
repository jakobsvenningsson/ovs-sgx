#include "enclave-batch-allocator.h"
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "list.h"

/*
#define BATCH_SIZE 1024

void
batch_allocator_add_block(struct batch_allocator * ba) {
    ba->bytes[ba->n_blocks] = malloc(ba->block_sz * BATCH_SIZE + sizeof(struct bblock) * BATCH_SIZE);
    if(ba->bytes[ba->n_blocks] == NULL) {
        printf("Failed to malloc\n");
    }

    struct bblock *b;
    for(size_t i = 0; i < BATCH_SIZE; ++i) {
        b = (struct bblock *) ((char *) ba->bytes[ba->n_blocks] + (ba->block_sz * BATCH_SIZE + i * sizeof(struct bblock)));
        b->ptr = ((char *) ba->bytes[ba->n_blocks] + i * ba->block_sz);
        list_insert(&ba->free_list, &b->list_node);
    }

    ba->n_blocks++;
}

struct bblock *


void
batch_allocator_free_block(struct batch_allocator *ba, struct list *list_node) {
    list_insert(&ba->free_list, list_node);
    ba->n_allocated--;
}


void
batch_allocator_init(struct batch_allocator *ba, unsigned int block_sz) {
    ba->n_blocks = 0;
    ba->n_allocated = 0;
    ba->block_sz = block_sz;
    list_init(&ba->free_list);
}
*/
