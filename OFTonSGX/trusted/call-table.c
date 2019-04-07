#include "call-table.h"
#include "ofproto-provider.h"

void
execute_function(int function, argument_list * args, void *ret){
    switch (function) {
        case hotcall_ecall_istable_readonly:
            ecall_istable_readonly((int *) ret, *((int *) args->args[0]), *((uint8_t *) args->args[1]));
            break;
        case hotcall_ecall_cls_rule_init:
                ecall_cls_rule_init(*((int *) args->args[0]), (struct cls_rule *) args->args[1],
          (const struct match *) args->args[2], *((unsigned int *) args->args[3]));
            break;
        case hotcall_ecall_cls_rule_destroy:
            ecall_cls_rule_destroy(*((int *) args->args[0]), (struct cls_rule *) args->args[1]);
            break;
        case hotcall_ecall_cr_rule_overlaps:
            ecall_cr_rule_overlaps((int *) ret, *((int *) args->args[0]), *((int *) args->args[1]), (struct cls_rule *) args->args[2]);
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
            ecall_cls_count((int *) ret,*((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_table_mflows:
            ecall_table_mflows((unsigned int *) ret, *((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_eviction_fields_enable:
            ecall_eviction_fields_enable((int *) ret, *((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_fet_ccfes_c:
            ecall_fet_ccfes_c((int *) ret, *((int *) args->args[0]));
            break;
        case hotcall_ecall_fet_ccfes_r:
            ecall_fet_ccfes_r(*((int *) args->args[0]), (struct cls_rule **) args->args[1], *((int *) args->args[2]));
            break;
        case hotcall_ecall_ofproto_destroy:
            ecall_ofproto_destroy(*((int *) args->args[0]));
            break;
        case hotcall_ecall_total_rules:
            ecall_total_rules((unsigned int *) ret, *((int *) args->args[0]));
            break;
        case hotcall_ecall_cls_find_match_exactly:
            ecall_cls_find_match_exactly(*((int *) args->args[0]), *((int *) args->args[1]), (const struct match *) args->args[2],
          *((unsigned int *) args->args[3]), (struct cls_rule **) args->args[4]);
            break;
        case hotcall_ecall_cr_priority:
            ecall_cr_priority((unsigned int *) ret, *((int *) args->args[0]), (struct cls_rule *) args->args[1]);
            break;
        case hotcall_ecall_rule_get_flags:
            ecall_rule_get_flags((enum oftable_flags *) ret, *((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_table_name:
            ecall_table_name(*((int *) args->args[0]), *((int *) args->args[1]), (char *) args->args[2], *((size_t *) args->args[1]));
            break;
        case hotcall_ecall_femt_ccfe_c:
            ecall_femt_ccfe_c((int *) ret, *((int *) args->args[0]), *((int *) args->args[1]), *((uint8_t *) args->args[2]), (const struct match *) args->args[3]);
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
            ecall_cls_rule_format((unsigned int *) ret, *((int *) args->args[0]), (const struct cls_rule *) args->args[1],
          (struct match *) args->args[2]);
            break;
        case hotcall_ecall_fet_ccfe_c:
            ecall_fet_ccfe_c((int *) ret, *((int *) args->args[0]));
            break;
        case hotcall_ecall_fet_ccfe_r:
            ecall_fet_ccfe_r(*((int *) args->args[0]), (struct cls_rule **) args->args[1], *((int *) args->args[2]));
            break;
        case hotcall_ecall_cls_rule_hash:
            ecall_cls_rule_hash((uint32_t *) ret, *((int *) args->args[0]), (const struct cls_rule *) args->args[1], *((uint32_t *) args->args[2]));
            break;
        case hotcall_ecall_cls_rule_equal:
            ecall_cls_rule_equal((int *) ret, *((int *) args->args[0]), (const struct cls_rule *) args->args[1],
          (const struct cls_rule *) args->args[2]);
            break;
        case hotcall_ecall_choose_rule_to_evict:
            ecall_choose_rule_to_evict(*((int *) args->args[0]), *((int *) args->args[1]), (struct cls_rule *) args->args[2]);
            break;
        case hotcall_ecall_choose_rule_to_evict_p:
            ecall_choose_rule_to_evict_p(((struct cls_rule **) ret), *((int *) args->args[0]), *((int *) args->args[1]), (struct cls_rule **) args->args[2], (struct cls_rule *) args->args[3]);
            break;
        case hotcall_ecall_collect_ofmonitor_util_c:
             ecall_collect_ofmonitor_util_c((int *) ret, *((int *) args->args[0]), *((int *) args->args[1]), *((int *) args->args[2]),
          (const struct minimatch *) args->args[3]);
            break;
        case hotcall_ecall_collect_ofmonitor_util_r:
            ecall_collect_ofmonitor_util_r(*((int *) args->args[0]), *((int *) args->args[1]), (struct cls_rule **) args->args[2], *((int *) args->args[3]),
          *((int *) args->args[4]),
          (const struct minimatch *) args->args[5]);
            break;
        case hotcall_ecall_cls_rule_is_loose_match:
            ecall_cls_rule_is_loose_match((int *) ret, *((int *) args->args[0]), (struct cls_rule *) args->args[1], (const struct minimatch *) args->args[2]);
            break;
        case hotcall_ecall_minimask_get_vid_mask:
            ecall_minimask_get_vid_mask((uint16_t *) ret, *((int *) args->args[0]), (struct cls_rule *) args->args[1]);
            break;
        case hotcall_ecall_miniflow_get_vid:
            ecall_miniflow_get_vid((uint16_t *) ret, *((int *) args->args[0]), (struct cls_rule *) args->args[1]);
            break;
        case hotcall_ecall_evg_group_resize:
            ecall_evg_group_resize(*((int *) args->args[0]), *((int *) args->args[1]), (struct cls_rule *) args->args[2], *((size_t *) args->args[3]));
            break;
        case hotcall_ecall_evg_add_rule:
            ecall_evg_add_rule((size_t *) ret, *((int *) args->args[0]), *((int *) args->args[1]),
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
            ecall_ccfe_c((int *) ret, *((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_ccfe_r:
            ecall_ccfe_r(*((int *) args->args[0]), (struct cls_rule **) args->args[1],
          *((int *) args->args[2]),
          *((int *) args->args[3]));
            break;
        case hotcall_ecall_cls_remove:
            ecall_cls_remove(*((int *) args->args[0]), *((int *) args->args[1]), (struct cls_rule *) args->args[2]);
            break;
        case hotcall_ecall_classifier_replace:
            ecall_classifier_replace(*((int *) args->args[0]), *((int *) args->args[1]),
          (struct cls_rule *) args->args[2],
          (struct cls_rule **) args->args[3]);
            break;
        case hotcall_ecall_ofproto_get_vlan_c:
            ecall_ofproto_get_vlan_c((int *) ret, *((int *) args->args[0]));
            break;
        case hotcall_ecall_ofproto_get_vlan_r:
            ecall_ofproto_get_vlan_r(*((int *) args->args[0]), (uint16_t *) args->args[1],
          *((int *) args->args[2]));
            break;
        case hotcall_ecall_femt_c:
            ecall_femt_ccfe_c((int *) ret, *((int *) args->args[0]), *((int *) args->args[1]),
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
            ecall_rule_calculate_tag((uint32_t *) ret, *((int *) args->args[0]), (struct cls_rule *) args->args[1], (const struct flow *) args->args[2],
          *((int *) args->args[3]));
            break;
        case hotcall_ecall_table_update_taggable:
            ecall_table_update_taggable((int *) ret, *((int *) args->args[0]), *((uint8_t *) args->args[1]));
            break;
        case hotcall_ecall_is_sgx_other_table:
            ecall_is_sgx_other_table((int *) ret, *((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_cls_lookup:
            ecall_cls_lookup(*((int *) args->args[0]), (struct cls_rule **) args->args[1], *((int *) args->args[2]), (const struct flow *) args->args[3],
          (struct flow_wildcards *) args->args[4]);
            break;
        case hotcall_ecall_sgx_table_dpif:
            ecall_sgx_table_dpif(*((int *) args->args[0]), *((int *) args->args[1]));
            break;
        case hotcall_ecall_evg_remove_rule:
            ecall_evg_remove_rule((int *) ret, *((int *) args->args[0]), *((int *) args->args[1]),
          (struct cls_rule *) args->args[2]);
            break;
        case hotcall_ecall_cls_rule_priority:
            ecall_cls_rule_priority((unsigned int *) ret, *((int *) args->args[0]), (struct cls_rule *) args->args[1]);
            break;
        case hotcall_ecall_desfet_ccfes_c:
            ecall_desfet_ccfes_c((int *) ret, *((int *) args->args[0]));
            break;
        case hotcall_ecall_desfet_ccfes_r:
            ecall_desfet_ccfes_r(*((int *) args->args[0]), (struct cls_rule **) args->args[1], *((int *) args->args[2]));
            break;
        case hotcall_ecall_table_dpif_init:
            ecall_table_dpif_init(*((int *) args->args[0]));
            break;
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
