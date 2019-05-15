#include "call-table.h"
#include "ofproto-provider.h"
#include "enclave_t.h"
#include "common.h"
#include "enclave.h"
#include "cache-trusted.h"
#include "flow.h"

void
execute_function(struct function_call *fc, flow_map_cache *flow_cache){
    cls_cache_entry * cache_entry, * next;

    argument_list * args;
    args       = &fc->args;


    switch (fc->id) {
        case hotcall_ecall_backup_and_set_evictable:
            ecall_backup_and_set_evictable(
                *(uint8_t *) args->args[0],
                args->args[1],
                *(bool *) args->args[2]
            );
            break;
        case hotcall_ecall_restore_evictable:
            ecall_restore_evictable(
                *(uint8_t *) args->args[0],
                args->args[1]
            );
            break;
        case hotcall_ecall_backup_evictable:
            ecall_backup_evictable(
                *(uint8_t *) args->args[0],
                args->args[1]
            );
            break;
        case hotcall_ecall_set_evictable:
            ecall_set_evictable(
                *(uint8_t *) args->args[0],
                args->args[1],
                *(bool *) args->args[2]
            );
            break;
        case hotcall_ecall_is_evictable:
            *(bool *) fc->return_value = ecall_is_evictable(
                *(uint8_t *) args->args[0],
                args->args[1]
            );
            break;
        case hotcall_ecall_rule_update_used:
            ecall_rule_update_used(
                *(uint8_t *) args->args[0],
                args->args[1],
                *(uint32_t *) args->args[2]
            );
            break;
        case hotcall_ecall_oftable_is_readonly:
            *(int *) fc->return_value = ecall_oftable_is_readonly(
              *(uint8_t *) args->args[0],
              *(uint8_t *) args->args[1]
              );
            break;
        case hotcall_ecall_cls_rule_init:
            ecall_cls_rule_init(
              *(uint8_t *) args->args[0],
              args->args[1],
              (const struct match *) args->args[2],
              *(unsigned int *) args->args[3]
            );
            break;
        case hotcall_ecall_cls_rule_destroy:
        {
            struct cls_rule * ut_cr = (struct cls_rule *) args->args[1];
            ecall_cls_rule_destroy(*((int *) args->args[0]), ut_cr);
            flow_map_cache_remove_ut_cr(flow_cache, ut_cr);
            break;
        }
        case hotcall_ecall_cr_rule_overlaps:
            *(int *) fc->return_value = ecall_cr_rule_overlaps(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (struct cls_rule *) args->args[2]
            );
            break;

        case hotcall_ecall_oftable_set_readonly:
            ecall_oftable_set_readonly(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1]
            );
            break;
        case hotcall_ecall_oftable_set_name:
            ecall_oftable_set_name(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (char *) args->args[2]
            );
            break;
        case hotcall_ecall_oftable_disable_eviction:
            ecall_oftable_disable_eviction(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1]
            );
            break;
        case hotcall_ecall_oftable_mflows_set:
            ecall_oftable_mflows_set(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                *(unsigned int *) args->args[2]
            );
            break;
        case hotcall_ecall_oftable_cls_count:
            *(int *) fc->return_value = ecall_oftable_cls_count(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1]
            );
            break;
        case hotcall_ecall_oftable_mflows:
            *(unsigned int *) fc->return_value = ecall_oftable_mflows(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1]
            );
            break;
        case hotcall_ecall_is_eviction_fields_enabled:
            *(int *) fc->return_value = ecall_is_eviction_fields_enabled(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1]
            );
            break;
        case hotcall_ecall_flush_c:
            *(int *) fc->return_value = ecall_flush_c(
                *(uint8_t *) args->args[0]
            );
            break;
        case hotcall_ecall_flush_r:
            ecall_flush_r(
                *(uint8_t *) args->args[0],
                (struct cls_rule **) args->args[1],
                *(int *) args->args[2]
            );
            break;
        case hotcall_ecall_ofproto_destroy:
            ecall_ofproto_destroy(
                *(uint8_t *) args->args[0]
            );
            break;
        case hotcall_ecall_total_rules:
            *(unsigned int *) fc->return_value = ecall_total_rules(
                *(int *) args->args[0]
            );
            break;
        case hotcall_ecall_oftable_cls_find_match_exactly:
            ecall_oftable_cls_find_match_exactly(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (const struct match *) args->args[2],
                *(unsigned int *) args->args[3],
                (struct cls_rule **) args->args[4]
            );
            break;
        case hotcall_ecall_cr_priority:
            *(unsigned int *) fc->return_value = ecall_cr_priority(
                *(uint8_t *) args->args[0],
                (struct cls_rule *) args->args[1]
            );
            break;
        case hotcall_ecall_oftable_get_flags:
            *(enum oftable_flags *) fc->return_value = ecall_oftable_get_flags(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1]
            );
            break;
        case hotcall_ecall_oftable_name:
            ecall_oftable_name(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (char *) args->args[2],
                *(size_t *) args->args[3]
            );
            break;
        case hotcall_ecall_collect_rules_loose_c:
            *(int *) fc->return_value = ecall_collect_rules_loose_c(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                *(uint8_t *) args->args[2],
                (const struct match *) args->args[3]
            );
            break;
        case hotcall_ecall_collect_rules_loose_r:
            ecall_collect_rules_loose_r(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (struct cls_rule **) args->args[2],
                *(int *) args->args[3],
                *(uint8_t *) args->args[4],
                (const struct match *) args->args[5]
            );
            break;
        case hotcall_ecall_minimatch_expand:
            ecall_minimatch_expand(
                *(uint8_t *) args->args[0],
                (struct cls_rule *) args->args[1],
                (struct match *) args->args[2]
            );
            break;
        case hotcall_ecall_cls_rule_format:
            *(unsigned int *) fc->return_value = ecall_cls_rule_format(
                *(uint8_t *) args->args[0],
                (const struct cls_rule *) args->args[1],
                (struct match *) args->args[2]
            );
            break;
        case hotcall_ecall_flow_stats_c:
            *(int *) fc->return_value = ecall_flow_stats_c(
                *(uint8_t *) args->args[0]
            );
            break;
        case hotcall_ecall_flow_stats_r:
            ecall_flow_stats_r(
                *(uint8_t *) args->args[0],
                (struct cls_rule **) args->args[1],
                *(int *) args->args[2]
            );
            break;
        case hotcall_ecall_cls_rule_hash:
            *((uint32_t *) fc->return_value) = ecall_cls_rule_hash(
                *(uint8_t *) args->args[0],
                (const struct cls_rule *) args->args[1],
                *(uint32_t *) args->args[2]
            );
            break;
        case hotcall_ecall_cls_rule_equal:
            *((int *) fc->return_value) = ecall_cls_rule_equal(
                *(uint8_t *) args->args[0],
                (const struct cls_rule *) args->args[1],
                (const struct cls_rule *) args->args[2]
            );
            break;
        case hotcall_ecall_choose_rule_to_evict:
            ecall_choose_rule_to_evict(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (struct cls_rule **) args->args[2]
            );
            break;
        case hotcall_ecall_choose_rule_to_evict_p:
            ecall_choose_rule_to_evict_p(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (struct cls_rule **) args->args[2],
                (struct cls_rule *) args->args[3]
            );
            break;
        case hotcall_ecall_collect_ofmonitor_util_c:
            *(int *) fc->return_value = ecall_collect_ofmonitor_util_c(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                *(uint8_t *) args->args[2],
                (const struct minimatch *) args->args[3]
            );
            break;
        case hotcall_ecall_collect_ofmonitor_util_r:
            ecall_collect_ofmonitor_util_r(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (struct cls_rule **) args->args[2],
                *(int *) args->args[3],
                *(uint8_t *) args->args[4],
                (const struct minimatch *) args->args[5]
            );
            break;
        case hotcall_ecall_cls_rule_is_loose_match:
            *(int *) fc->return_value = ecall_cls_rule_is_loose_match(
                *(uint8_t *) args->args[0],
                (struct cls_rule *) args->args[1],
                (const struct minimatch *) args->args[2]
            );
            break;
        case hotcall_ecall_minimask_get_vid_mask:
            *(uint16_t *) fc->return_value = ecall_minimask_get_vid_mask(
                *(uint8_t *) args->args[0],
                (struct cls_rule *) args->args[1]
            );
            break;
        case hotcall_ecall_miniflow_get_vid:
            *(uint16_t *) fc->return_value = ecall_miniflow_get_vid(
                *(uint8_t *) args->args[0],
                (struct cls_rule *) args->args[1]
            );
            break;
        case hotcall_ecall_evg_add_rule:
            ecall_evg_add_rule(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (struct cls_rule *) args->args[2],
                (uint32_t *) args->args[3],
                *(uint32_t *) args->args[4]
            );
            break;
        case hotcall_ecall_oftable_enable_eviction:
            ecall_oftable_enable_eviction(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (const struct mf_subfield *) args->args[2],
                *(size_t *) args->args[3],
                *(uint32_t *) args->args[4],
                (bool *) args->args[5]
            );
            break;
        case hotcall_ecall_oftable_enable_eviction_c:
            *(int *) fc->return_value = ecall_oftable_enable_eviction_c(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1]
            );
            break;
        case hotcall_ecall_oftable_enable_eviction_r:
            ecall_oftable_enable_eviction_r(
                *(uint8_t *) args->args[0],
                (struct cls_rule **) args->args[1],
                *(int *) args->args[2],
                *(uint8_t *) args->args[3]
            );
            break;
        case hotcall_ecall_cls_remove:
        {
            uint8_t bridge_id = *(uint8_t *) args->args[0];
            uint8_t table_id  = *(uint8_t *) args->args[1];
            struct cls_rule * ut_cr = (struct cls_rule *) args->args[2];

            ecall_cls_remove(bridge_id, table_id, ut_cr);
            flow_map_cache_remove_ut_cr(flow_cache, ut_cr);
            break;
        }
        case hotcall_ecall_oftable_classifier_replace:
        {
            uint8_t bridge_id = *(uint8_t *) args->args[0];
            uint8_t table_id  = *(uint8_t *) args->args[1];
            struct cls_rule * ut_cr_insert  = (struct cls_rule *) args->args[2];
            struct cls_rule ** ut_cr_remove = (struct cls_rule **) args->args[3];

            ecall_oftable_classifier_replace(bridge_id, table_id, ut_cr_insert, ut_cr_remove);

            if (!*ut_cr_remove) {
                return;
            }
            flow_map_cache_remove_ut_cr(flow_cache, *ut_cr_remove);
            break;
        }
        case hotcall_ecall_ofproto_get_vlan_c:
            *(int *) fc->return_value = ecall_ofproto_get_vlan_c(
                *(uint8_t *) args->args[0]
            );
            break;
        case hotcall_ecall_ofproto_get_vlan_r:
            ecall_ofproto_get_vlan_r(
                *(uint8_t *) args->args[0],
                (uint16_t *) args->args[1],
                *((int *) args->args[2])
            );
            break;
        case hotcall_ecall_collect_rules_strict_c:
            *((int *) fc->return_value) = ecall_collect_rules_strict_c(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                *(uint8_t *) args->args[2],
                (const struct match *) args->args[3],
                *(unsigned int *) args->args[4]
            );
            break;
        case hotcall_ecall_collect_rules_strict_r:
            ecall_collect_rules_strict_r(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (struct cls_rule **) args->args[2],
                *(int *) args->args[3],
                *(uint8_t *) args->args[4],
                (const struct match *) args->args[5],
                *(unsigned int *) args->args[6]
            );
            break;
        case hotcall_ecall_oftable_hidden_check:
            ecall_oftable_hidden_check(
                *(uint8_t *) args->args[0]
            );
            break;
        case hotcall_ecall_miniflow_expand:
            ecall_miniflow_expand(
                *(uint8_t *) args->args[0],
                (struct cls_rule *) args->args[1],
                (struct flow *) args->args[2]
            );
            break;
        case hotcall_ecall_rule_calculate_tag:
            *(uint32_t *) fc->return_value = ecall_rule_calculate_tag(
                *(uint8_t *) args->args[0],
                (struct cls_rule *) args->args[1],
                (const struct flow *) args->args[2],
                *(uint8_t *) args->args[3]
            );
            break;
        case hotcall_ecall_rule_calculate_tag_s:
            *(uint32_t *) fc->return_value = ecall_rule_calculate_tag_s(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (const struct flow *) args->args[2]
            );
            break;
        case hotcall_ecall_oftable_update_taggable:
        {
            int * ret_value = (int *) fc->return_value;
            *ret_value = ecall_oftable_update_taggable(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1]
            );
            if (*ret_value == 4) {
                flow_map_cache_flush(flow_cache);
            }
        }
        break;
        case hotcall_ecall_oftable_is_other_table:
            *(int *) fc->return_value = ecall_oftable_is_other_table(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1]
            );
            break;
        case hotcall_ecall_oftable_cls_lookup:
        {
            const struct flow * flow   = (const struct flow *) args->args[3];
            struct flow_wildcards * wc = args->args[4];
            int table_id  = *(uint8_t *) args->args[2];
            int bridge_id = *(uint8_t *) args->args[0];
            struct cls_rule ** ut_cr = (struct cls_rule **) args->args[1];

            ecall_oftable_cls_lookup(bridge_id,
              ut_cr,
              table_id,
              flow,
              wc);

            if (*ut_cr) {
                flow_map_cache_insert(flow_cache, flow, wc, *ut_cr, bridge_id, table_id);
            }
            break;
        }
        case hotcall_ecall_evg_remove_rule:
            ecall_evg_remove_rule(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                 (struct cls_rule *) args->args[2]
             );
            break;
        case hotcall_ecall_dpif_destroy_c:
            *(int *) fc->return_value = ecall_dpif_destroy_c(
                *(uint8_t *) args->args[0]
            );
            break;
        case hotcall_ecall_dpif_destroy_r:
            ecall_dpif_destroy_r(
                *(uint8_t *) args->args[0],
                (struct cls_rule **) args->args[1],
                *(int *) args->args[2]
            );
            break;
        case hotcall_ecall_oftable_get_cls_rules:
            *(size_t *) fc->return_value = ecall_oftable_get_cls_rules(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                *(size_t *) args->args[2],
                *(size_t *) args->args[3],
                (struct cls_rule **) args->args[4],
                *(size_t *) args->args[5],
                (size_t *) args->args[6]
            );
            break;
        case hotcall_ecall_get_cls_rules_and_enable_eviction:
            *(size_t *) fc->return_value = ecall_get_cls_rules_and_enable_eviction(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                *(size_t *) args->args[2],
                *(size_t *) args->args[3],
                (struct cls_rule **) args->args[4],
                *(size_t *) args->args[5],
                (size_t *) args->args[6],
                (const struct mf_subfield *) args->args[7],
                *(size_t *) args->args[8],
                *(uint32_t *) args->args[9],
                (bool *) args->args[10],
                (bool *) args->args[11]
            );
            break;
        case hotcall_ecall_eviction_group_add_rules:
            ecall_eviction_group_add_rules(
                *(int *) args->args[0],
                *(int *) args->args[1],
                *(size_t *) args->args[2],
                (struct cls_rule **) args->args[3],
                (uint32_t *) args->args[4]
            );
            break;
        case hotcall_ecall_ofproto_get_vlan_usage:
            *(size_t *) fc->return_value = ecall_ofproto_get_vlan_usage(
                *(uint8_t *) args->args[0],
                *(size_t *) args->args[1],
                (uint16_t *) args->args[2],
                *(size_t *) args->args[3],
                *(size_t *) args->args[4],
                (size_t *) args->args[5]
            );
            break;
        case hotcall_ecall_ofproto_flush:
            *(size_t *) fc->return_value = ecall_ofproto_flush(
                *(uint8_t *) args->args[0],
                (struct cls_rule **) args->args[1],
                (uint32_t *) args->args[2],
                *(size_t *) args->args[3],
                *(size_t *) args->args[4],
                *(size_t *) args->args[5],
                (size_t *) args->args[6]
            );
            flow_map_cache_flush(flow_cache);
            break;
        case hotcall_ecall_ofproto_evict:
            *(size_t *) fc->return_value = ecall_ofproto_evict(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                *(size_t *) args->args[2],
                (uint32_t *) args->args[3],
                (struct cls_rule **) args->args[4],
                *(size_t *) args->args[5],
                (size_t *) args->args[6]
            );
            break;
        case hotcall_ecall_add_flow:
            ecall_add_flow(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (struct cls_rule *) args->args[2],
                (struct cls_rule **) args->args[3],
                (struct cls_rule **) args->args[4],
                (struct match *) args->args[5],
                (uint32_t *) args->args[6],
                (uint16_t *) args->args[7],
                (uint16_t *) args->args[8],
                *(unsigned int *) args->args[9],
                *(uint16_t *) args->args[10],
                *(uint32_t *) args->args[11],
                (struct cls_rule **) args->args[12],
                *(int *) args->args[13],
                *(bool *) args->args[14],
                (uint16_t *) args->args[15],
                (int *) args->args[16]
            );
            break;
        case hotcall_ecall_collect_rules_loose:
            *(size_t *) fc->return_value = ecall_collect_rules_loose(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                *(int *) args->args[2],
                *(size_t *) args->args[3],
                (struct match *) args->args[4],
                (bool *) args->args[5],
                (struct cls_rule **) args->args[6],
                *(size_t *) args->args[7],
                (bool *) args->args[8],
                (size_t *) args->args[9]
            );
            break;
        case hotcall_ecall_collect_rules_strict:
            *(size_t *) fc->return_value = ecall_collect_rules_strict(
                *(int *) args->args[0],
                *(int *) args->args[1],
                *(int *) args->args[2],
                (struct match *) args->args[3],
                *(unsigned int *) args->args[4],
                (bool *) args->args[5],
                (struct cls_rule **) args->args[6],
                (bool *) args->args[7],
                *(size_t *) args->args[8]
            );
            break;
        case hotcall_ecall_delete_flows:
        {
            struct cls_rule ** ut_crs = (struct cls_rule **) args->args[2];
            size_t n = *(size_t *) args->args[7];
            ecall_delete_flows(
              *(uint8_t *) args->args[0],
              (uint8_t *) args->args[1],
              ut_crs,
              (bool *) args->args[3],
              (uint32_t *) args->args[4],
              (unsigned int *) args->args[5],
              (struct match *) args->args[6],
              n
            );
            for (size_t i = 0; i < n; ++i) {
                flow_map_cache_remove_ut_cr(flow_cache, ut_crs[i]);
            }
            break;
        }
        case hotcall_ecall_oftable_configure:
            ecall_oftable_configure(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                (char *) args->args[2],
                *(unsigned int *) args->args[3],
                (struct mf_subfield *) args->args[4],
                *(size_t *) args->args[5],
                (bool *) args->args[6],
                (bool *) args->args[7]
            );
            break;
        case hotcall_ecall_need_to_evict:
            *(bool *) fc->return_value = ecall_need_to_evict(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1]
            );
            break;
        case hotcall_ecall_collect_rules_loose_stats_request:
            *(size_t *) fc->return_value = ecall_collect_rules_loose_stats_request(
                *(uint8_t *) args->args[0],
                *(uint8_t *) args->args[1],
                *(int *) args->args[2],
                *(size_t *) args->args[3],
                *(size_t *) args->args[4],
                (struct match *) args->args[5],
                (struct cls_rule **) args->args[6],
                (struct match *) args->args[7],
                (unsigned int *) args->args[8],
                (size_t *) args->args[9]
            );
            break;
        case hotcall_ecall_ofproto_rule_send_removed:
            ecall_ofproto_rule_send_removed(
                *(uint8_t *) args->args[0],
                (struct cls_rule *) args->args[1],
                (struct match *) args->args[2],
                (unsigned int *) args->args[3],
                (bool *) args->args[4]
            );
            break;
        case hotcall_ecall_oftable_remove_rules:
        {
            struct cls_rule ** ut_crs = (struct cls_rule **) args->args[2];
            size_t n = *(size_t *) args->args[4];
            ecall_oftable_remove_rules(
                *(uint8_t *) args->args[0],
                (uint8_t *) args->args[1],
                (struct cls_rule **) args->args[2],
                (bool *) args->args[3],
                *(size_t *) args->args[4]
             );
            for (size_t i = 0; i < n; ++i) {
                flow_map_cache_remove_ut_cr(flow_cache, ut_crs[i]);
            }
            break;
        }
        case hotcall_ecall_minimatch_expand_and_get_priority:
            ecall_minimatch_expand_and_get_priority(
                *(uint8_t *) args->args[0],
                (struct cls_rule *) args->args[1],
                (struct match *) args->args[2],
                (unsigned int *) args->args[3]
            );
            break;
        case hotcall_ecall_miniflow_expand_and_tag:
            ecall_miniflow_expand_and_tag(
                *(uint8_t *) args->args[0],
                (struct cls_rule *) args->args[1],
                (struct flow *) args->args[2],
                *(uint8_t *) args->args[3]
            );
            break;
        /*case hotcall_ecall_ofproto_evict_get_rest:
         *  ecall_ofproto_evict_get_rest(
         *      (uint32_t *) args->args[0],
         *      (struct cls_rule **) args->args[1],
         * *(size_t *) args->args[2])
         *  );
         *  break;*/


        /*case ecall_destroy_rule_if_overlaps:
         *  ecall_destroy_rule_if_overlaps(*(int *) args->args[1]), (struct cls_rule *) args->args[2]);
         *  break;
         * case ecall_get_rule_to_evict_if_neccesary:
         * *((bool *) fc->return_value) = ecall_get_rule_to_evict_if_neccesary(*((int *) args->args[1]),
         * (struct cls_rule *) args->args[2]);
         *  break;
         * case ecall_miniflow_expand_and_tag:
         * *((uint32_t *) fc->return_value) =
         * ecall_miniflow_expand_and_tag((struct cls_rule *) args->args[1], (struct flow *) args->args[2],
         * *((int *) args->args[3]));
         *  break;
         * case ecall_allocate_cls_rule_if_not_read_only:
         * *((bool *) fc->return_value) =
         * ecall_allocate_cls_rule_if_not_read_only(*((int *) args->args[1]), (struct cls_rule *) args->args[2],
         * (struct match *) args->args[3],
         * *((unsigned int *) args->args[4]));
         *  break;
         * case ecall_classifier_replace_if_modifiable:
         *  ecall_classifer_replace_if_modifiable(*((int *) args->args[1]),
         * (struct cls_rule *) args->args[2],
         * (struct cls_rule **) args->args[3],
         * (bool *) args->args[4]);
         *  break;
         * case ecall_ofproto_configure_table:
         * *((bool *) fc->return_value) = ecall_ofproto_configure_table(
         * *((int *) args->args[1]),
         * (struct mf_subfield *) args->args[2],
         * (char *) args->args[3],
         * *((unsigned int *) args->args[4]),
         * *((unsigned int *) args->args[5]),
         * *((unsigned int *) args->args[6]),
         * (unsigned int *) args->args[7],
         * (struct cls_rule *) args->args[8],
         * (bool *) args->args[9]);
         *
         *  break;*/

        default:
            printf("Error, no matching switch case for %d in call table.\n", fc->id);
    }
} /* execute_function */
