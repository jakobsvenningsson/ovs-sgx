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
#include "enclave-batch-allocator.h"


// Global data structures
struct oftable * SGX_oftables[100];
struct SGX_table_dpif * SGX_table_dpif[100];
int SGX_n_tables[100];
struct sgx_cls_table * SGX_hmap_table[100];
struct batch_allocator cr_ba;
struct batch_allocator evg_ba;


void
ecall_ofproto_init_tables(uint8_t bridge_id, int n_tables){
    struct oftable * table;

    SGX_n_tables[bridge_id] = n_tables;
    SGX_oftables[bridge_id] = xmalloc(n_tables * sizeof(struct oftable));
    OFPROTO_FOR_EACH_TABLE(table, SGX_oftables[bridge_id]){
        oftable_init(table);
    }
    sgx_table_cls_init(bridge_id);
    sgx_table_dpif_init(bridge_id, n_tables);

    batch_allocator_init(&cr_ba, sizeof(struct sgx_cls_rule));
    batch_allocator_add_block(&cr_ba);

    batch_allocator_init(&evg_ba, sizeof(struct eviction_group));
    batch_allocator_add_block(&evg_ba);
}


size_t ecall_async_test(int *x, size_t len) {
    size_t sum = 0;
    for(size_t i = 0; i < len; ++i) {
        sum += x[i];
    }
    return sum;
}
