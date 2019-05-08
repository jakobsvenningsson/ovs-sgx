#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include "enclave.h"
#include "enclave_t.h" /* print_string */
#include <stdbool.h>
#include "classifier.h"
#include "ofproto-provider.h"

#include "call-table.h"
#include "hotcall-trusted.h"
#include "openflow-common.h"
#include <sgx_spinlock.h>
#include "oftable.h"

// Global data structures
struct oftable * SGX_oftables[100];
struct SGX_table_dpif * SGX_table_dpif[100];
int SGX_n_tables[100];
struct sgx_cls_table * SGX_hmap_table[100];

size_t ecall_async_test(int *x, size_t len) {
    size_t sum = 0;
    for(size_t i = 0; i < len; ++i) {
        sum += x[i];
    }
    return sum;
}
