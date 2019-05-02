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
#include "hotcall-producer.h"
#include "cache-untrusted.h"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid        = 0;
static bool enclave_is_initialized = false;
static int bridge_counter = 0;
static async_ecall ctx;

#ifdef HOTCALL
# define ECALL(f, has_return, n_args, args ...) \
    argument_list arg_list; \
    void * return_val; \
    compile_arg_list(&return_val, &arg_list, has_return, n_args, args); \
    make_hotcall(&ctx, hotcall_ ## f, &arg_list, return_val)
    # define CAST(X) &X
#else
# define ECALL(f, has_return, n_args, args ...) \
    f(global_eid, args)
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

        initialize_enclave_cache(&ctx.flow_cache);

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
SGX_cls_lookup(int bridge_id, struct cls_rule ** ut_cr, int table_id, const struct flow *flow,
  struct flow_wildcards * wc){

    // deallocate any marker pages from shared memory

    //deallocate_marked_pages(&ctx.flow_cache.shared_memory);


    // Check if mapping is in cache
    cls_cache_entry *cache_entry;
    cache_entry = flow_map_cache_get_entry(&ctx.flow_cache, flow, wc, bridge_id, table_id);
    if(cache_entry) {
        printf("CACHE HIT!!!!\n");
        *ut_cr = cache_entry->cr;
        return;
    } else {
        printf("NO CACHE HIT...\n");
    }

    ECALL(ecall_cls_lookup, false, 5, CAST(bridge_id), ut_cr, CAST(table_id), flow, wc);
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

// Optimized calls

size_t
SGX_get_cls_rules(int bridge_id,
                  int table_id,
                  size_t start_index,
                  size_t end_index,
                  struct cls_rule ** buf,
                  size_t buf_size,
                  size_t *n_rules) {
    size_t n;
    ECALL(ecall_get_cls_rules,
          true, 7, &n,
          CAST(bridge_id),
          CAST(table_id),
          CAST(start_index),
          CAST(end_index),
          buf,
          CAST(buf_size),
          n_rules);
    return n;
}


size_t
SGX_get_cls_rules_and_enable_eviction(int bridge_id,
									 int table_id,
									 size_t start_index,
									 size_t end_index,
									 struct cls_rule ** buf,
									 size_t buf_size,
									 size_t *n_rules,
								 	 const struct mf_subfield *fields,
								 	 size_t n_fields,
							 	 	 uint32_t random_v,
						 		 	 bool *no_change,
                                     bool *is_eviction_fields_enabled)
{
    size_t n;
    ECALL(ecall_get_cls_rules_and_enable_eviction,
          true, 12, &n,
          CAST(bridge_id),
          CAST(table_id),
          CAST(start_index),
          CAST(end_index),
          buf,
          CAST(buf_size),
          n_rules,
          fields,
          CAST(n_fields),
          CAST(random_v),
          no_change,
          is_eviction_fields_enabled);
    return n;
}

void
SGX_eviction_group_add_rules(int bridge_id,
                             int table_id,
                             size_t n,
                             struct cls_rule **cls_rules,
                             struct heap_node *evg_nodes,
                             uint32_t *rule_priorities,
                             uint32_t group_priority)
{
    ECALL(ecall_eviction_group_add_rules,
          false, 7,
          CAST(bridge_id),
          CAST(table_id),
          CAST(n),
          cls_rules,
          evg_nodes,
          rule_priorities,
          CAST(group_priority));
}

size_t
SGX_ofproto_get_vlan_usage(int bridge_id,
                           size_t buf_size,
                           uint16_t *vlan_buffer,
                           size_t start_index,
                           size_t end_index,
                           size_t *n_vlan)
{
    size_t n;
    ECALL(ecall_ofproto_get_vlan_usage,
          true, 6, &n,
          CAST(bridge_id),
          CAST(buf_size),
          vlan_buffer,
          CAST(start_index),
          CAST(end_index),
          n_vlan);
    return n;
}

size_t
SGX_ofproto_flush(int bridge_id,
                  struct cls_rule **cls_rules,
                  uint32_t *hashes,
                  size_t buf_size,
                  size_t start_index,
                  size_t end_index,
                  size_t *n_rules) {
    size_t n;
    ECALL(ecall_ofproto_flush,
          true, 7, &n,
          CAST(bridge_id),
          cls_rules,
          hashes,
          CAST(buf_size),
          CAST(start_index),
          CAST(end_index),
          n_rules);
    return n;
}


size_t
SGX_ofproto_evict(int bridge_id,
                  int ofproto_n_tables,
                  size_t start_index,
                  uint32_t *hashes,
                  struct cls_rule **cls_rules,
                  size_t buf_size,
                  size_t *n_evictions)
{
    size_t n;
    ECALL(ecall_ofproto_evict,
          true, 7, &n,
          CAST(bridge_id),
          CAST(ofproto_n_tables),
          CAST(start_index),
          hashes,
          cls_rules,
          CAST(buf_size),
          n_evictions);
    return n;
}

void
SGX_add_flow(int bridge_id,
			 int table_id,
			 struct cls_rule *cr,
             struct cls_rule **victim,
             struct cls_rule **evict,
			 struct match *match,
             uint32_t *evict_rule_hash,
             uint16_t *vid,
             uint16_t *vid_mask,
			 unsigned int priority,
			 uint16_t flags,
             uint32_t group_eviction_priority,
			 uint32_t rule_eviction_priority,
             struct heap_node eviction_node,
             struct cls_rule **pending_deletions,
             int n_pending,
             bool has_timeout,
             bool *table_overflow,
             bool *is_rule_modifiable,
             bool *is_rule_overlapping,
             bool *is_deletion_pending,
			 bool *is_read_only)
 {
     ECALL(
         ecall_add_flow, false, 22,
         CAST(bridge_id),
         CAST(table_id),
         cr,
         victim,
         evict,
         match,
         evict_rule_hash,
         vid,
         vid_mask,
         CAST(priority),
         CAST(flags),
         CAST(group_eviction_priority),
         CAST(rule_eviction_priority),
         CAST(eviction_node),
         pending_deletions,
         CAST(n_pending),
         CAST(has_timeout),
         table_overflow,
         is_rule_modifiable,
         is_rule_overlapping,
         is_deletion_pending,
         is_read_only
     );
 }

 size_t
 SGX_collect_rules_strict(int bridge_id,
                         int table_id,
                         int n_tables,
                         struct match *match,
                         unsigned int priority,
                         bool *rule_is_hidden_buffer,
                         struct cls_rule **cls_rule_buffer,
                         bool *rule_is_modifiable,
                         size_t buffer_size)
{
    size_t n;
    ECALL(
        ecall_collect_rules_strict, true, 9,
        &n, CAST(bridge_id), CAST(table_id), CAST(n_tables), match, CAST(priority),
        rule_is_hidden_buffer, cls_rule_buffer, rule_is_modifiable, CAST(buffer_size)
    );
    return n;
}


size_t
SGX_collect_rules_loose(int bridge_id,
                       int table_id,
                       int ofproto_n_tables,
                       size_t start_index,
                       struct match *match,
                       bool *rule_is_hidden_buffer,
                       struct cls_rule **cls_rule_buffer,
                       size_t buffer_size,
                       bool *rule_is_modifiable,
                       size_t *n_rules)
{
    size_t n;
    ECALL(
        ecall_collect_rules_loose, true, 10,
        &n, CAST(bridge_id), CAST(table_id), CAST(ofproto_n_tables), CAST(start_index), match,
        rule_is_hidden_buffer, cls_rule_buffer, CAST(buffer_size), rule_is_modifiable, n_rules
    );
    return n;
}

void
SGX_delete_flows(int bridge_id,
				 int *rule_table_ids,
				 struct cls_rule **cls_rules,
				 bool *rule_is_hidden,
				 uint32_t *rule_hashes,
				 unsigned int *rule_priorities,
				 struct match *match, size_t n)
 {

     /*for(size_t i = 0; i < n; ++i) {
         struct flow flow = match[i].flow;
         struct flow_wildcards wc = match[i].wc;
         flow_zero_wildcards(&flow, &wc);
         size_t hash = flow_hash(&flow, 0);

         cls_cache_entry *cache_entry;
         HMAP_FOR_EACH_WITH_HASH(cache_entry, hmap_node, hash, &ctx.lru_cache) {
             hmap_remove(&ctx.lru_cache, &cache_entry->hmap_node);
             free(cache_entry);
             break;
         }
     }*/

     /*cls_cache_entry *cache_entry, *next;
     HMAP_FOR_EACH_SAFE(cache_entry, next, hmap_node, &ctx.lru_cache) {
         hmap_remove(&ctx.lru_cache, &cache_entry->hmap_node);
         free(cache_entry);
     }*/


     ECALL(
         ecall_delete_flows, false, 8, CAST(bridge_id), rule_table_ids, cls_rules, rule_is_hidden, rule_hashes, rule_priorities, match, CAST(n)
     );
 }

 void
 SGX_configure_table(int bridge_id,
                     int table_id,
                     char *name,
                     unsigned int max_flows,
                     struct mf_subfield *groups,
                     size_t n_groups,
                     bool *need_to_evict,
                     bool *is_read_only)
{
    ECALL(
        ecall_configure_table, false, 8, CAST(bridge_id), CAST(table_id), name, CAST(max_flows), groups, CAST(n_groups), need_to_evict, is_read_only
    );
}

bool
SGX_need_to_evict(int bridge_id, int table_id) {
    bool evict;
    ECALL(
            ecall_need_to_evict, true, 2, &evict, CAST(bridge_id), CAST(table_id)
    );
    return evict;
}

size_t
SGX_collect_rules_loose_stats_request(int bridge_id,
                                      int table_id,
                                      int n_tables,
                                      size_t start_index,
                                      size_t buffer_size,
                                      struct match *match,
                                      struct cls_rule **cls_rules,
                                      struct match *matches,
                                      unsigned int *priorities,
                                      size_t *n_rules)
{
    size_t n;
    ECALL(ecall_collect_rules_loose_stats_request,
        true, 10, &n,
        CAST(bridge_id),
        CAST(table_id),
        CAST(n_tables),
        CAST(start_index),
        CAST(buffer_size),
        match,
        cls_rules,
        matches,
        priorities,
        n_rules
    );
    return n;
}

void
SGX_ofproto_rule_send_removed(int bridge_id, struct cls_rule *cr, struct match *match, unsigned int *priority, bool *rule_is_hidden) {
    ECALL(ecall_ofproto_rule_send_removed, false, 5, CAST(bridge_id), cr, match, priority, rule_is_hidden);
}


size_t
SGX_remove_rules(int bridge_id, int *table_ids, struct cls_rule **rules, bool *is_hidden, size_t n_rules) {
    /*cls_cache_entry *cache_entry, *next;
    HMAP_FOR_EACH_SAFE(cache_entry, next, hmap_node, &ctx.lru_cache) {
        hmap_remove(&ctx.lru_cache, &cache_entry->hmap_node);
        free(cache_entry);
    }*/
    size_t n;
    ECALL(ecall_remove_rules, true, 5, &n, CAST(bridge_id), table_ids, rules, is_hidden, CAST(n_rules));
    return n;
}

// 37. CLS_RULE_DEPENDENCIES
void
SGX_cls_rules_format(int bridge_id, const struct cls_rule *cls_rules, struct match *megamatches, size_t n){
    ECALL(ecall_cls_rules_format, false, 3, CAST(bridge_id), cls_rules, megamatches, CAST(n));
}

/*size_t
SGX_ofproto_evict_get_rest(uint32_t *rule_hashes, struct cls_rule ** cls_rules, size_t buf_size) {
    ECALL(ecall_ofproto_evict_get_rest, false, 3, rule_hashes, cls_rules, CAST(buf_size));
}*/
