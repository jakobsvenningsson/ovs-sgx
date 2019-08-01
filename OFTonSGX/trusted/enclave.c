#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include "enclave.h"
#include "enclave_t.h" /* print_string */
#include <stdbool.h>
#include "classifier.h"
#include "ofproto-provider.h"

#include "call-table.h"
#include "openflow-common.h"
#include <sgx_spinlock.h>
#include "oftable.h"
#include "hotcall.h"
#include "hotcall-bundler-trusted.h"
#include "cache-trusted.h"
#include "functions.h"


bool hotcall_configured = false;
struct ovs_enclave_ctx e_ctx = { 0 };

void
ecall_plus_one(int *x) {
    ++*x;
}

void
ecall_minus_one(int *x) {
    --*x;
}

static void
wrapper_ecall_evg_add_rule(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_evg_add_rule(
            *(uint8_t *) args[0][i],
            *(uint8_t *) args[1][i],
            (struct cls_rule *) args[2][i],
            (uint32_t *) args[3][i],
            *(uint32_t *) args[4][i]
        );
    }
}

static void
wrapper_ecall_rule_eviction_priority(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[n_params - 1][i]  = rule_eviction_priority(args[0][i], *(uint32_t *) args[1][i]);
    }
}

static void
wrapper_ecall_cls_rule_hash(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[n_params - 1][i]  = ecall_cls_rule_hash(*(uint8_t *) args[0][i], args[1][i], *(uint32_t *) args[2][i]);
    }
}

static void
wrapper_ecall_cls_remove(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_cls_remove(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i]);
    }
}

static void
wrapper_ecall_evg_remove_rule(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_evg_remove_rule(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i]);
    }
}

static void
wrapper_ecall_cr_priority(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[n_params - 1][i] = ecall_cr_priority(*(uint8_t *) args[0][i], args[1][i]);
    }
}

static void
wrapper_ecall_oftable_is_other_table(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_oftable_is_other_table(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i]);
    }
}

static void
wrapper_ecall_oftable_update_taggable(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_oftable_update_taggable(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i]);
    }
}

static void
wrapper_ecall_oftable_get_flags(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_oftable_get_flags(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i]);
    }
}

static void
wrapper_ecall_is_eviction_fields_enabled(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_is_eviction_fields_enabled(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i]);
    }
}

static void
wrapper_ecall_oftable_cls_count(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_oftable_cls_count(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i]);
    }
}

static void
wrapper_ecall_oftable_mflows(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_oftable_mflows(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i]);
    }
}

static void
wrapper_ecall_oftable_is_readonly(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_oftable_is_readonly(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i]);
    }
}

static void
wrapper_ecall_oftable_set_readonly(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_oftable_set_readonly(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i]);
    }
}


static void
wrapper_ecall_cls_rule_init(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_cls_rule_init(*(uint8_t *) args[0][i], args[1][i], (const struct match *) args[2][i], *(unsigned int *) args[3][i]);
    }
}

static void
wrapper_ecall_oftable_classifier_replace(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(void **) args[n_params - 1][i] = ecall_oftable_classifier_replace(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], (struct cls_rule *) args[2][i]);
    }
}

static void
wrapper_ecall_miniflow_expand(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_miniflow_expand(*(uint8_t *) args[0][i], args[1][i], args[2][i]);
    }
}

static void
wrapper_ecall_rule_calculate_tag(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(uint32_t *) args[n_params - 1][i] = ecall_rule_calculate_tag(*(uint8_t *) args[0][i], args[1][i], args[2][i], *(uint8_t *) args[3][i]);
    }
}

static void
wrapper_ecall_minimask_get_vid_mask(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(uint16_t *) args[n_params - 1][i] = ecall_minimask_get_vid_mask(*(uint8_t *) args[0][i], args[1][i]);
    }
}

