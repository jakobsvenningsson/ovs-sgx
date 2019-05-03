#ifndef _H_SHARED_MEMORY_TRUSTED_
#define _H_SHARED_MEMORY_TRUSTED_

#include "common.h"

struct page *
shared_memory_get_page(shared_memory *shared_memory, size_t requested_size, uint8_t page_status);
void
shared_memory_free_page(shared_memory *shared_memory, void *page_addr);
void
shared_memory_mark_page_for_deallocation(shared_memory *shared_memory, void *page_addr);

#endif
