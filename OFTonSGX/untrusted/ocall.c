#include "ocall.h"
#include <stdio.h>      /* vsnprintf */
#include <unistd.h>
#include "cache-untrusted.h"

void ocall_print(const char *str) {
    printf("%s", str);
}

void
ocall_allocate_page(size_t sz, shared_memory *shared_memory, size_t *page_idx) {
    *page_idx = allocate_page(shared_memory, sz);
}
