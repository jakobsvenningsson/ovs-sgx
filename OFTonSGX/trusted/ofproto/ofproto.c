#include "enclave_t.h"
#include "enclave.h"
#include "ofproto.h"
#include "oftable.h"


// Global data structures
extern struct oftable * SGX_oftables[100];
extern struct SGX_table_dpif * SGX_table_dpif[100];
extern int SGX_n_tables[100];
extern struct sgx_cls_table * SGX_hmap_table[100];

// Vanilla ECALLS

void
ecall_ofproto_destroy(uint8_t bridge_id){
    int i;

    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        sgx_oftable_destroy(bridge_id, i);
    }
    free(SGX_oftables[bridge_id]);
}

unsigned int
ecall_total_rules(uint8_t bridge_id){
    int i;
    unsigned int n_rules;

    n_rules = 0;
    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        n_rules += classifier_count(&SGX_oftables[bridge_id][i].cls);
    }
    return n_rules;
}

size_t
ecall_ofproto_get_vlan_c(uint8_t bridge_id){
    return ecall_ofproto_get_vlan_r(bridge_id, NULL, -1);
}

size_t
ecall_ofproto_get_vlan_r(uint8_t bridge_id, uint16_t * buf, int elem){
    struct oftable * oftable;
    bool count_only = buf == NULL ? true : false;
    size_t p = 0;
    int i;
    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        const struct cls_table * table;

        HMAP_FOR_EACH(table, hmap_node, &oftable->cls.tables){
            if (minimask_get_vid_mask(&table->mask) == VLAN_VID_MASK) {
                const struct cls_rule * rule;

                HMAP_FOR_EACH(rule, hmap_node, &table->rules){
                    if(count_only) {
                        p++;
                        continue;
                    }

                    uint16_t vid = miniflow_get_vid(&rule->match.flow);

                    if (p > elem) {
                        return p;
                    }
                    buf[p] = vid;
                    p++;
                }
            }
        }
    }
    return p;
}

size_t
ecall_collect_ofmonitor_util_c(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id, const struct minimatch * match){
    return ecall_collect_ofmonitor_util_r(bridge_id, ofproto_n_tables, NULL, -1, table_id, match);
}

size_t
ecall_collect_ofmonitor_util_r(uint8_t bridge_id, int ofproto_n_tables, struct cls_rule ** buf, int elem, uint8_t table_id,
  const struct minimatch * match){
    struct cls_rule target;
    const struct oftable * table;
    bool count_only = buf == NULL ? true : false;
    size_t p = 0;

    cls_rule_init_from_minimatch(&target, match, 0);
    FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, ofproto_n_tables){
        struct cls_cursor cursor;
        struct sgx_cls_rule * rule;

        cls_cursor_init(&cursor, &table->cls, &target);
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
    minimatch_destroy(&target.match); // to diassociate the target because if its a temp cls_rule
    return p;
}

size_t
ecall_ofproto_get_vlan_usage(uint8_t bridge_id,
                           size_t buf_size,
                           uint16_t *vlan_buffer,
                           size_t start_index,
                           size_t end_index,
                           size_t *n_vlan)
{
    struct oftable * oftable;
    size_t i = 0, n = 0;
    for (size_t i = 0; i < SGX_n_tables[bridge_id]; i++) {
        const struct cls_table * table;
        HMAP_FOR_EACH(table, hmap_node, &oftable->cls.tables){
            if (minimask_get_vid_mask(&table->mask) == VLAN_VID_MASK) {
                const struct cls_rule * rule;
                HMAP_FOR_EACH(rule, hmap_node, &table->rules){
                    bool buffer_is_full = n >= buf_size;
                    bool is_outside_range = i < start_index || (end_index != -1 && i >= end_index);
                    i++;
                    if(is_outside_range || buffer_is_full) {
                        continue;
                    }
                    vlan_buffer[n++] = miniflow_get_vid(&rule->match.flow);
                }
            }
        }
    }
    *n_vlan = i;
    return n;
}

size_t
ecall_ofproto_flush(uint8_t bridge_id,
                    struct cls_rule **cls_rules,
                    uint32_t *hashes,
                    size_t buf_size,
                    size_t start_index,
                    size_t end_index,
                    size_t *n_rules) {

    int count = 0, n = 0;
    for (size_t i = 0; i < SGX_n_tables[bridge_id]; i++) {
        struct sgx_cls_rule * rule, * next_rule;
        struct cls_cursor cursor;
        if (SGX_oftables[bridge_id][i].flags & OFTABLE_HIDDEN) {
            continue;
        }
        cls_cursor_init(&cursor, &SGX_oftables[bridge_id][i].cls, NULL);
        CLS_CURSOR_FOR_EACH_SAFE(rule, next_rule, cls_rule, &cursor){
            bool buffer_is_full = n >= buf_size;
            count++;
            if(buffer_is_full) {
                continue;
            }
            cls_rules[n] = rule->o_cls_rule;
            hashes[n++] = ecall_cls_rule_hash(bridge_id, rule->o_cls_rule, i);
            ecall_cls_remove(bridge_id, i, rule->o_cls_rule);
            ecall_evg_remove_rule(bridge_id, i, rule->o_cls_rule);
        }
    }
    *n_rules = count;
    return n;
}

void
ecall_ofproto_rule_send_removed(uint8_t bridge_id, struct cls_rule *cr, struct match *match, unsigned int *priority, bool *rule_is_hidden)
{
    unsigned int pr = ecall_cr_priority(bridge_id, cr);
    *rule_is_hidden = pr > UINT16_MAX;
    if (*rule_is_hidden) {
        return;
    }
    ecall_minimatch_expand(bridge_id, cr, match);
    *priority = pr;
}
