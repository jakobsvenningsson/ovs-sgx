#include "enclave_t.h"
#include "enclave.h"
#include "stats.h"
#include "oftable.h"



// Global data structures
extern struct oftable * SGX_oftables[100];
extern struct SGX_table_dpif * SGX_table_dpif[100];
extern int SGX_n_tables[100];
extern struct sgx_cls_table * SGX_hmap_table[100];


// Vanilla ECALLS

size_t
ecall_flow_stats_c(uint8_t bridge_id){
    return ecall_flow_stats_r(bridge_id, NULL, -1);
}

// 51.2 REQUEST
size_t
ecall_flow_stats_r(uint8_t bridge_id, struct cls_rule ** buf, int elem){
    size_t p = 0;
    bool count_only = buf == NULL ? true : false;

    int i;
    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        struct sgx_cls_rule * rule;
        struct cls_cursor cursor;
        cls_cursor_init(&cursor, &SGX_oftables[bridge_id][i].cls, NULL);
        CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
            if(count_only) {
                p++;
                continue;
            }

            if (p > elem) {
                return p;
            }
            buf[p] = rule->o_cls_rule;
            p++;
        }
    }
    return p;
}

// Optimized ECALLS

size_t
ecall_collect_rules_loose_stats_request(uint8_t bridge_id,
                                      uint8_t table_id,
                                      int n_tables,
                                      size_t start_index,
                                      size_t buffer_size,
                                      struct match *match,
                                      struct cls_rule **cls_rules,
                                      struct match *matches,
                                      unsigned int *priorities,
                                      size_t *n_rules)
{
    struct oftable * table;
    struct cls_rule cr;
    sgx_cls_rule_init_i(bridge_id, &cr, match, 0);

    size_t n = 0, count = 0;

    FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, n_tables){
        struct cls_cursor cursor;
        struct sgx_cls_rule * rule;

        cls_cursor_init(&cursor, &table->cls, &cr);
        CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
            unsigned int priority = ecall_cr_priority(bridge_id, rule->o_cls_rule);
            bool rule_is_hidden = priority > UINT16_MAX;
            if(rule_is_hidden) {
                continue;
            }
            if (n >= buffer_size || (count < start_index)) {
                count++;
                continue;
            }
            count++;

            cls_rules[n] = rule->o_cls_rule;
            priorities[n] = ecall_cr_priority(bridge_id, rule->o_cls_rule);
            ecall_minimatch_expand(bridge_id, rule->o_cls_rule, &matches[n]);
            n++;
        }
    }
    cls_rule_destroy(&cr);
    if(n_rules) {
        *n_rules = count;
    }
    return n;
}