static inline uint32_t
cls_hash_trusted(const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id) {
    uint32_t hash;
    if(wc) {
        struct minimask m;
        minimask_init(&m, wc);
        hash = flow_hash_in_minimask(flow, &m, bridge_id + table_id);
    } else {
        hash = flow_hash(flow, bridge_id + table_id);
    }
    return hash;
}

static void
wrapper_ecall_oftable_cls_lookup(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(void **) args[n_params - 1][i] = ecall_oftable_cls_lookup(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i], args[3][i]);
        #ifdef MEMOIZE1
        *(uint32_t *) args[n_params - 2][i] = cls_hash_trusted(args[2][i], args[3][i], *(uint8_t *) args[0][i], *(uint8_t *) args[1][i]);
        #endif
    }
}

static void
wrapper_ecall_oftable_hidden_check(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_oftable_hidden_check(*(uint8_t *) args[0][i]);
    }
}

static void
wrapper_ecall_oftable_set_name(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_oftable_set_name(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i]);
    }
}

static void
wrapper_ecall_oftable_disable_eviction(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_oftable_disable_eviction(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i]);
    }
}

static void
wrapper_ecall_oftable_enable_eviction(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_oftable_enable_eviction(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i], *(size_t *) args[3][i], *(uint32_t *) args[4][i], args[5][i]);
    }
}

static void
wrapper_ecall_oftable_mflows_set(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_oftable_mflows_set(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], *(unsigned int *) args[2][i]);
    }
}

static void
wrapper_ecall_set_evictable(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_set_evictable(*(uint8_t *) args[0][i], args[1][i], *(bool *) args[2][i]);
    }
}

static void
wrapper_ecall_is_evictable(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(bool *) args[n_params - 1][i] = ecall_is_evictable(*(uint8_t *) args[0][i], args[1][i]);
    }
}

static void
wrapper_ecall_restore_evictable(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_restore_evictable(*(uint8_t *) args[0][i], args[1][i]);
    }
}

static void
wrapper_ecall_backup_evictable(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_backup_evictable(*(uint8_t *) args[0][i], args[1][i]);
    }
}

static void
wrapper_ecall_rule_update_used(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_rule_update_used(*(uint8_t *) args[0][i], args[1][i], *(uint32_t *) args[2][i]);
    }
}

static void
wrapper_ecall_backup_and_set_evictable(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_backup_and_set_evictable(*(uint8_t *) args[0][i], args[1][i], *(bool *) args[2][i]);
    }
}

static void
wrapper_ecall_flush_c(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_flush_c(*(uint8_t *) args[0][i]);
    }
}

static void
wrapper_ecall_flush_r(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_flush_r(*(uint8_t *) args[0][i], args[1][i], *(int *) args[2][i]);
    }
}

static void
wrapper_ecall_oftable_cls_find_match_exactly(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_oftable_cls_find_match_exactly(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i], *(unsigned int *) args[3][i], args[4][i]);
    }
}

static void
wrapper_ecall_total_rules(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_total_rules(*(int *) args[0][i]);
    }
}

static void
wrapper_ecall_collect_rules_loose_c(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_collect_rules_loose_c(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], *(uint8_t *) args[2][i], args[3][i]);
    }
}

static void
wrapper_ecall_collect_rules_loose_r(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_collect_rules_loose_r(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i], *(int *) args[3][i], *(uint8_t *) args[4][i], args[5][i]);
    }
}

static void
wrapper_ecall_cls_rule_destroy(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_cls_rule_destroy(*(int *) args[0][i], args[1][i]);
    }
}

static void
wrapper_ecall_oftable_enable_eviction_c(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[n_params - 1][i] = ecall_oftable_enable_eviction_c(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i]);
    }
}

static void
wrapper_ecall_oftable_enable_eviction_r(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[n_params - 1][i] = ecall_oftable_enable_eviction_r(*(uint8_t *) args[0][i], args[1][i], *(int *) args[2][i], *(uint8_t *) args[3][i], args[4][i]);
    }
}

static void
wrapper_ecall_choose_rule_to_evict(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_choose_rule_to_evict(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i]);
    }
}

