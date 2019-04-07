#include <string.h>
#include <assert.h>
#include <libgen.h>
#include "sgx-utils.h"
#include "enclave_u.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "app.h"
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "spinlock.h"
#include "common.h"
#include "hotcall-producer.h"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid        = 0;
static bool enclave_is_initialized = false;
static int bridge_counter = 0;
//    printf("done %s\n", #f)
//    printf("%s\n", #f);
#ifdef HOTCALL
# define ECALL(f, has_return, n_args, args ...) \
    argument_list arg_list; \
    void * return_val; \
    compile_arg_list(&return_val, &arg_list, has_return, n_args, args); \
    make_hotcall(&ctx, hotcall_ ## f, &arg_list, return_val)
    # define CAST(X) &X
#else
# define ECALL(f, has_return, n_args, args ...) \
    f(global_eid, args);
# define CAST(X) X
#endif /* ifdef HOTCALL */


void *
ecall_polling_thread(void * vargp){
    printf("Running polling thread.\n");
    int ecall_return;
    MAKE_ECALL_ARGS(start_poller, &ecall_return, global_eid, &ctx);
    if (ecall_return == 0) {
        printf("Application ran with success\n");
    } else {
        printf("Application failed %d \n", ecall_return);
    }
}

// 1. Creation and Initialization of tables
int
sgx_ofproto_init_tables(int n_tables){
    printf("Inside FLOW TABLE ENCLAVE.\n");
    int ecall_return;
    if (!enclave_is_initialized) {
        if (initialize_enclave(&global_eid) < 0) {
            return -1;
        }
        enclave_is_initialized = true;

        #ifdef HOTCALL
        printf("HOTCALLS ENABLED STARTING THREAD.\n");
        pthread_t thread_id;

        pthread_create(&thread_id, NULL, ecall_polling_thread, NULL);
        #else
        puts("NO HOTCALLS.");
        #endif

        #ifdef TIMEOUT
        puts("TIMEOUT ENABLED\n");
        #endif
    }
    int this_bridge_id = bridge_counter++;
    ecall_ofproto_init_tables(global_eid, this_bridge_id, n_tables);
    return this_bridge_id;
}

void
SGX_readonly_set(int bridge_id, int table_id){
    ECALL(ecall_readonly_set, false, 2, CAST(bridge_id), CAST(table_id));
}

int
SGX_istable_readonly(int bridge_id, uint8_t table_id){
    int ecall_return;
    ECALL(ecall_istable_readonly, true, 2, &ecall_return, CAST(bridge_id), CAST(table_id));
    return ecall_return;
}

void
SGX_cls_rule_init(int bridge_id, struct cls_rule * o_cls_rule,
  const struct match * match, unsigned int priority){
    ECALL(ecall_cls_rule_init, false, 4, CAST(bridge_id), o_cls_rule, match, CAST(priority));
}

// 4. SGX Classifier_rule_overlap
int
SGX_cr_rule_overlaps(int bridge_id, int table_id, struct cls_rule * o_cls_rule){
    int ecall_return;
    ECALL(ecall_cr_rule_overlaps, true, 3, &ecall_return, CAST(bridge_id), CAST(table_id), o_cls_rule);
    return ecall_return;
}

// 5. SGX_CLS_RULE_DESTROY
void
SGX_cls_rule_destroy(int bridge_id, struct cls_rule * o_cls_rule){
    ECALL(ecall_cls_rule_destroy, false, 2, CAST(bridge_id), o_cls_rule);
}

// 6. cls_rule_hash
uint32_t
SGX_cls_rule_hash(int bridge_id, const struct cls_rule * o_cls_rule, uint32_t basis){
    uint32_t ecall_return;
    ECALL(ecall_cls_rule_hash, true, 3, &ecall_return, CAST(bridge_id), o_cls_rule, CAST(basis));
    return ecall_return;
}

// 7. cls_rule_equal
int
SGX_cls_rule_equal(int bridge_id, const struct cls_rule * o_cls_rule_a,
  const struct cls_rule * o_cls_rule_b){
    int ecall_return;
    ECALL(ecall_cls_rule_equal, true, 3, &ecall_return, CAST(bridge_id), o_cls_rule_a, o_cls_rule_b);
    return ecall_return;
}

// 8. classifier_replace
void
SGX_classifier_replace(int bridge_id, int table_id, struct cls_rule * o_cls_rule, struct cls_rule ** cls_rule_rtrn){
    ECALL(ecall_classifier_replace, false, 4, CAST(bridge_id), CAST(table_id), o_cls_rule, cls_rule_rtrn);
}

// 9 rule_get_flags
enum oftable_flags
SGX_rule_get_flags(int bridge_id, int table_id){
    enum oftable_flags ecall_return;
    ECALL(ecall_rule_get_flags, true, 2, &ecall_return, CAST(bridge_id), CAST(table_id));
    return ecall_return;
}

// 10. classifier count of cls_rules
int
SGX_cls_count(int bridge_id, int table_id){
    int ecall_return;
    ECALL(ecall_cls_count, true, 2, &ecall_return, CAST(bridge_id), CAST(table_id));
    return ecall_return;
}

// 11. is eviction_fields in the table with table_id enabled?
int
SGX_eviction_fields_enable(int bridge_id, int table_id){
    int ecall_return;
    ECALL(ecall_eviction_fields_enable, true, 2, &ecall_return, CAST(bridge_id), CAST(table_id));
    return ecall_return;
}

// 12.Add a rule to a eviction group
size_t
SGX_evg_add_rule(int bridge_id, int table_id, struct cls_rule * o_cls_rule, uint32_t priority,
  uint32_t rule_evict_prioriy, struct heap_node rule_evg_node){
    size_t ecall_return;
    ECALL(ecall_evg_add_rule, true, 6, &ecall_return, CAST(bridge_id), CAST(table_id), o_cls_rule, CAST(priority),
      CAST(rule_evict_prioriy), CAST(rule_evg_node));
    return ecall_return;
}

// 13. void ecall_evg_group_resize
void
SGX_evg_group_resize(int bridge_id, int table_id, struct cls_rule * o_cls_rule, size_t priority,
  struct eviction_group * evg){
    ECALL(ecall_evg_group_resize, false, 5, CAST(bridge_id), CAST(table_id), o_cls_rule, CAST(priority), evg);
}

// 14. Remove the evict group where a rule belongs to
int
SGX_evg_remove_rule(int bridge_id, int table_id, struct cls_rule * o_cls_rule){
    int ecall_return;
    ECALL(ecall_evg_remove_rule, true, 3, &ecall_return, CAST(bridge_id), CAST(table_id), o_cls_rule);
    return ecall_return;
}

// 15. Removes a cls_rule from the classifier
void
SGX_cls_remove(int bridge_id, int table_id, struct cls_rule * o_cls_rule){
    ECALL(ecall_cls_remove, false, 3, CAST(bridge_id), CAST(table_id), o_cls_rule);
}

// 16. SGX choose a cls_rule to evict from table
void
SGX_choose_rule_to_evict(int bridge_id, int table_id, struct cls_rule ** o_cls_rule){
    ECALL(ecall_choose_rule_to_evict, false, 3, CAST(bridge_id), CAST(table_id), o_cls_rule);
}

// 17.
struct cls_rule *
SGX_choose_rule_to_evict_p(int bridge_id, int table_id, struct cls_rule ** o_cls_rule, struct cls_rule * replacer){
    struct cls_rule * tmp;
    ECALL(ecall_choose_rule_to_evict_p, true, 4, &tmp, CAST(bridge_id), CAST(table_id), o_cls_rule, replacer);
    return tmp;
}

// 18 returns table max flow
unsigned int
SGX_table_mflows(int bridge_id, int table_id){
    unsigned int ecall_return;
    ECALL(ecall_table_mflows, true, 2, &ecall_return, CAST(bridge_id), CAST(table_id));
    return ecall_return;
}

// 19 set table max flow to value
void
SGX_table_mflows_set(int bridge_id, int table_id, unsigned int value){
    ECALL(ecall_table_mflows_set, false, 3, CAST(bridge_id), CAST(table_id), CAST(value));
}

// 19 minimatch_expand
void
SGX_minimatch_expand(int bridge_id, struct cls_rule * o_cls_rule, struct match * dst){
    ECALL(ecall_minimatch_expand, false, 3, CAST(bridge_id), o_cls_rule, dst);
}

// 20. cls_rule priority
unsigned int
SGX_cr_priority(int bridge_id, const struct cls_rule * o_cls_rule){
    unsigned ecall_return;
    ECALL(ecall_cr_priority, true, 2, &ecall_return, CAST(bridge_id), o_cls_rule);
    return ecall_return;
}

// 21  classifier find match exactly
void
SGX_cls_find_match_exactly(int bridge_id, int table_id,
  const struct match * target,
  unsigned int priority, struct cls_rule ** o_cls_rule){
    ECALL(ecall_cls_find_match_exactly, false, 5, CAST(bridge_id), CAST(table_id), target, CAST(priority), o_cls_rule);
}

// 22. SGX FOR_EACH_MATCHING_TABLE + CLS_CURSOR_FOR_EACH (count and request

// 22.1 Count
int
SGX_femt_ccfe_c(int bridge_id, int ofproto_n_tables, uint8_t table_id, const struct match * match){
    int ecall_return;
    ECALL(ecall_femt_ccfe_c, true, 4, &ecall_return, CAST(bridge_id), CAST(ofproto_n_tables), CAST(table_id), match);
    return ecall_return;
}

// 22.2 Request
void
SGX_femt_ccfe_r(int bridge_id, int ofproto_n_tables, struct cls_rule ** buf, int elem, uint8_t table_id,
  const struct match * match){
    ECALL(ecall_femt_ccfe_r, false, 6, CAST(bridge_id), CAST(ofproto_n_tables), buf, CAST(elem), CAST(table_id), match);
}

// 23. SGX FOR_EACH_MATCHING_TABLE get the rules

// 23.1 Count
int
SGX_ecall_femt_c(int bridge_id, int ofproto_n_tables, uint8_t table_id, const struct match * match,
  unsigned int priority){
    int buf_size;
    ECALL(ecall_femt_c, true, 5, &buf_size, CAST(bridge_id), CAST(ofproto_n_tables), CAST(table_id), match,
      CAST(priority));
    return buf_size;
}

// 23.2 Request
void
SGX_ecall_femt_r(int bridge_id, int ofproto_n_tables, struct cls_rule ** buf, int elem, uint8_t table_id,
  const struct match * match,
  unsigned int priority){
    ECALL(ecall_femt_r, false, 7, CAST(bridge_id), CAST(ofproto_n_tables), buf, CAST(elem), CAST(table_id), match,
      CAST(priority));
}

// 24 CLS_CURSOR_FOR_EACH
// 24.1 Count
int
SGX_ccfe_c(int bridge_id, int table_id){
    int buffer_size;
    ECALL(ecall_ccfe_c, true, 2, &buffer_size, CAST(bridge_id), CAST(table_id));
    return buffer_size;
}

// 24.2 Request
void
SGX_ccfe_r(int bridge_id, struct cls_rule ** buf, int elem, int table_id){
    ECALL(ecall_ccfe_r, false, 4, CAST(bridge_id), buf, CAST(elem), CAST(table_id));
}

int
SGX_collect_ofmonitor_util_c(int bridge_id, int ofproto_n_tables, int table_id, const struct minimatch * match){
    int count;
    ECALL(ecall_collect_ofmonitor_util_c, true, 4, &count, CAST(bridge_id), CAST(ofproto_n_tables), CAST(
          table_id), match);
    return count;
}

void
SGX_collect_ofmonitor_util_r(int bridge_id, int ofproto_n_tables, struct cls_rule ** buf, int elem, int table_id,
  const struct minimatch * match){
    ECALL(ecall_collect_ofmonitor_util_r, false, 6, CAST(bridge_id), CAST(ofproto_n_tables), buf, CAST(elem),
      CAST(table_id), match);
}

// 25. One Part of Enable_eviction
void
SGX_oftable_enable_eviction(int bridge_id, int table_id, const struct mf_subfield * fields, size_t n_fields,
  uint32_t random_v,
  bool * no_change){
    ECALL(ecall_oftable_enable_eviction, false, 6, CAST(bridge_id), CAST(table_id), fields, CAST(n_fields),
      CAST(random_v), no_change);
}

// 25.1
void
SGX_oftable_disable_eviction(int bridge_id, int table_id){
    ECALL(ecall_oftable_disable_eviction, false, 2, CAST(bridge_id), CAST(table_id));
}

// 26 oftable destroy
void
SGX_ofproto_destroy(int bridge_id){
    ECALL(ecall_ofproto_destroy, false, 1, CAST(bridge_id));
}

// 27 Count total number of rules
unsigned int
SGX_total_rules(int bridge_id){
    unsigned int n_rules;
    ECALL(ecall_total_rules, true, 1, &n_rules, CAST(bridge_id));
    return n_rules;
}

// 28 Copy the name of the table
void
SGX_table_name(int bridge_id, int table_id, char * buf, size_t len){
    ECALL(ecall_table_name, false, 4, CAST(bridge_id), CAST(table_id), buf, CAST(len));
}

// 29 loose_match
int
SGX_cls_rule_is_loose_match(int bridge_id, struct cls_rule * o_cls_rule, const struct minimatch * criteria){
    int result;
    ECALL(ecall_cls_rule_is_loose_match, true, 3, &result, CAST(bridge_id), o_cls_rule, criteria);
    return result;
}

// 30. Dependencies for ofproto_flush__
int
SGX_fet_ccfes_c(int bridge_id){
    int count;
    ECALL(ecall_fet_ccfes_c, true, 1, &count, CAST(bridge_id));
    return count;
}

// 30.1
void
SGX_fet_ccfes_r(int bridge_id, struct cls_rule ** buf, int elem){
    ECALL(ecall_fet_ccfes_r, false, 3, CAST(bridge_id), buf, CAST(elem));
}

// 31 Dependencies for ofproto_get_all_flows
int
SGX_fet_ccfe_c(int bridge_id){
    int count;
    ECALL(ecall_fet_ccfe_c, true, 1, &count, CAST(bridge_id));
    return count;
}

// 31.2 REQUEST
void
SGX_fet_ccfe_r(int bridge_id, struct cls_rule ** buf, int elem){
    ECALL(ecall_fet_ccfe_r, false, 3, CAST(bridge_id), buf, CAST(elem));
}

// 33 Classifier_lookup
void
SGX_cls_lookup(int bridge_id, struct cls_rule ** o_cls_rule, int table_id, const struct flow * flow,
  struct flow_wildcards * wc){
    ECALL(ecall_cls_lookup, false, 5, CAST(bridge_id), o_cls_rule, CAST(table_id), flow, wc);
}

// 34. CLS_RULE priority
unsigned int
SGX_cls_rule_priority(int bridge_id, struct cls_rule * o_cls_rule){
    unsigned int priority;
    ECALL(ecall_cls_rule_priority, true, 2, &priority, CAST(bridge_id), o_cls_rule);
    return priority;
}

// Dependencies for destroy
int
SGX_desfet_ccfes_c(int bridge_id){
    int count;
    ECALL(ecall_desfet_ccfes_c, true, 1, &count, CAST(bridge_id));
    return count;
}

// 2.
void
SGX_desfet_ccfes_r(int bridge_id, struct cls_rule ** buf, int elem){
    ECALL(ecall_desfet_ccfes_r, false, 3, CAST(bridge_id), buf, CAST(elem));
}

// 37. CLS_RULE_DEPENDENCIES
unsigned int
SGX_cls_rule_format(int bridge_id, const struct cls_rule * o_cls_rule, struct match * megamatch){
    unsigned int priority;
    ECALL(ecall_cls_rule_format, true, 3, &priority, CAST(bridge_id), o_cls_rule, megamatch);
    return priority;
}

// 38 miniflow_expand inside the enclave
// This functions copies from the enclave information into the struct flow.
void
SGX_miniflow_expand(int bridge_id, struct cls_rule * o_cls_rule, struct flow * flow){
    ECALL(ecall_miniflow_expand, false, 3, CAST(bridge_id), o_cls_rule, flow);
}

// 39. Rule_calculate tag this needs to check the result and if not zero
// Calculate the tag_create deterministics
uint32_t
SGX_rule_calculate_tag(int bridge_id, struct cls_rule * o_cls_rule, const struct flow * flow, int table_id){
    uint32_t hash;
    ECALL(ecall_rule_calculate_tag, true, 4, &hash, CAST(bridge_id), o_cls_rule, flow, CAST(table_id));
    return hash;
}

// This Functions are used for the table_dpif in ofproto_dpif {

// 1.
void
SGX_table_dpif_init(int bridge_id, int n_tables){
    ECALL(ecall_sgx_table_dpif, false, 2, CAST(bridge_id), CAST(n_tables));
}

// 2.
int
SGX_table_update_taggable(int bridge_id, uint8_t table_id){
    int todo;
    ECALL(ecall_table_update_taggable, true, 2, &todo, CAST(bridge_id), CAST(table_id));
    return todo;
}

// 3.
int
SGX_is_sgx_other_table(int bridge_id, int id){
    int result;
    ECALL(ecall_is_sgx_other_table, true, 2, &result, CAST(bridge_id), CAST(id));
    return result;
}

// 4
uint32_t
SGX_rule_calculate_tag_s(int bridge_id, int table_id, const struct flow * flow){
    uint32_t hash;
    ECALL(ecall_rule_calculate_tag_s, true, 3, &hash, CAST(bridge_id), CAST(table_id), flow);
    return hash;
}

void
sgx_oftable_check_hidden(int bridge_id){
    ECALL(ecall_hidden_tables_check, false, 1, CAST(bridge_id));
}

void
SGX_oftable_set_name(int bridge_id, int table_id, char * name){
    ECALL(ecall_oftable_set_name, false, 3, CAST(bridge_id), CAST(table_id), name);
}

// These functions are going to be used by ofopgroup_complete
uint16_t
SGX_minimask_get_vid_mask(int bridge_id, struct cls_rule * o_cls_rule){
    uint16_t result;
    ECALL(ecall_minimask_get_vid_mask, true, 2, &result, CAST(bridge_id), o_cls_rule);
    return result;
}

uint16_t
SGX_miniflow_get_vid(int bridge_id, struct cls_rule * o_cls_rule){
    uint16_t result;
    ECALL(ecall_miniflow_get_vid, true, 2, &result, CAST(bridge_id), o_cls_rule);
    return result;
}

// These functions are depencencies for ofproto_get_vlan_usage
// 1. Count
int
SGX_ofproto_get_vlan_usage_c(int bridge_id){
    int count;
    ECALL(ecall_ofproto_get_vlan_c, true, 1, &count, CAST(bridge_id));
    return count;
}

// 2. Allocate
void
SGX_ofproto_get_vlan_usage__r(int bridge_id, uint16_t * buf, int elem){
    ECALL(ecall_ofproto_get_vlan_r, false, 3, CAST(bridge_id), buf, CAST(elem));
}
