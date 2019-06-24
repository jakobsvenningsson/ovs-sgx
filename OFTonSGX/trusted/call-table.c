#include "call-table.h"
#include "ofproto-provider.h"
#include "enclave_t.h"
#include "hotcall.h"
#include "enclave.h"
#include "cache-trusted.h"
#include "flow.h"
#include "functions.h"

void
execute_function(uint8_t function_id, void *args[], void *return_value){


    //cls_cache_entry * cache_entry, * next;

    switch (function_id) {
        case hotcall_ecall_rule_eviction_priority:
            *(unsigned int *) return_value = rule_eviction_priority(args[0], *(uint32_t *) args[1]);
            break;
        case hotcall_ecall_plus_one:
            ecall_plus_one((int *) args[0]);
            break;
        case hotcall_ecall_container_of:
            ecall_offset_of(args[0], -(*(int *) args[1]));
            break;
        case hotcall_ecall_ofproto_init_tables:
            ecall_ofproto_init_tables(*(int *) args[0], *(uint8_t *) args[1]);
            break;
        case hotcall_ecall_ofproto_is_flow_deletion_pending:
            *(bool *) return_value = ecall_ofproto_is_flow_deletion_pending(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (struct hmap *) args[2],
                (struct cls_rule *) args[3]
            );
            break;
        case hotcall_ecall_configure_tables:
            ecall_configure_tables(
                *(uint8_t *) args[0],
                *(int *) args[1],
                *(uint32_t *) args[2],
                args[3],
                args[4]
            );
            break;
        case hotcall_ecall_backup_and_set_evictable:
            ecall_backup_and_set_evictable(
                *(uint8_t *) args[0],
                args[1],
                *(bool *) args[2]
            );
            break;
        case hotcall_ecall_restore_evictable:
            ecall_restore_evictable(
                *(uint8_t *) args[0],
                args[1]
            );
            break;
        case hotcall_ecall_backup_evictable:
            ecall_backup_evictable(
                *(uint8_t *) args[0],
                args[1]
            );
            break;
        case hotcall_ecall_set_evictable:
            ecall_set_evictable(
                *(uint8_t *) args[0],
                args[1],
                *(bool *) args[2]
            );
            break;
        case hotcall_ecall_is_evictable:
            *(bool *) return_value = ecall_is_evictable(
                *(uint8_t *) args[0],
                args[1]
            );
            break;
        case hotcall_ecall_rule_update_used:
            ecall_rule_update_used(
                *(uint8_t *) args[0],
                args[1],
                *(uint32_t *) args[2]
            );
            break;
        case hotcall_ecall_oftable_is_readonly:
            *(int *) return_value = ecall_oftable_is_readonly(
              *(uint8_t *) args[0],
              *(uint8_t *) args[1]
              );
            break;
        case hotcall_ecall_cls_rule_init:
            ecall_cls_rule_init(
              *(uint8_t *) args[0],
              args[1],
              (const struct match *) args[2],
              *(unsigned int *) args[3]
            );
            break;
        case hotcall_ecall_cls_rule_destroy:
        {
            struct cls_rule * ut_cr = (struct cls_rule *) args[1];
            ecall_cls_rule_destroy(*((int *) args[0]), ut_cr);
            //flow_map_cache_remove_ut_cr(flow_cache, ut_cr);
            break;
        }
        case hotcall_ecall_cr_rule_overlaps:
            *(int *) return_value = ecall_cr_rule_overlaps(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (struct cls_rule *) args[2]
            );
            break;

        case hotcall_ecall_oftable_set_readonly:
            ecall_oftable_set_readonly(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1]
            );
            break;
        case hotcall_ecall_oftable_set_name:
            ecall_oftable_set_name(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (char *) args[2]
            );
            break;
        case hotcall_ecall_oftable_disable_eviction:
            ecall_oftable_disable_eviction(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1]
            );
            break;
        case hotcall_ecall_oftable_mflows_set:
            ecall_oftable_mflows_set(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                *(unsigned int *) args[2]
            );
            break;
        case hotcall_ecall_oftable_cls_count:
            *(int *) return_value = ecall_oftable_cls_count(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1]
            );
            break;
        case hotcall_ecall_oftable_mflows:
            *(unsigned int *) return_value = ecall_oftable_mflows(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1]
            );
            break;
        case hotcall_ecall_is_eviction_fields_enabled:
            *(int *) return_value = ecall_is_eviction_fields_enabled(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1]
            );
            break;
        case hotcall_ecall_flush_c:
            *(int *) return_value = ecall_flush_c(
                *(uint8_t *) args[0]
            );
            break;
        case hotcall_ecall_flush_r:
            ecall_flush_r(
                *(uint8_t *) args[0],
                (struct cls_rule **) args[1],
                *(int *) args[2]
            );
            break;
        case hotcall_ecall_ofproto_destroy:
            ecall_ofproto_destroy(
                *(uint8_t *) args[0]
            );
            break;
        case hotcall_ecall_total_rules:
            *(unsigned int *) return_value = ecall_total_rules(
                *(int *) args[0]
            );
            break;
        case hotcall_ecall_oftable_cls_find_match_exactly:
            ecall_oftable_cls_find_match_exactly(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (const struct match *) args[2],
                *(unsigned int *) args[3],
                (struct cls_rule **) args[4]
            );
            break;
        case hotcall_ecall_cr_priority:
            *(unsigned int *) return_value = ecall_cr_priority(
                *(uint8_t *) args[0],
                (struct cls_rule *) args[1]
            );
            break;
        case hotcall_ecall_oftable_get_flags:
            *(enum oftable_flags *) return_value = ecall_oftable_get_flags(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1]
            );
            break;
        case hotcall_ecall_oftable_name:
            ecall_oftable_name(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (char *) args[2],
                *(size_t *) args[3]
            );
            break;
        case hotcall_ecall_collect_rules_loose_c:
            *(int *) return_value = ecall_collect_rules_loose_c(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                *(uint8_t *) args[2],
                (const struct match *) args[3]
            );
            break;
        case hotcall_ecall_collect_rules_loose_r:
            ecall_collect_rules_loose_r(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (struct cls_rule **) args[2],
                *(int *) args[3],
                *(uint8_t *) args[4],
                (const struct match *) args[5]
            );
            break;
        case hotcall_ecall_minimatch_expand:
            ecall_minimatch_expand(
                *(uint8_t *) args[0],
                (struct cls_rule *) args[1],
                (struct match *) args[2]
            );
            break;
        case hotcall_ecall_cls_rule_format:
            *(unsigned int *) return_value = ecall_cls_rule_format(
                *(uint8_t *) args[0],
                (const struct cls_rule *) args[1],
                (struct match *) args[2]
            );
            break;
        case hotcall_ecall_flow_stats_c:
            *(int *) return_value = ecall_flow_stats_c(
                *(uint8_t *) args[0]
            );
            break;
        case hotcall_ecall_flow_stats_r:
            ecall_flow_stats_r(
                *(uint8_t *) args[0],
                (struct cls_rule **) args[1],
                *(int *) args[2]
            );
            break;
        case hotcall_ecall_cls_rule_hash:
            *((uint32_t *) return_value) = ecall_cls_rule_hash(
                *(uint8_t *) args[0],
                (const struct cls_rule *) args[1],
                *(uint32_t *) args[2]
            );
            break;
        case hotcall_ecall_cls_rule_equal:
            *((int *) return_value) = ecall_cls_rule_equal(
                *(uint8_t *) args[0],
                (const struct cls_rule *) args[1],
                (const struct cls_rule *) args[2]
            );
            break;
        case hotcall_ecall_choose_rule_to_evict:
            ecall_choose_rule_to_evict(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (struct cls_rule **) args[2]
            );
            break;
        case hotcall_ecall_choose_rule_to_evict_p:
            ecall_choose_rule_to_evict_p(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (struct cls_rule **) args[2],
                (struct cls_rule *) args[3]
            );
            break;
        case hotcall_ecall_collect_ofmonitor_util_c:
            *(int *) return_value = ecall_collect_ofmonitor_util_c(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                *(uint8_t *) args[2],
                (const struct minimatch *) args[3]
            );
            break;
        case hotcall_ecall_collect_ofmonitor_util_r:
            ecall_collect_ofmonitor_util_r(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (struct cls_rule **) args[2],
                *(int *) args[3],
                *(uint8_t *) args[4],
                (const struct minimatch *) args[5]
            );
            break;
        case hotcall_ecall_cls_rule_is_loose_match:
            *(int *) return_value = ecall_cls_rule_is_loose_match(
                *(uint8_t *) args[0],
                (struct cls_rule *) args[1],
                (const struct minimatch *) args[2]
            );
            break;
        case hotcall_ecall_minimask_get_vid_mask:
            *(uint16_t *) return_value = ecall_minimask_get_vid_mask(
                *(uint8_t *) args[0],
                (struct cls_rule *) args[1]
            );
            break;
        case hotcall_ecall_miniflow_get_vid:
            *(uint16_t *) return_value = ecall_miniflow_get_vid(
                *(uint8_t *) args[0],
                (struct cls_rule *) args[1]
            );
            break;
        case hotcall_ecall_evg_add_rule:
            ecall_evg_add_rule(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (struct cls_rule *) args[2],
                (uint32_t *) args[3],
                *(uint32_t *) args[4]
            );
            break;
        case hotcall_ecall_oftable_enable_eviction:
            ecall_oftable_enable_eviction(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (const struct mf_subfield *) args[2],
                *(size_t *) args[3],
                *(uint32_t *) args[4],
                (bool *) args[5]
            );
            break;
        case hotcall_ecall_oftable_enable_eviction_c:
            *(int *) return_value = ecall_oftable_enable_eviction_c(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1]
            );
            break;
        case hotcall_ecall_oftable_enable_eviction_r:
            *(int *) return_value = ecall_oftable_enable_eviction_r(
                *(uint8_t *) args[0],
                (struct cls_rule **) args[1],
                *(int *) args[2],
                *(uint8_t *) args[3],
                (int *) args[4]
            );
            break;
        case hotcall_ecall_cls_remove:
        {
            uint8_t bridge_id = *(uint8_t *) args[0];
            uint8_t table_id  = *(uint8_t *) args[1];
            struct cls_rule * ut_cr = (struct cls_rule *) args[2];

            ecall_cls_remove(bridge_id, table_id, ut_cr);
            //flow_map_cache_remove_ut_cr(flow_cache, ut_cr);
            break;
        }
        case hotcall_ecall_oftable_classifier_replace:
        {
            uint8_t bridge_id = *(uint8_t *) args[0];
            uint8_t table_id  = *(uint8_t *) args[1];
            struct cls_rule * ut_cr_insert  = (struct cls_rule *) args[2];
            struct cls_rule ** ut_cr_remove = (struct cls_rule **) args[3];

            ecall_oftable_classifier_replace(bridge_id, table_id, ut_cr_insert, ut_cr_remove);

            if (!*ut_cr_remove) {
                return;
            }
            //flow_map_cache_remove_ut_cr(flow_cache, *ut_cr_remove);
            break;
        }
        case hotcall_ecall_ofproto_get_vlan_c:
            *(int *) return_value = ecall_ofproto_get_vlan_c(
                *(uint8_t *) args[0]
            );
            break;
        case hotcall_ecall_ofproto_get_vlan_r:
            ecall_ofproto_get_vlan_r(
                *(uint8_t *) args[0],
                (uint16_t *) args[1],
                *((int *) args[2])
            );
            break;
        case hotcall_ecall_collect_rules_strict_c:
            *((int *) return_value) = ecall_collect_rules_strict_c(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                *(uint8_t *) args[2],
                (const struct match *) args[3],
                *(unsigned int *) args[4]
            );
            break;
        case hotcall_ecall_collect_rules_strict_r:
            ecall_collect_rules_strict_r(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (struct cls_rule **) args[2],
                *(int *) args[3],
                *(uint8_t *) args[4],
                (const struct match *) args[5],
                *(unsigned int *) args[6]
            );
            break;
        case hotcall_ecall_oftable_hidden_check:
            ecall_oftable_hidden_check(
                *(uint8_t *) args[0]
            );
            break;
        case hotcall_ecall_miniflow_expand:
            ecall_miniflow_expand(
                *(uint8_t *) args[0],
                (struct cls_rule *) args[1],
                (struct flow *) args[2]
            );
            break;
        case hotcall_ecall_rule_calculate_tag:
            *(uint32_t *) return_value = ecall_rule_calculate_tag(
                *(uint8_t *) args[0],
                (struct cls_rule *) args[1],
                (const struct flow *) args[2],
                *(uint8_t *) args[3]
            );
            break;
        case hotcall_ecall_rule_calculate_tag_s:
            *(uint32_t *) return_value = ecall_rule_calculate_tag_s(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (const struct flow *) args[2]
            );
            break;
        case hotcall_ecall_oftable_update_taggable:
        {
            int * ret_value = (int *) return_value;
            *ret_value = ecall_oftable_update_taggable(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1]
            );
            if (*ret_value == 4) {
                //flow_map_cache_flush(flow_cache);
            }
        }
        break;
        case hotcall_ecall_oftable_is_other_table:
            *(int *) return_value = ecall_oftable_is_other_table(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1]
            );
            break;
        case hotcall_ecall_oftable_cls_lookup:
        {
            const struct flow * flow   = (const struct flow *) args[3];
            struct flow_wildcards * wc = args[4];
            int table_id  = *(uint8_t *) args[2];
            int bridge_id = *(uint8_t *) args[0];
            struct cls_rule ** ut_cr = (struct cls_rule **) args[1];

            ecall_oftable_cls_lookup(bridge_id,
              ut_cr,
              table_id,
              flow,
              wc);

            if (*ut_cr) {
                //flow_map_cache_insert(flow_cache, flow, wc, *ut_cr, bridge_id, table_id);
            }
            break;
        }
        case hotcall_ecall_evg_remove_rule:
            ecall_evg_remove_rule(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                 (struct cls_rule *) args[2]
             );
            break;
        case hotcall_ecall_dpif_destroy_c:
            *(int *) return_value = ecall_dpif_destroy_c(
                *(uint8_t *) args[0]
            );
            break;
        case hotcall_ecall_dpif_destroy_r:
            ecall_dpif_destroy_r(
                *(uint8_t *) args[0],
                (struct cls_rule **) args[1],
                *(int *) args[2]
            );
            break;
        case hotcall_ecall_oftable_get_cls_rules:
            *(size_t *) return_value = ecall_oftable_get_cls_rules(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                *(size_t *) args[2],
                *(size_t *) args[3],
                (struct cls_rule **) args[4],
                *(size_t *) args[5],
                (size_t *) args[6]
            );
            break;





        /*case hotcall_ecall_get_cls_rules_and_enable_eviction:
            *(size_t *) return_value = ecall_get_cls_rules_and_enable_eviction(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                *(size_t *) args[2],
                *(size_t *) args[3],
                (struct cls_rule **) args[4],
                *(size_t *) args[5],
                (size_t *) args[6],
                (const struct mf_subfield *) args[7],
                *(size_t *) args[8],
                *(uint32_t *) args[9],
                *(uint32_t *) args[10],
                (bool *) args[11],
                (bool *) args[12]
            );
            break;
        case hotcall_ecall_eviction_group_add_rules:
            ecall_eviction_group_add_rules(
                *(int *) args[0],
                *(int *) args[1],
                *(size_t *) args[2],
                (struct cls_rule **) args[3],
                (uint32_t *) args[4],
                *(uint32_t *) args[5]
            );
            break;*/




        case hotcall_ecall_ofproto_get_vlan_usage:
            *(size_t *) return_value = ecall_ofproto_get_vlan_usage(
                *(uint8_t *) args[0],
                *(size_t *) args[1],
                (uint16_t *) args[2],
                *(size_t *) args[3],
                *(size_t *) args[4],
                (size_t *) args[5]
            );
            break;
        case hotcall_ecall_ofproto_flush:
            *(size_t *) return_value = ecall_ofproto_flush(
                *(uint8_t *) args[0],
                (struct cls_rule **) args[1],
                (uint32_t *) args[2],
                *(size_t *) args[3],
                *(size_t *) args[4],
                *(size_t *) args[5],
                (size_t *) args[6]
            );
            //flow_map_cache_flush(flow_cache);
            break;
        case hotcall_ecall_ofproto_evict:
            *(size_t *) return_value = ecall_ofproto_evict(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (uint32_t *) args[2],
                (struct cls_rule **) args[3],
                (uint8_t *) args[4],
                *(size_t *) args[5],
                (size_t *) args[6]
            );
            break;
        case hotcall_ecall_add_flow:
            ecall_add_flow(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (struct cls_rule *) args[2],
                (struct cls_rule **) args[3],
                (struct cls_rule **) args[4],
                (struct match *) args[5],
                (uint32_t *) args[6],
                (uint16_t *) args[7],
                (uint16_t *) args[8],
                *(unsigned int *) args[9],
                *(uint16_t *) args[10],
                *(uint32_t *) args[11],
                (struct cls_rule **) args[12],
                *(int *) args[13],
                *(bool *) args[14],
                (uint16_t *) args[15],
                (int *) args[16],
                (unsigned int *) args[17]
            );
            break;
        case hotcall_ecall_collect_rules_loose:
            *(size_t *) return_value = ecall_collect_rules_loose(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                *(int *) args[2],
                *(size_t *) args[3],
                (struct match *) args[4],
                (struct cls_rule **) args[5],
                *(size_t *) args[6],
                *(ovs_be64 *) args[7],
                *(ovs_be64 *) args[8],
                *(uint16_t *) args[9],
                (bool *) args[10],
                (size_t *) args[11]
            );
            break;
        case hotcall_ecall_modify_flows_strict:
            *(size_t *) return_value = ecall_modify_flows_strict(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                *(int *) args[2],
                args[3],
                *(unsigned int *) args[4],
                *(ovs_be64 *) args[5],
                *(ovs_be64 *) args[6],
                *(uint16_t *) args[7],
                args[8],
                args[9],
                args[10],
                args[11],
                *(size_t *) args[12]
            );
            break;
        case hotcall_ecall_modify_flows_loose:
            *(size_t *) return_value = ecall_modify_flows_loose(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                *(int *) args[2],
                *(size_t *) args[3],
                args[4],
                args[5],
                *(size_t *) args[6],
                *(ovs_be64 *) args[7],
                *(ovs_be64 *) args[8],
                *(uint16_t *) args[9],
                args[10],
                args[11],
                args[12],
                args[13]
            );
            break;
        case hotcall_ecall_collect_rules_strict:
            *(size_t *) return_value = ecall_collect_rules_strict(
                *(int *) args[0],
                *(int *) args[1],
                *(int *) args[2],
                (struct match *) args[3],
                *(unsigned int *) args[4],
                *(ovs_be64 *) args[5],
                *(ovs_be64 *) args[6],
                *(uint16_t *) args[7],
                (struct cls_rule **) args[8],
                (bool *) args[9],
                *(size_t *) args[10]
            );
            break;
        case hotcall_ecall_delete_flows_loose:
            *(size_t *) return_value = ecall_delete_flows_loose(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                *(int *) args[2],
                *(size_t *) args[3],
                args[4],
                *(ovs_be64 *) args[5],
                *(ovs_be64 *) args[6],
                *(uint16_t *) args[7],
                args[8],
                args[9],
                args[10],
                args[11],
                *(size_t *) args[12],
                args[13],
                args[14],
                args[15],
                args[16]
            );
            break;
        case hotcall_ecall_delete_flows_strict:
            *(size_t *) return_value = ecall_delete_flows_strict(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                *(int *) args[2],
                args[3],
                *(unsigned int *) args[4],
                *(ovs_be64 *) args[5],
                *(ovs_be64 *) args[6],
                *(uint16_t *) args[7],
                args[8],
                args[9],
                args[10],
                args[11],
                args[12],
                args[13],
                args[14],
                *(size_t *) args[15]
            );
            break;
        case hotcall_ecall_oftable_configure:
            ecall_oftable_configure(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                (char *) args[2],
                *(unsigned int *) args[3],
                (struct mf_subfield *) args[4],
                *(size_t *) args[5],
                *(uint32_t *) args[6],
                (bool *) args[7],
                (bool *) args[8]
            );
            break;
        case hotcall_ecall_need_to_evict:
            *(bool *) return_value = ecall_need_to_evict(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1]
            );
            break;
        case hotcall_ecall_collect_rules_loose_stats_request:
            *(size_t *) return_value = ecall_collect_rules_loose_stats_request(
                *(uint8_t *) args[0],
                *(uint8_t *) args[1],
                *(int *) args[2],
                *(size_t *) args[3],
                *(size_t *) args[4],
                (struct match *) args[5],
                (struct cls_rule **) args[6],
                (struct match *) args[7],
                (unsigned int *) args[8],
                (size_t *) args[9]
            );
            break;
        case hotcall_ecall_ofproto_rule_send_removed:
            ecall_ofproto_rule_send_removed(
                *(uint8_t *) args[0],
                (struct cls_rule *) args[1],
                (struct match *) args[2],
                (unsigned int *) args[3],
                (bool *) args[4]
            );
            break;
        case hotcall_ecall_oftable_remove_rules:
        {
            struct cls_rule ** ut_crs = (struct cls_rule **) args[2];
            size_t n = *(size_t *) args[4];
            ecall_oftable_remove_rules(
                *(uint8_t *) args[0],
                (uint8_t *) args[1],
                (struct cls_rule **) args[2],
                (bool *) args[3],
                *(size_t *) args[4]
             );
            for (size_t i = 0; i < n; ++i) {
                //flow_map_cache_remove_ut_cr(flow_cache, ut_crs[i]);
            }
            break;
        }
        case hotcall_ecall_minimatch_expand_and_get_priority:
            ecall_minimatch_expand_and_get_priority(
                *(uint8_t *) args[0],
                (struct cls_rule *) args[1],
                (struct match *) args[2],
                (unsigned int *) args[3]
            );
            break;
        case hotcall_ecall_miniflow_expand_and_tag:
            ecall_miniflow_expand_and_tag(
                *(uint8_t *) args[0],
                (struct cls_rule *) args[1],
                (struct flow *) args[2],
                *(uint8_t *) args[3]
            );
            break;

        default:
            printf("Error, no matching switch case for %d in call table.\n", function_id);
    }
} /* execute_function */