static void
wrapper_ecall_choose_rule_to_evict_p(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_choose_rule_to_evict_p(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i], args[3][i]);
    }
}

static void
wrapper_ecall_cls_rule_equal(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_cls_rule_equal(*(uint8_t *) args[0][i], args[1][i], args[2][i]);
    }
}

static void
wrapper_ecall_collect_rules_strict_c(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_collect_rules_strict_c(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], *(uint8_t *) args[2][i], args[3][i], *(unsigned int *) args[4][i]);
    }
}

static void
wrapper_ecall_collect_rules_strict_r(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_collect_rules_strict_r(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i], *(int *) args[3][i], *(uint8_t *) args[4][i], args[5][i], *(unsigned int *) args[6][i]);
    }
}

static void
wrapper_ecall_collect_ofmonitor_util_c(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_collect_ofmonitor_util_c(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], *(uint8_t *) args[2][i], args[3][i]);
    }
}


static void
wrapper_ecall_collect_ofmonitor_util_r(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_collect_ofmonitor_util_r(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i], *(int *) args[3][i], *(uint8_t *) args[4][i], args[5][i]);
    }
}

static void
wrapper_ecall_cr_rule_overlaps(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_cr_rule_overlaps(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i]);
    }
}

static void
wrapper_ecall_minimatch_expand(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_minimatch_expand(*(uint8_t *) args[0][i], args[1][i], args[2][i]);
    }
}

static void
wrapper_ecall_ofproto_destroy(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_ofproto_destroy(*(uint8_t *) args[0][i]);
    }
}

static void
wrapper_ecall_oftable_name(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_oftable_name(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i], *(size_t *) args[3][i]);
    }
}

static void
wrapper_ecall_cls_rule_is_loose_match(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_cls_rule_is_loose_match(*(uint8_t *) args[0][i], args[1][i], args[2][i]);
    }
}

static void
wrapper_ecall_miniflow_get_vid(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(uint16_t *) args[n_params - 1][i] = ecall_miniflow_get_vid(*(uint8_t *) args[0][i], args[1][i]);
    }
}

static void
wrapper_ecall_flow_stats_c(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[n_params - 1][i] = ecall_flow_stats_c(*(uint8_t *) args[0][i]);
    }
}

static void
wrapper_ecall_flow_stats_r(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[n_params - 1][i] = ecall_flow_stats_r(*(uint8_t *) args[0][i], args[1][i], *(int *) args[2][i]);
    }
}

static void
wrapper_ecall_dpif_destroy_c(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[n_params - 1][i] = ecall_dpif_destroy_c(*(uint8_t *) args[0][i]);
    }
}

static void
wrapper_ecall_dpif_destroy_r(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[n_params - 1][i] = ecall_dpif_destroy_r(*(uint8_t *) args[0][i], args[1][i], *(int *) args[2][i]);
    }
}

static void
wrapper_ecall_cls_rule_format(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[n_params - 1][i] = ecall_cls_rule_format(*(uint8_t *) args[0][i], args[1][i], args[2][i]);
    }
}

static void
wrapper_ecall_rule_calculate_tag_s(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(uint32_t *) args[n_params - 1][i] = ecall_rule_calculate_tag_s(*(uint8_t *) args[0][i], *(uint8_t *) args[1][i], args[2][i]);
    }
}

static void
wrapper_ecall_ofproto_get_vlan_c(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_ofproto_get_vlan_c(*(uint8_t *) args[0][i]);
    }
}

static void
wrapper_ecall_ofproto_get_vlan_r(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[n_params - 1][i] = ecall_ofproto_get_vlan_r(*(uint8_t *) args[0][i], args[1][i], *(int *) args[2][i]);
    }
}

