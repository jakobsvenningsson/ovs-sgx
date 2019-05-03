#ifndef _H_SHARED_MEMORY_UNTRUSTED_
#define _H_SHARED_MEMORY_UNTRUSTED_

#include "common.h"
#include <stdint.h>
#include "enclave_u.h"

#define DEFAULT_PAGE_SIZE 64
#define MAX_N_PAGES 10


void
shared_memory_init(shared_memory *shared_memory);
int
shared_memory_allocate_page(shared_memory *shared_memory, size_t page_sz);
void
shared_memory_deallocate_marked_pages(shared_memory *shared_memory);

#endif
