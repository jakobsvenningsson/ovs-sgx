#include "enclave_t.h"
#include "enclave.h"
#include "dpif.h"

// Global data structures
/*
extern struct oftable * SGX_oftables[100];
extern struct SGX_table_dpif * SGX_table_dpif[100];
extern int SGX_n_tables[100];
extern struct sgx_cls_table * SGX_hmap_table[100];*/

size_t
ecall_dpif_destroy_r(uint8_t bridge_id, struct cls_rule ** buf, int elem){
    size_t p = 0;
    bool count_only = buf == NULL ? true : false;
    int i;
    for (i = 0; i < e_ctx.SGX_n_tables[bridge_id]; i++) {
        struct sgx_cls_rule * rule, * next_rule;
        struct cls_cursor cursor;
        cls_cursor_init(&cursor, &e_ctx.SGX_oftables[bridge_id][i].cls, NULL);
        CLS_CURSOR_FOR_EACH_SAFE(rule, next_rule, cls_rule, &cursor){
            if(count_only) {
                p++;
                continue;
            }
            if (p > elem) {
                // overflow: this needs to be handle.
                return p;
            }

            buf[p] = rule->o_cls_rule;
            p++;
        }
    }
    return p;
}

size_t
ecall_dpif_destroy_c(uint8_t bridge_id){
    return ecall_dpif_destroy_r(bridge_id, NULL, -1);
}