static void
wrapper_ecall_oftable_get_cls_rules(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(size_t *) args[n_params - 1][i] = ecall_oftable_get_cls_rules(
            *(uint8_t *) args[0][i],
            *(uint8_t *) args[1][i],
            *(size_t *) args[2][i],
            *(size_t *) args[3][i],
            (struct cls_rule **) args[4][i],
            *(size_t *) args[5][i],
            (size_t *) args[6][i]
        );
    }
}

static void
wrapper_ecall_ofproto_get_vlan_usage(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(size_t *) args[n_params - 1][i] = ecall_ofproto_get_vlan_usage(
            *(uint8_t *) args[0][i],
            *(size_t *) args[1][i],
            (uint16_t *) args[2][i],
            *(size_t *) args[3][i],
            *(size_t *) args[4][i],
            (size_t *) args[5][i]
        );
    }
}

static void
wrapper_ecall_ofproto_flush(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(size_t *) args[n_params - 1][i] = ecall_ofproto_flush(
            *(uint8_t *) args[0][i],
            (struct cls_rule **) args[1][i],
            (uint32_t *) args[2][i],
            *(size_t *) args[3][i],
            *(size_t *) args[4][i],
            *(size_t *) args[5][i],
            (size_t *) args[6][i]
        );
    }
}

static void
wrapper_ecall_ofproto_evict(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(size_t *) args[n_params - 1][i] = ecall_ofproto_evict(
            *(uint8_t *) args[0][i],
            *(uint8_t *) args[1][i],
            (uint32_t *) args[2][i],
            (struct cls_rule **) args[3][i],
            (uint8_t *) args[4][i],
            *(size_t *) args[5][i],
            (size_t *) args[6][i]
        );
    }
}

static void
wrapper_ecall_add_flow(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_add_flow(
            *(uint8_t *) args[0][i],
            *(uint8_t *) args[1][i],
            (struct cls_rule *) args[2][i],
            (struct cls_rule **) args[3][i],
            (struct cls_rule **) args[4][i],
            (struct match *) args[5][i],
            (uint32_t *) args[6][i],
            *(unsigned int *) args[7][i],
            *(uint16_t *) args[8][i],
            *(uint32_t *) args[9][i],
            (struct cls_rule **) args[10][i],
            *(int *) args[11][i],
            (uint16_t *) args[12][i],
            (unsigned int *) args[13][i]
        );
    }
}

static void
wrapper_ecall_collect_rules_strict(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(size_t *) args[n_params - 1][i] = ecall_collect_rules_strict(
            *(int *) args[0][i],
            *(int *) args[1][i],
            *(int *) args[2][i],
            (struct match *) args[3][i],
            *(unsigned int *) args[4][i],
            *(ovs_be64 *) args[5][i],
            *(ovs_be64 *) args[6][i],
            *(uint16_t *) args[7][i],
            (struct cls_rule **) args[8][i],
            (bool *) args[9][i],
            *(size_t *) args[10][i]
        );
    }
}

static void
wrapper_ecall_collect_rules_loose(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(size_t *) args[n_params - 1][i] = ecall_collect_rules_loose(
            *(uint8_t *) args[0][i],
            *(uint8_t *) args[1][i],
            *(int *) args[2][i],
            *(size_t *) args[3][i],
            (struct match *) args[4][i],
            (struct cls_rule **) args[5][i],
            *(size_t *) args[6][i],
            *(ovs_be64 *) args[7][i],
            *(ovs_be64 *) args[8][i],
            *(uint16_t *) args[9][i],
            (bool *) args[10][i],
            (size_t *) args[11][i]
        );
    }
}


static void
wrapper_ecall_ofproto_rule_send_removed(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_ofproto_rule_send_removed(
            *(uint8_t *) args[0][i],
            (struct cls_rule *) args[1][i],
            (struct match *) args[2][i],
            (unsigned int *) args[3][i],
            (bool *) args[4][i]
        );
    }
}


static void
wrapper_ecall_oftable_remove_rules(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_oftable_remove_rules(
            *(uint8_t *) args[0][i],
            (uint8_t *) args[1][i],
            (struct cls_rule **) args[2][i],
            (bool *) args[3][i],
            *(size_t *) args[4][i]
        );
    }
}

