#include "enclave-batch-allocator.h"
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "list.h"

#define BATCH_SIZE 512

void
batch_allocator_add_block(struct batch_allocator * ba) {
    //ba->bytes[ba->n_blocks] = malloc(ba->block_sz * BATCH_SIZE + ba->block_sz);
    //ba->free_list[ba->n_blocks] = ba->bytes[ba->n_blocks] + ba->block_sz * BATCH_SIZE;
    //memset(ba->free_list[ba->n_blocks], 1, ba->block_sz);

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
batch_allocator_get_block(struct batch_allocator * ba) {
    if(list_is_empty(&ba->free_list)) {
        batch_allocator_add_block(ba);
    }
    struct list *free_list_node;
    free_list_node = list_pop_front(&ba->free_list);
    struct bblock *b;
    b = CONTAINER_OF(free_list_node, struct bblock, list_node);

    //hmap_insert(&ba->allocated_blocks, &b->hmap_node, (uint32_t)(uintptr_t) b->ptr, NULL, 0);

    return b;

    /*for(size_t n = 0; n < ba->n_blocks; ++n) {
        for(size_t m = 0; m < BATCH_SIZE; ++m) {
            if(ba->free_list[n][m]) {
                ba->free_list[n][m] = 0;
                ba->n_allocated++;
                if(ba->n_allocated == ba->n_blocks * BATCH_SIZE) {
                    batch_allocator_add_block(ba);
                }
                return ba->bytes[n] + m * ba->block_sz;
            }
        }
    }
    return NULL;*/
}

void
batch_allocator_free_block(struct batch_allocator *ba, struct list *list_node) {
    //HMAP_FOR_EACH_WITH_HASH(b, hmap_node, (uint32_t)(uintptr_t) ptr, &ba->allocated_blocks) {
    list_insert(&ba->free_list, list_node);
    ba->n_allocated--;
        //hmap_remove(&ba->allocated_blocks, &b->hmap_node);
    //    break;
    //}


    /*for(size_t n = 0; n < ba->n_blocks; ++n) {
        for(size_t m = 0; m < BATCH_SIZE; ++m) {
            if((ba->bytes[n] + m * ba->block_sz) == block) {
                ba->free_list[n][m] = 1;
                ba->n_allocated--;
            }
        }
    }*/
    //struct block *b = CONTAINER_OF(block, struct block, ptr);
    //list_insert(&ba->free_list, &b->list_node);
}


void
batch_allocator_init(struct batch_allocator *ba, unsigned int block_sz) {
    ba->n_blocks = 0;
    ba->n_allocated = 0;
    ba->block_sz = block_sz;
    list_init(&ba->free_list);
    //hmap_init(&ba->allocated_blocks, NULL);
}
