#include "shared-memory-trusted.h"

int
shared_memory_index_of_allocated_page(shared_memory *shared_memory, void *page_addr) {
    struct page *page;
    for(int i = 0; i < shared_memory->cap; ++i) {
        if(!shared_memory->allocated[i]) {
            continue;
        }
        page = shared_memory->pages[i];
        if(page->bytes == page_addr) {
            return i;
        }
    }
    return -1;
}

void
shared_memory_mark_page_for_deallocation(shared_memory *shared_memory, void *page_addr) {
    printf("marking page at addr %p for deallocation.\n", page_addr);
    int page_index;
    page_index = shared_memory_index_of_allocated_page(shared_memory, page_addr);
    if(page_index == -1) {
        printf("Error, page address does not point towards any pages in shared memory.\n");
        return;
    }
    printf("marking page %zu for deallocation.\n", page_index);
    shared_memory->pages[page_index]->pending_deallocation = 1;
}

void
shared_memory_free_page(shared_memory *shared_memory, void *page_addr) {
    int page_index;
    page_index = shared_memory_index_of_allocated_page(shared_memory, page_addr);
    if(page_index == -1) {
        printf("Error, page address does not point towards any pages in shared memory.\n");
        return;
    }
    printf("Freeing page %zu.\n", page_index);
    shared_memory->pages[page_index]->status = PAGE_STATUS_FREE;
}

struct page *
shared_memory_get_page(shared_memory *shared_memory, size_t requested_size, uint8_t page_status) {
    int page_n = -1;
    // Try to find a already allocated page which is big enough and free.
    struct page *page;
    for(size_t i = 0; i < shared_memory->cap; ++i) {
        if(!shared_memory->allocated[i]) {
            continue;
        }
        page = shared_memory->pages[i];
        if(requested_size <= page->size && page->status == PAGE_STATUS_FREE) {
            page_n = i;
        }
    }

    // If we did not find a already allocated page in the previous loop, try to allocate a new one.
    if(page_n == -1) {
        ocall_allocate_page(requested_size, shared_memory, &page_n);
        if(page_n == -1) {
            printf("Failed to allocate page...\n");
            return NULL;
        }
    }
    shared_memory->pages[page_n]->status = page_status;

    return shared_memory->pages[page_n];
}