static void
wrapper_ecall_minimatch_expand_and_get_priority(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_minimatch_expand_and_get_priority(
            *(uint8_t *) args[0][i],
            (struct cls_rule *) args[1][i],
            (struct match *) args[2][i],
            (unsigned int *) args[3][i]
        );
    }
}

static void
wrapper_ecall_miniflow_expand_and_tag(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_miniflow_expand_and_tag(
            *(uint8_t *) args[0][i],
            (struct cls_rule *) args[1][i],
            (struct flow *) args[2][i],
            *(uint8_t *) args[3][i]
        );
    }
}

static void
wrapper_ecall_oftable_configure(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_oftable_configure(
            *(uint8_t *) args[0][i],
            *(uint8_t *) args[1][i],
            (char *) args[2][i],
            *(unsigned int *) args[3][i],
            (struct mf_subfield *) args[4][i],
            *(size_t *) args[5][i],
            *(uint32_t *) args[6][i],
            (bool *) args[7][i],
            (bool *) args[8][i]
        );
    }
}

static void
wrapper_ecall_need_to_evict(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(bool *) args[n_params - 1][i] = ecall_need_to_evict(
            *(uint8_t *) args[0][i],
            *(uint8_t *) args[1][i]
        );
    }
}

static void
wrapper_ecall_collect_rules_loose_stats_request(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(size_t *) args[n_params - 1][i] = ecall_collect_rules_loose_stats_request(
            *(uint8_t *) args[0][i],
            *(uint8_t *) args[1][i],
            *(int *) args[2][i],
            *(size_t *) args[3][i],
            *(size_t *) args[4][i],
            (struct match *) args[5][i],
            (struct cls_rule **) args[6][i],
            (struct match *) args[7][i],
            (unsigned int *) args[8][i],
            (size_t *) args[9][i]
        );
    }
}

static void
wrapper_ecall_delete_flows_loose(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(size_t *) args[n_params - 1][i] = ecall_delete_flows_loose(
            *(uint8_t *) args[0][i],
            *(uint8_t *) args[1][i],
            *(int *) args[2][i],
            *(size_t *) args[3][i],
            args[4][i],
            *(ovs_be64 *) args[5][i],
            *(ovs_be64 *) args[6][i],
            *(uint16_t *) args[7][i],
            args[8][i],
            args[9][i],
            args[10][i],
            args[11][i],
            *(size_t *) args[12][i],
            args[13][i],
            args[14][i],
            args[15][i],
            args[16][i]
        );
    }
}

static void
wrapper_ecall_delete_flows_strict(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(size_t *) args[n_params - 1][i] = ecall_delete_flows_strict(
            *(uint8_t *) args[0][i],
            *(uint8_t *) args[1][i],
            *(int *) args[2][i],
            args[3][i],
            *(unsigned int *) args[4][i],
            *(ovs_be64 *) args[5][i],
            *(ovs_be64 *) args[6][i],
            *(uint16_t *) args[7][i],
            args[8][i],
            args[9][i],
            args[10][i],
            args[11][i],
            args[12][i],
            args[13][i],
            args[14][i],
            *(size_t *) args[15][i]
        );
    }
}

static void
wrapper_ecall_modify_flows_strict(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(size_t *) args[n_params - 1][i] = ecall_modify_flows_strict(
            *(uint8_t *) args[0][i],
            *(uint8_t *) args[1][i],
            *(int *) args[2][i],
            args[3][i],
            *(unsigned int *) args[4][i],
            *(ovs_be64 *) args[5][i],
            *(ovs_be64 *) args[6][i],
            *(uint16_t *) args[7][i],
            args[8][i],
            args[9][i],
            args[10][i],
            args[11][i],
            *(size_t *) args[12][i]
        );
    }
}

