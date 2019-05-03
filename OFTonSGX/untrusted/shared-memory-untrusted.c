#include "shared-memory-untrusted.h"

int
next_unallocated_page(shared_memory *shared_memory);
int
next_free_page(shared_memory *shared_memory);


int
shared_memory_allocate_page(shared_memory *shared_memory, size_t page_sz) {
    int page_n = -1;
    // try to find a page which is not allocated
    page_n = next_unallocated_page(shared_memory);
    if(page_n == -1) {
        page_n = next_free_page(shared_memory);
        if(page_n == -1) {
            return -1;
        }
        free(shared_memory->pages[page_n]->bytes);
    }

    shared_memory->allocated[page_n] = 1;

    struct page *page = malloc(sizeof(struct page));
    page->size = page_sz ? 2 * page_sz : shared_memory->default_page_sz;
    page->bytes = malloc(page->size);
    page->pending_deallocation = 0;

    shared_memory->pages[page_n] = page;

    return page_n;
}

void
shared_memory_init(shared_memory *shared_memory) {
    shared_memory->cap = MAX_N_PAGES;
    shared_memory->allocated = calloc(shared_memory->cap, sizeof(uint8_t));
    shared_memory->default_page_sz = DEFAULT_PAGE_SIZE;
    shared_memory->pages = (struct page **) malloc(shared_memory->cap * sizeof(struct page *));

    int res;
    for(size_t i = 0; i < MAX_N_PAGES/2; ++i) {
        res = shared_memory_allocate_page(shared_memory, 0);
        if(res == -1) {
            printf("Init: Failed to allocate page.\n");
            exit(-1);
        }
    }
}

void
shared_memory_deallocate_marked_pages(shared_memory *shared_memory) {
    struct page *page;
    for(size_t i = 0; i < shared_memory->cap; ++i) {
        if(!shared_memory->allocated[i]) {
            continue;
        }
        page = shared_memory->pages[i];
        if(page->pending_deallocation) {
            shared_memory->allocated[i] = 0;
            free(page->bytes);
            free(page);
        }
    }
}

// Helper functions

int
next_unallocated_page(shared_memory *shared_memory) {
    for(int i = 0; i < shared_memory->cap; ++i) {
        if(!shared_memory->allocated[i]) {
            return i;
        }
    }
    return -1;
}

int
next_free_page(shared_memory *shared_memory) {
    for(int i = 0; i < shared_memory->cap; ++i) {
        if(shared_memory->allocated[i] && shared_memory->pages[i]->status == PAGE_STATUS_FREE) {
            return i;
        }
    }
    return -1;
}
