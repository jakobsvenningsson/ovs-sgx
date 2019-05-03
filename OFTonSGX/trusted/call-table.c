#include "call-table.h"
#include "ofproto-provider.h"
#include "enclave_t.h"
#include "common.h"
#include "enclave.h"
#include "cache-trusted.h"
#include "flow.h"


void
execute_function(int function, argument_list * args, void *ret, flow_map_cache *flow_cache){
    cls_cache_entry *cache_entry, *next;

    switch (function) {
        case hotcall_ecall_istable_readonly:
            *((int *) ret) = ecall_istable_readonly(*((int *) args->args[0]), *((uint8_t *) args->args[1]));
            break;
        case hotcall_ecall_cls_rule_init:
                ecall_cls_rule_init(*((int *) args->args[0]), (struct cls_rule *) args->args[1],
          (const struct match *) args->args[2], *((unsigned int *) args->args[3]));
            break;
        case hotcall_ecall_cls_rule_destroy:
            {
                struct cls_rule *ut_cr = (struct cls_rule *) args->args[1];
                ecall_cls_rule_destroy(*((int *) args->args[0]), ut_cr);
                flow_map_cache_remove_ut_cr(flow_cache, ut_cr);
                break;
            }
        case hotcall_ecall_cr_rule_overlaps:
            *((int *) ret) = ecall_cr_rule_overlaps(*((int *) args->args[0]), *((int *) args->args[1]), (struct cls_rule *) args->args[2]);
            break;
        case hotcall_ecall_readonly_set:
            ecall_readonly_set(*((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_oftable_set_name:
            ecall_oftable_set_name(*((int *) args->args[0]), *((int *) args->args[1]), (char *) args->args[2]);
            break;
        case hotcall_ecall_oftable_disable_eviction:
            ecall_oftable_disable_eviction(*((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_table_mflows_set:
            ecall_table_mflows_set(*((int *) args->args[0]), *((int *) args->args[1]), *((unsigned int *) args->args[2]));
            break;
        case hotcall_ecall_cls_count:
            *((int *) ret) = ecall_cls_count(*((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_table_mflows:
            *((unsigned int *) ret) = ecall_table_mflows(*((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_eviction_fields_enable:
            *((int *) ret) = ecall_eviction_fields_enable(*((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_fet_ccfes_c:
            *((int *) ret) = ecall_fet_ccfes_c(*((int *) args->args[0]));
            break;
        case hotcall_ecall_fet_ccfes_r:
            ecall_fet_ccfes_r(*((int *) args->args[0]), (struct cls_rule **) args->args[1], *((int *) args->args[2]));
            break;
        case hotcall_ecall_ofproto_destroy:
            ecall_ofproto_destroy(*((int *) args->args[0]));
            break;
        case hotcall_ecall_total_rules:
            *((unsigned int *) ret) = ecall_total_rules(*((int *) args->args[0]));
            break;
        case hotcall_ecall_cls_find_match_exactly:
            ecall_cls_find_match_exactly(*((int *) args->args[0]), *((int *) args->args[1]), (const struct match *) args->args[2],
          *((unsigned int *) args->args[3]), (struct cls_rule **) args->args[4]);
            break;
        case hotcall_ecall_cr_priority:
            *((unsigned int *) ret) = ecall_cr_priority(*((int *) args->args[0]), (struct cls_rule *) args->args[1]);
            break;
        case hotcall_ecall_rule_get_flags:
            *((enum oftable_flags *) ret) = ecall_rule_get_flags(*((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_table_name:
            ecall_table_name(*((int *) args->args[0]), *((int *) args->args[1]), (char *) args->args[2], *((size_t *) args->args[3]));
            break;
        case hotcall_ecall_femt_ccfe_c:
            *((int *) ret) = ecall_femt_ccfe_c( *((int *) args->args[0]), *((int *) args->args[1]), *((uint8_t *) args->args[2]), (const struct match *) args->args[3]);
            break;
        case hotcall_ecall_femt_ccfe_r:
            ecall_femt_ccfe_r(*((int *) args->args[0]), *((int *) args->args[1]),
          (struct cls_rule **) args->args[2],
          *((int *) args->args[3]),
          *((uint8_t *) args->args[4]),
          (const struct match *) args->args[5]);
            break;
        case hotcall_ecall_minimatch_expand:
            ecall_minimatch_expand(*((int *) args->args[0]), (struct cls_rule *) args->args[1], (struct match *) args->args[2]);
            break;
        case hotcall_ecall_cls_rule_format:
            *((unsigned int *) ret) = ecall_cls_rule_format(*((int *) args->args[0]), (const struct cls_rule *) args->args[1],
          (struct match *) args->args[2]);
            break;
        case hotcall_ecall_fet_ccfe_c:
            *((int *) ret) = ecall_fet_ccfe_c(*((int *) args->args[0]));
            break;
        case hotcall_ecall_fet_ccfe_r:
            ecall_fet_ccfe_r(*((int *) args->args[0]), (struct cls_rule **) args->args[1], *((int *) args->args[2]));
            break;
        case hotcall_ecall_cls_rule_hash:
            *((uint32_t *) ret) = ecall_cls_rule_hash(*((int *) args->args[0]), (const struct cls_rule *) args->args[1], *((uint32_t *) args->args[2]));
            break;
        case hotcall_ecall_cls_rule_equal:
            *((int *) ret) = ecall_cls_rule_equal(*((int *) args->args[0]), (const struct cls_rule *) args->args[1],
          (const struct cls_rule *) args->args[2]);
            break;
        case hotcall_ecall_choose_rule_to_evict:
            ecall_choose_rule_to_evict(*((int *) args->args[0]), *((int *) args->args[1]), (struct cls_rule **) args->args[2]);
            break;
        case hotcall_ecall_choose_rule_to_evict_p:
            *((struct cls_rule **) ret) = ecall_choose_rule_to_evict_p(*((int *) args->args[0]), *((int *) args->args[1]), (struct cls_rule **) args->args[2], (struct cls_rule *) args->args[3]);
            break;
        case hotcall_ecall_collect_ofmonitor_util_c:
             *((int *) ret) = ecall_collect_ofmonitor_util_c(*((int *) args->args[0]), *((int *) args->args[1]), *((int *) args->args[2]),
          (const struct minimatch *) args->args[3]);
            break;
        case hotcall_ecall_collect_ofmonitor_util_r:
            ecall_collect_ofmonitor_util_r(*((int *) args->args[0]), *((int *) args->args[1]), (struct cls_rule **) args->args[2], *((int *) args->args[3]),
          *((int *) args->args[4]),
          (const struct minimatch *) args->args[5]);
            break;
        case hotcall_ecall_cls_rule_is_loose_match:
            *((int *) ret) = ecall_cls_rule_is_loose_match(*((int *) args->args[0]), (struct cls_rule *) args->args[1], (const struct minimatch *) args->args[2]);
            break;
        case hotcall_ecall_minimask_get_vid_mask:
            *((uint16_t *) ret) = ecall_minimask_get_vid_mask(*((int *) args->args[0]), (struct cls_rule *) args->args[1]);
            break;
        case hotcall_ecall_miniflow_get_vid:
            *((uint16_t *) ret) = ecall_miniflow_get_vid(*((int *) args->args[0]), (struct cls_rule *) args->args[1]);
            break;
        case hotcall_ecall_evg_group_resize:
            ecall_evg_group_resize(*((int *) args->args[0]), *((int *) args->args[1]), (struct cls_rule *) args->args[2], *((size_t *) args->args[3]), (struct eviction_group *) args->args[4]);
            break;
        case hotcall_ecall_evg_add_rule:
            *((size_t *) ret) = ecall_evg_add_rule(*((int *) args->args[0]), *((int *) args->args[1]),
          (struct cls_rule *) args->args[2],
          *((uint32_t *) args->args[3]),
          *((uint32_t *) args->args[4]),
          *((struct heap_node *) args->args[5]));
            break;
        case hotcall_ecall_oftable_enable_eviction:
            ecall_oftable_enable_eviction(*((int *) args->args[0]), *((int *) args->args[1]),
          (const struct mf_subfield *) args->args[2],
          *((size_t *) args->args[3]),
          *((uint32_t *) args->args[4]), (bool *) args->args[5]);
            break;
        case hotcall_ecall_ccfe_c:
            *((int *) ret) = ecall_ccfe_c(*((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_ccfe_r:
            ecall_ccfe_r(*((int *) args->args[0]), (struct cls_rule **) args->args[1],
          *((int *) args->args[2]),
          *((int *) args->args[3]));
            break;
        case hotcall_ecall_cls_remove:
            {
                int bridge_id = *((int *) args->args[0]);
                int table_id = *((int *) args->args[1]);
                struct cls_rule *ut_cr = (struct cls_rule *) args->args[2];

                ecall_cls_remove(bridge_id, table_id, ut_cr);
                flow_map_cache_remove_ut_cr(flow_cache, ut_cr);
                break;
            }
        case hotcall_ecall_classifier_replace:
            {

                int bridge_id = *((int *) args->args[0]);
                int table_id = *((int *) args->args[1]);
                struct cls_rule *ut_cr_insert = (struct cls_rule *) args->args[2];
                struct cls_rule **ut_cr_remove = (struct cls_rule **) args->args[3];

                ecall_classifier_replace(bridge_id, table_id, ut_cr_insert, ut_cr_remove);

                if(!*ut_cr_remove) {
                    return;
                }
                flow_map_cache_remove_ut_cr(flow_cache, *ut_cr_remove);
                break;
            }
        case hotcall_ecall_ofproto_get_vlan_c:
            *((int *) ret) = ecall_ofproto_get_vlan_c(*((int *) args->args[0]));
            break;
        case hotcall_ecall_ofproto_get_vlan_r:
            ecall_ofproto_get_vlan_r(*((int *) args->args[0]), (uint16_t *) args->args[1],
          *((int *) args->args[2]));
            break;
        case hotcall_ecall_femt_c:
            *((int *) ret) = ecall_femt_c(*((int *) args->args[0]), *((int *) args->args[1]),
          *((uint8_t *) args->args[2]),
          (const struct match *) args->args[3], *((unsigned int *) args->args[4]));
            break;
        case hotcall_ecall_femt_r:
            ecall_femt_r(*((int *) args->args[0]), *((int *) args->args[1]),
          (struct cls_rule **) args->args[2],
          *((int *) args->args[3]),
          *((uint8_t *) args->args[4]),
          (const struct match *) args->args[5],
          *((unsigned int *) args->args[6]));
            break;
        case hotcall_ecall_hidden_tables_check:
            ecall_hidden_tables_check(*((int *) args->args[0]));
            break;
        case hotcall_ecall_miniflow_expand:
            ecall_miniflow_expand(*((int *) args->args[0]), (struct cls_rule *) args->args[1], (struct flow *) args->args[2]);
            break;
        case hotcall_ecall_rule_calculate_tag:
            *((uint32_t *) ret) = ecall_rule_calculate_tag(*((int *) args->args[0]), (struct cls_rule *) args->args[1], (const struct flow *) args->args[2],
          *((int *) args->args[3]));
            break;
        case hotcall_ecall_rule_calculate_tag_s:
            *((uint32_t *) ret) = ecall_rule_calculate_tag_s(*((int *) args->args[0]), *((int *) args->args[1]), (const struct flow *) args->args[2]);
            break;
        case hotcall_ecall_table_update_taggable:
            {
                int *ret_value = (int *) ret;
                *ret_value = ecall_table_update_taggable(*((int *) args->args[0]), *((uint8_t *) args->args[1]));
                if(*ret_value == 4) {
                    flow_map_cache_flush(flow_cache);
                }
            }
            break;
        case hotcall_ecall_is_sgx_other_table:
            *((int *) ret) = ecall_is_sgx_other_table(*((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_cls_lookup:
        {

            const struct flow *flow = (const struct flow *) args->args[3];
            struct flow_wildcards *wc = args->args[4];
            int table_id = *((int *) args->args[2]);
            int bridge_id = *((int *) args->args[0]);
            struct cls_rule **ut_cr = (struct cls_rule **) args->args[1];

            ecall_cls_lookup(bridge_id,
                            ut_cr,
                            table_id,
                            flow,
                            wc);

            if(*ut_cr) {
                flow_map_cache_insert(flow_cache, flow, wc, *ut_cr, bridge_id, table_id);
            }
            break;
        }

        case hotcall_ecall_evg_remove_rule:
            *((int *) ret) = ecall_evg_remove_rule(*((int *) args->args[0]), *((int *) args->args[1]),
          (struct cls_rule *) args->args[2]);
            break;
        case hotcall_ecall_cls_rule_priority:
            *((unsigned *) ret) = ecall_cls_rule_priority(*((int *) args->args[0]), (struct cls_rule *) args->args[1]);
            break;
        case hotcall_ecall_desfet_ccfes_c:
            *((int *) ret) = ecall_desfet_ccfes_c(*((int *) args->args[0]));
            break;
        case hotcall_ecall_desfet_ccfes_r:
            ecall_desfet_ccfes_r(*((int *) args->args[0]), (struct cls_rule **) args->args[1], *((int *) args->args[2]));
            break;
        /*case hotcall_ecall_table_dpif_init:
            ecall_table_dpif_init(*((int *) args->args[0]));
            break;*/
        case hotcall_ecall_get_cls_rules:
            *((size_t *) ret) = ecall_get_cls_rules(*((int *) args->args[0]),
                                                    *((int *) args->args[1]),
                                                    *((size_t *) args->args[2]),
                                                    *((size_t *) args->args[3]),
                                                    (struct cls_rule **) args->args[4],
                                                    *((size_t *) args->args[5]),
                                                    (size_t *) args->args[6]);
            break;
        case hotcall_ecall_get_cls_rules_and_enable_eviction:
            *((size_t *) ret) = ecall_get_cls_rules_and_enable_eviction(*((int *) args->args[0]),
                                                                        *((int *) args->args[1]),
                                                                        *((size_t *) args->args[2]),
                                                                        *((size_t *) args->args[3]),
                                                                        (struct cls_rule **) args->args[4],
                                                                        *((size_t *) args->args[5]),
                                                                        (size_t *) args->args[6],
                                                                        (const struct mf_subfield *) args->args[7],
                                                                        *((size_t *) args->args[8]),
                                                                        *((uint32_t *) args->args[9]),
                                                                        (bool *) args->args[10],
                                                                        (bool *) args->args[11]);
            break;
        case hotcall_ecall_eviction_group_add_rules:
            ecall_eviction_group_add_rules(*((int *) args->args[0]),
                                           *((int *) args->args[1]),
                                           *((size_t *) args->args[2]),
                                           (struct cls_rule **) args->args[3],
                                           (struct heap_node *) args->args[4],
                                           (uint32_t *) args->args[5],
                                           *((uint32_t *) args->args[6]));
            break;
        case hotcall_ecall_ofproto_get_vlan_usage:
            *((size_t *) ret) = ecall_ofproto_get_vlan_usage(*((int *) args->args[0]),
                                                             *((size_t *) args->args[1]),
                                                             (uint16_t *) args->args[2],
                                                             *((size_t *) args->args[3]),
                                                             *((size_t *) args->args[4]),
                                                             (size_t *) args->args[5]);
            break;
        case hotcall_ecall_ofproto_flush:
            *((size_t *) ret) = ecall_ofproto_flush(*((int *) args->args[0]),
                                                        (struct cls_rule **) args->args[1],
                                                        (uint32_t *) args->args[2],
                                                        *((size_t *) args->args[3]),
                                                        *((size_t *) args->args[4]),
                                                        *((size_t *) args->args[5]),
                                                        (size_t *) args->args[6]);
            break;
        case hotcall_ecall_ofproto_evict:
            *((size_t *) ret) = ecall_ofproto_evict(*((int *) args->args[0]),
                                                    *((int *) args->args[1]),
                                                    *((size_t *) args->args[2]),
                                                    (uint32_t *) args->args[3],
                                                    (struct cls_rule **) args->args[4],
                                                    *((size_t *) args->args[5]),
                                                    (size_t *) args->args[6]);
            break;
        case hotcall_ecall_add_flow:
            ecall_add_flow(*((int *) args->args[0]),
                           *((int *) args->args[1]),
                            (struct cls_rule *) args->args[2],
                            (struct cls_rule **) args->args[3],
                            (struct cls_rule **) args->args[4],
                            (struct match *) args->args[5],
                            (uint32_t *) args->args[6],
                            (uint16_t *) args->args[7],
                            (uint16_t *) args->args[8],
                            *((unsigned int *) args->args[9]),
                            *((uint16_t *) args->args[10]),
                            *((uint32_t *) args->args[11]),
                            *((uint32_t *) args->args[12]),
                            *((struct heap_node *) args->args[13]),
                            (struct cls_rule **) args->args[14],
                            *((int *) args->args[15]),
                            *((bool *) args->args[16]),
                            (bool *) args->args[17],
                            (bool *) args->args[18],
                            (bool *) args->args[19],
                            (bool *) args->args[20],
                            (bool *) args->args[21]);
            break;
        case hotcall_ecall_collect_rules_loose:
            *((size_t *) ret) = ecall_collect_rules_loose(
                *((int *) args->args[0]),
                *((int *) args->args[1]),
                *((int *) args->args[2]),
                *((size_t *) args->args[3]),
                (struct match *) args->args[4],
                (bool *) args->args[5],
                (struct cls_rule **) args->args[6],
                *((size_t *) args->args[7]),
                (bool *) args->args[8],
                (size_t *) args->args[9]
            );
            break;
        case hotcall_ecall_collect_rules_strict:
            *((size_t *) ret) = ecall_collect_rules_strict(
                *((int *) args->args[0]),
                *((int *) args->args[1]),
                *((int *) args->args[2]),
                (struct match *) args->args[3],
                *((unsigned int *) args->args[4]),
                (bool *) args->args[5],
                (struct cls_rule **) args->args[6],
                (bool *) args->args[7],
                *((size_t *) args->args[8])
            );
            break;
        case hotcall_ecall_delete_flows:
            ecall_delete_flows(
                *((int *) args->args[0]),
                (int *) args->args[1],
                (struct cls_rule **) args->args[2],
                (bool *) args->args[3],
                (uint32_t *) args->args[4],
                (unsigned int *) args->args[5],
                (struct match *) args->args[6],
                *((size_t *) args->args[7])
            );
            break;
        case hotcall_ecall_configure_table:
            ecall_configure_table(
                *((int *) args->args[0]),
                *((int *) args->args[1]),
                (char *) args->args[2],
                *((unsigned int *) args->args[3]),
                (struct mf_subfield *) args->args[4],
                *((size_t *) args->args[5]),
                (bool *) args->args[6],
                (bool *) args->args[7]
            );
            break;
        case hotcall_ecall_need_to_evict:
            *((bool *) ret) = ecall_need_to_evict(
                *((int *) args->args[0]),
                *((int *) args->args[1])
            );
            break;
        case hotcall_ecall_collect_rules_loose_stats_request:
            *((size_t *) ret) = ecall_collect_rules_loose_stats_request(
                *((int *) args->args[0]),
                *((int *) args->args[1]),
                *((int *) args->args[2]),
                *((size_t *) args->args[3]),
                *((size_t *) args->args[4]),
                (struct match *) args->args[5],
                (struct cls_rule **) args->args[6],
                (struct match *) args->args[7],
                (unsigned int *) args->args[8],
                (size_t *) args->args[9]
            );
            break;
        case hotcall_ecall_ofproto_rule_send_removed:
            ecall_ofproto_rule_send_removed(
                *((int *) args->args[0]),
                (struct cls_rule *) args->args[1],
                (struct match *) args->args[2],
                (unsigned int *) args->args[3],
                (bool *) args->args[4]
            );
            break;
        case hotcall_ecall_remove_rules:
            *((size_t *) ret) = ecall_remove_rules(
                *((int *) args->args[0]),
                (int *) args->args[1],
                (struct cls_rule **) args->args[2],
                (bool *) args->args[3],
                *((size_t *) args->args[4])
            );
            break;
        /*case hotcall_ecall_ofproto_evict_get_rest:
            ecall_ofproto_evict_get_rest(
                (uint32_t *) args->args[0],
                (struct cls_rule **) args->args[1],
                *((size_t *) args->args[2])
            );
            break;*/


        /*case ecall_destroy_rule_if_overlaps:
            ecall_destroy_rule_if_overlaps(*((int *) args->args[1]), (struct cls_rule *) args->args[2]);
            break;
        case ecall_get_rule_to_evict_if_neccesary:
            *((bool *) ret) = ecall_get_rule_to_evict_if_neccesary(*((int *) args->args[1]),
          (struct cls_rule *) args->args[2]);
            break;
        case ecall_miniflow_expand_and_tag:
            *((uint32_t *) ret) =
          ecall_miniflow_expand_and_tag((struct cls_rule *) args->args[1], (struct flow *) args->args[2],
          *((int *) args->args[3]));
            break;
        case ecall_allocate_cls_rule_if_not_read_only:
            *((bool *) ret) =
          ecall_allocate_cls_rule_if_not_read_only(*((int *) args->args[1]), (struct cls_rule *) args->args[2],
          (struct match *) args->args[3],
          *((unsigned int *) args->args[4]));
            break;
        case ecall_classifier_replace_if_modifiable:
            ecall_classifer_replace_if_modifiable(*((int *) args->args[1]),
          (struct cls_rule *) args->args[2],
          (struct cls_rule **) args->args[3],
          (bool *) args->args[4]);
            break;
        case ecall_ofproto_configure_table:
            *((bool *) ret) = ecall_ofproto_configure_table(
          *((int *) args->args[1]),
          (struct mf_subfield *) args->args[2],
          (char *) args->args[3],
          *((unsigned int *) args->args[4]),
          *((unsigned int *) args->args[5]),
          *((unsigned int *) args->args[6]),
          (unsigned int *) args->args[7],
          (struct cls_rule *) args->args[8],
          (bool *) args->args[9]);

            break;*/

        default:
            printf("Error, no matching switch case for %d.\n", function);
    }
} /* execute_function */