static void
wrapper_ecall_modify_flows_loose(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        *(size_t *) args[n_params - 1][i] = ecall_modify_flows_loose(
            *(uint8_t *) args[0][i],
            *(uint8_t *) args[1][i],
            *(int *) args[2][i],
            *(size_t *) args[3][i],
            args[4][i],
            args[5][i],
            *(size_t *) args[6][i],
            *(ovs_be64 *) args[7][i],
            *(ovs_be64 *) args[8][i],
            *(uint16_t *) args[9][i],
            args[10][i],
            args[11][i],
            args[12][i],
            args[13][i]
        );
    }
}

static void
wrapper_ecall_minus_one(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_minus_one((int *) args[0][i]);
    }
}

static void
wrapper_ecall_plus_one(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_plus_one((int *) args[0][i]);
    }
}

#define CALL_TABLE_CAPACITY 256

void *call_table[CALL_TABLE_CAPACITY] = {
    // Vanilla hotcalls
    [hotcall_ecall_ofproto_get_vlan_c] = wrapper_ecall_ofproto_get_vlan_c,
    [hotcall_ecall_ofproto_get_vlan_r] = wrapper_ecall_ofproto_get_vlan_r,
    [hotcall_ecall_rule_calculate_tag_s] = wrapper_ecall_rule_calculate_tag_s,
    [hotcall_ecall_cls_rule_format] = wrapper_ecall_cls_rule_format,
    [hotcall_ecall_dpif_destroy_c] = wrapper_ecall_dpif_destroy_c,
    [hotcall_ecall_dpif_destroy_r] = wrapper_ecall_dpif_destroy_r,
    [hotcall_ecall_flow_stats_c] = wrapper_ecall_flow_stats_c,
    [hotcall_ecall_flow_stats_r] = wrapper_ecall_flow_stats_r,
    [hotcall_ecall_miniflow_get_vid] = wrapper_ecall_miniflow_get_vid,
    [hotcall_ecall_cls_rule_is_loose_match] = wrapper_ecall_cls_rule_is_loose_match,
    [hotcall_ecall_oftable_name] = wrapper_ecall_oftable_name,
    [hotcall_ecall_ofproto_destroy] = wrapper_ecall_ofproto_destroy,
    [hotcall_ecall_minimatch_expand] = wrapper_ecall_minimatch_expand,
    [hotcall_ecall_cr_rule_overlaps] = wrapper_ecall_cr_rule_overlaps,
    [hotcall_ecall_evg_add_rule] = wrapper_ecall_evg_add_rule,
    [hotcall_ecall_rule_eviction_priority] = wrapper_ecall_rule_eviction_priority,
    [hotcall_ecall_cls_rule_hash] = wrapper_ecall_cls_rule_hash,
    [hotcall_ecall_cls_remove] = wrapper_ecall_cls_remove,
    [hotcall_ecall_evg_remove_rule] = wrapper_ecall_evg_remove_rule,
    [hotcall_ecall_cr_priority] = wrapper_ecall_cr_priority,
    [hotcall_ecall_oftable_is_other_table] = wrapper_ecall_oftable_is_other_table,
    [hotcall_ecall_oftable_update_taggable] = wrapper_ecall_oftable_update_taggable,
    [hotcall_ecall_oftable_get_flags] = wrapper_ecall_oftable_get_flags,
    [hotcall_ecall_is_eviction_fields_enabled] = wrapper_ecall_is_eviction_fields_enabled,
    [hotcall_ecall_oftable_cls_count] = wrapper_ecall_oftable_cls_count,
    [hotcall_ecall_oftable_mflows] = wrapper_ecall_oftable_mflows,
    [hotcall_ecall_oftable_is_readonly] = wrapper_ecall_oftable_is_readonly,
    [hotcall_ecall_cls_rule_init] = wrapper_ecall_cls_rule_init,
    [hotcall_ecall_oftable_classifier_replace] = wrapper_ecall_oftable_classifier_replace,
    [hotcall_ecall_miniflow_expand] = wrapper_ecall_miniflow_expand,
    [hotcall_ecall_rule_calculate_tag] = wrapper_ecall_rule_calculate_tag,
    [hotcall_ecall_minimask_get_vid_mask] = wrapper_ecall_minimask_get_vid_mask,
    [hotcall_ecall_oftable_cls_lookup] = wrapper_ecall_oftable_cls_lookup,
    [hotcall_ecall_oftable_set_readonly] = wrapper_ecall_oftable_set_readonly,
    [hotcall_ecall_oftable_hidden_check] = wrapper_ecall_oftable_hidden_check,
    [hotcall_ecall_oftable_set_name] = wrapper_ecall_oftable_set_name,
    [hotcall_ecall_oftable_disable_eviction] = wrapper_ecall_oftable_disable_eviction,
    [hotcall_ecall_oftable_enable_eviction] = wrapper_ecall_oftable_enable_eviction,
    [hotcall_ecall_oftable_mflows_set] = wrapper_ecall_oftable_mflows_set,
    [hotcall_ecall_set_evictable] = wrapper_ecall_set_evictable,
    [hotcall_ecall_is_evictable] = wrapper_ecall_is_evictable,
    [hotcall_ecall_restore_evictable] = wrapper_ecall_restore_evictable,
    [hotcall_ecall_backup_evictable] = wrapper_ecall_backup_evictable,
    [hotcall_ecall_rule_update_used] = wrapper_ecall_rule_update_used,
    [hotcall_ecall_backup_and_set_evictable] = wrapper_ecall_backup_and_set_evictable,
    [hotcall_ecall_flush_c] = wrapper_ecall_flush_c,
    [hotcall_ecall_flush_r] = wrapper_ecall_flush_r,
    [hotcall_ecall_oftable_cls_find_match_exactly] = wrapper_ecall_oftable_cls_find_match_exactly,
    [hotcall_ecall_total_rules] = wrapper_ecall_total_rules,
    [hotcall_ecall_collect_rules_loose_c] = wrapper_ecall_collect_rules_loose_c,
    [hotcall_ecall_collect_rules_loose_r] = wrapper_ecall_collect_rules_loose_r,
    [hotcall_ecall_cls_rule_destroy] = wrapper_ecall_cls_rule_destroy,
    [hotcall_ecall_oftable_enable_eviction_c] = wrapper_ecall_oftable_enable_eviction_c,
    [hotcall_ecall_oftable_enable_eviction_r] = wrapper_ecall_oftable_enable_eviction_r,
    [hotcall_ecall_choose_rule_to_evict] = wrapper_ecall_choose_rule_to_evict,
    [hotcall_ecall_choose_rule_to_evict_p] = wrapper_ecall_choose_rule_to_evict_p,
    [hotcall_ecall_evg_remove_rule] = wrapper_ecall_evg_remove_rule,
    [hotcall_ecall_cls_rule_equal] = wrapper_ecall_cls_rule_equal,
    [hotcall_ecall_collect_rules_strict_c] = wrapper_ecall_collect_rules_strict_c,
    [hotcall_ecall_collect_rules_strict_r] = wrapper_ecall_collect_rules_strict_r,
    [hotcall_ecall_collect_ofmonitor_util_c] = wrapper_ecall_collect_ofmonitor_util_c,
    [hotcall_ecall_collect_ofmonitor_util_r] = wrapper_ecall_collect_ofmonitor_util_r,

    // Optimized hotcalls
    [hotcall_ecall_oftable_get_cls_rules] = wrapper_ecall_oftable_get_cls_rules,
    [hotcall_ecall_ofproto_get_vlan_usage] = wrapper_ecall_ofproto_get_vlan_usage,
    [hotcall_ecall_ofproto_flush] = wrapper_ecall_ofproto_flush,
    [hotcall_ecall_ofproto_evict] = wrapper_ecall_ofproto_evict,
    [hotcall_ecall_add_flow] = wrapper_ecall_add_flow,
    [hotcall_ecall_collect_rules_strict] = wrapper_ecall_collect_rules_strict,
    [hotcall_ecall_collect_rules_loose] = wrapper_ecall_collect_rules_loose,
    [hotcall_ecall_delete_flows_strict] = wrapper_ecall_delete_flows_strict,
    [hotcall_ecall_delete_flows_loose] = wrapper_ecall_delete_flows_loose,
    [hotcall_ecall_modify_flows_strict] = wrapper_ecall_modify_flows_strict,
    [hotcall_ecall_modify_flows_loose] = wrapper_ecall_modify_flows_loose,
    [hotcall_ecall_oftable_configure] = wrapper_ecall_oftable_configure,
    [hotcall_ecall_need_to_evict] = wrapper_ecall_need_to_evict,
    [hotcall_ecall_collect_rules_loose_stats_request] = wrapper_ecall_collect_rules_loose_stats_request,
    [hotcall_ecall_ofproto_rule_send_removed] = wrapper_ecall_ofproto_rule_send_removed,
    [hotcall_ecall_oftable_remove_rules] = wrapper_ecall_oftable_remove_rules,
    [hotcall_ecall_minimatch_expand_and_get_priority] = wrapper_ecall_minimatch_expand_and_get_priority,
    [hotcall_ecall_miniflow_expand_and_tag] = wrapper_ecall_miniflow_expand_and_tag,

    // Helpers
    [hotcall_ecall_plus_one] = wrapper_ecall_plus_one,
    [hotcall_ecall_minus_one] = wrapper_ecall_minus_one
};


void
batch_execute_function(uint8_t function_id, unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]) {
    //printf("Executing f %d\n", function_id);
    void (*f)(unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters]);
    f = call_table[function_id];
    #ifdef SGX_DEBUG
    if(!f) {
        printf("unknown hotcall function %d.\n", function_id);
    }
    #endif
    f(n_iters, n_params, args);

}


void
configure_hotcall() {
    struct hotcall_config conf = {
        .execute_function_legacy = NULL,
        .execute_function = batch_execute_function,
        .n_spinlock_jobs = 0,
        //.n_spinlock_jobs = 1,
        //.spin_lock_tasks = { &flow_map_cache_validate },
        //.spin_lock_task_timeouts = { 99999999 },
        //.spin_lock_task_count = { 0 }
    };
    struct hotcall_config *config = malloc(sizeof(struct hotcall_config));
    memcpy(config, &conf, sizeof(struct hotcall_config));
    hotcall_register_config(config);
    hotcall_configured = true;
}


void
ecall_ofproto_init_tables(uint8_t bridge_id, int n_tables){

    if(!hotcall_configured) {
        configure_hotcall();
        //e_ctx = malloc(sizeof(struct ovs_enclave_ctx));
        //memset(e_ctx, 0, sizeof(struct ovs_enclave_ctx));
    }

    struct oftable * table;

    e_ctx.SGX_n_tables[bridge_id] = n_tables;
    e_ctx.SGX_oftables[bridge_id] = xmalloc(n_tables * sizeof(struct oftable));
    OFPROTO_FOR_EACH_TABLE(table, e_ctx.SGX_oftables[bridge_id], e_ctx.SGX_n_tables[bridge_id]){
        oftable_init(table);
    }
    sgx_table_cls_init(bridge_id);
    sgx_table_dpif_init(bridge_id, n_tables);


    #ifdef BATCH_ALLOCATION

    batch_allocator_init(&e_ctx.cr_ba, sizeof(struct sgx_cls_rule));
    batch_allocator_add_block(&e_ctx.cr_ba);

    batch_allocator_init(&e_ctx.evg_ba, sizeof(struct eviction_group));
    batch_allocator_add_block(&e_ctx.evg_ba);

    /*batch_allocator_init(&heap_ba, sizeof(struct heap_node));
    batch_allocator_add_block(&heap_ba);*/

    #endif



}
