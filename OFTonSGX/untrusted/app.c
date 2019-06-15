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
#include "cache-untrusted.h"
#include "shared-memory-untrusted.h"
#include "common.h"
#include "ovs-benchmark.h"

#include "hotcall_utils.h"
#include "hotcall.h"
#include "hotcall-untrusted.h"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid        = 0;
static bool enclave_is_initialized = false;
static uint8_t bridge_counter = 0;

static struct shared_memory_ctx sm_ctx;

//static struct preallocated_function_calls pfc;

#define ECALL(f, args ...) f(global_eid, ## args)


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
        printf("HOTCALLS ENABLED.\n");
        //flow_map_cache_init(&sm_ctx.flow_cache);
        #endif

        #ifdef BATCH_ALLOCATION
        printf("BATCH ALLOCATIONS ENABLED.\n");
        #endif

        #ifdef BATCHING
        printf("BATCH ECALL ENABLED.\n");
        #endif

        #ifdef TIMEOUT
        puts("TIMEOUT ENABLED\n");
        #endif
    }
    uint8_t this_bridge_id = bridge_counter++;
    ecall_ofproto_init_tables(global_eid, this_bridge_id, n_tables);

    #ifdef HOTCALL
    if(this_bridge_id == 0) {
        hotcall_init(&sm_ctx, global_eid);
    }
    #endif

    return this_bridge_id;
}

void
SGX_async_test() {}

void
SGX_readonly_set(uint8_t bridge_id, uint8_t table_id){
    #ifdef HOTCALL
    /*/*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_uint8(&pfc, table_id) : &table_id;
    HCALL(ecall_oftable_set_readonly, async, NULL, 2, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_set_readonly), VAR(bridge_id, ui8), VAR(table_id, ui8));
    #else
    ECALL(ecall_oftable_set_readonly, bridge_id, table_id);
    #endif
}

int
SGX_istable_readonly(uint8_t bridge_id, uint8_t table_id){
    int ecall_return;
    #ifdef HOTCALL
    /*/*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    HCALL(ecall_oftable_is_readonly, async, &ecall_return, 2, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_is_readonly, .has_return = true), VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd'));
    #else
    ECALL(ecall_oftable_is_readonly, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

void
SGX_cls_rule_init(uint8_t bridge_id, struct cls_rule * o_cls_rule,
  const struct match * match, unsigned int priority){
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_voidptr(&pfc, o_cls_rule) : o_cls_rule;
    args[2] = async ? next_voidptr(&pfc, (struct match *) match) : (struct match *) match;
    args[3] = async ? next_unsigned(&pfc, priority) : &priority;
    HCALL(ecall_cls_rule_init, async, NULL, 4, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_rule_init),
        VAR(bridge_id, ui8), PTR(o_cls_rule, 'p'), PTR(match, 'p'), VAR(priority, 'u')
    );
    #else
    ECALL(ecall_cls_rule_init, bridge_id, o_cls_rule, match, priority);
    #endif

}

int
SGX_cr_rule_overlaps(uint8_t bridge_id, uint8_t table_id, struct cls_rule * ut_cr){
    int ecall_return;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = ut_cr;
    HCALL(ecall_cr_rule_overlaps, async, &ecall_return, 3, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cr_rule_overlaps, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr, 'p'), VAR(ecall_return, 'd')
    );
    #else
    ECALL(ecall_cr_rule_overlaps, &ecall_return, bridge_id, table_id, ut_cr);
    #endif
    return ecall_return;
}

void
SGX_cls_rule_destroy(uint8_t bridge_id, struct cls_rule * ut_cr){
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_voidptr(&pfc, ut_cr) : ut_cr;
    HCALL(ecall_cls_rule_destroy, async, NULL, 2, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_rule_destroy),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p')
    );
    #else
    ECALL(ecall_cls_rule_destroy, bridge_id, ut_cr);
    #endif
}

uint32_t
SGX_cls_rule_hash(uint8_t bridge_id, const struct cls_rule * ut_cr, uint32_t basis){
    uint32_t ecall_return;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = (struct cls_rule *) ut_cr;
    args[2] = &basis;
    HCALL(ecall_cls_rule_hash, async, &ecall_return, 3, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_rule_hash, .has_return = true),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(basis, ui32), VAR(ecall_return, ui32)
    );
    #else
    ECALL(ecall_cls_rule_hash, &ecall_return, bridge_id, ut_cr, basis);
    #endif
    return ecall_return;
}

int
SGX_cls_rule_equal(uint8_t bridge_id, const struct cls_rule * ut_cr_a, const struct cls_rule * ut_cr_b){
    int ecall_return;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = (struct cls_rule *) ut_cr_a;
    args[2] = (struct cls_rule *) ut_cr_b;
    HCALL(ecall_cls_rule_equal, async, &ecall_return, 3, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_rule_equal, .has_return = true),
        VAR(bridge_id, ui8), PTR(ut_cr_a, 'p'), PTR(ut_cr_b, 'p'), VAR(ecall_return, 'd')
    );
    #else
    ECALL(ecall_cls_rule_equal, &ecall_return, bridge_id, ut_cr_a, ut_cr_b);
    #endif
    return ecall_return;
}

void
SGX_classifier_replace(uint8_t bridge_id, uint8_t table_id, struct cls_rule * ut_cr_insert, struct cls_rule ** ut_cr_remove){
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = ut_cr_insert;
    args[3] = ut_cr_remove;
    HCALL(ecall_oftable_classifier_replace, async, NULL, 4, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_classifier_replace),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr_insert, 'p'), PTR(ut_cr_remove, 'p')
    );
    #else
    ECALL(ecall_oftable_classifier_replace, bridge_id, table_id, ut_cr_insert, ut_cr_remove);
    #endif
}

enum oftable_flags
SGX_rule_get_flags(uint8_t bridge_id, uint8_t table_id){
    enum oftable_flags ecall_return;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    HCALL(ecall_oftable_get_flags, async, &ecall_return, 2, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_get_flags, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd')
    );
    #else
    ECALL(ecall_oftable_get_flags, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

int
SGX_cls_count(uint8_t bridge_id, uint8_t table_id){
    int ecall_return;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    HCALL(ecall_oftable_cls_count, async, &ecall_return, 2, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_cls_count, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd')
    );
    #else
    ECALL(ecall_oftable_cls_count, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

int
SGX_eviction_fields_enable(uint8_t bridge_id, uint8_t table_id){
    int ecall_return;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    HCALL(ecall_is_eviction_fields_enabled, async, &ecall_return, 2, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_is_eviction_fields_enabled, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd')
    );
    #else
    ECALL(ecall_is_eviction_fields_enabled, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

void
SGX_evg_add_rule(uint8_t bridge_id, uint8_t table_id, struct cls_rule * ut_cr, uint32_t priority,
  uint32_t rule_evict_prioriy){
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_uint8(&pfc, table_id) : &table_id;
    args[2] = async ? next_voidptr(&pfc, ut_cr) : ut_cr;
    args[3] = async ? next_uint32(&pfc, priority) : &priority;;
    args[4] = async ? next_uint32(&pfc, rule_evict_prioriy) : &rule_evict_prioriy;
    HCALL(ecall_evg_add_rule, async, NULL, 5, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_evg_add_rule),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr, 'p'), VAR(priority, ui32), VAR(rule_evict_prioriy, ui32)
    );
    #else
    ECALL(
        ecall_evg_add_rule,
        bridge_id,
        table_id,
        ut_cr,
        &priority,
        rule_evict_prioriy
    );
    #endif
}

void
SGX_evg_remove_rule(uint8_t bridge_id, uint8_t table_id, struct cls_rule * ut_cr){
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_uint8(&pfc, table_id) : &table_id;
    args[2] = async ? next_voidptr(&pfc, ut_cr) : ut_cr;
    HCALL(ecall_evg_remove_rule, async, NULL, 3, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_evg_remove_rule),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr, 'p')
    );
    #else
    ECALL(ecall_evg_remove_rule, bridge_id, table_id, ut_cr);
    #endif
}

void
SGX_cls_remove(uint8_t bridge_id, uint8_t table_id, struct cls_rule * ut_cr){
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_uint8(&pfc, table_id) : &table_id;
    args[2] = async ? next_voidptr(&pfc, ut_cr) : ut_cr;
    HCALL(ecall_cls_remove, async, NULL, 3, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_remove),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr, 'p')
    );
    #else
    ECALL(ecall_cls_remove, bridge_id, table_id, ut_cr);
    #endif
}

void
SGX_choose_rule_to_evict(uint8_t bridge_id, uint8_t table_id, struct cls_rule ** ut_cr_victim){
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = ut_cr_victim;
    HCALL(ecall_choose_rule_to_evict, async, NULL, 3, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_choose_rule_to_evict),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr_victim, 'p')
    );
    #else
    ECALL(ecall_choose_rule_to_evict, bridge_id, table_id, ut_cr_victim);
    #endif
}

void
SGX_choose_rule_to_evict_p(uint8_t bridge_id, uint8_t table_id, struct cls_rule **ut_cr_victim, struct cls_rule *ut_cr_replacer){
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = ut_cr_victim;
    args[3] = ut_cr_replacer;
    HCALL(ecall_choose_rule_to_evict_p, async, NULL, 4, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_choose_rule_to_evict_p),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr_victim, 'p'), PTR(ut_cr_replacer, 'p')
    );
    #else
    ECALL(ecall_choose_rule_to_evict_p, bridge_id, table_id, ut_cr_victim, ut_cr_replacer);
    #endif
}

unsigned int
SGX_table_mflows(uint8_t bridge_id, uint8_t table_id){
    unsigned int ecall_return;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    HCALL(ecall_oftable_mflows, async, &ecall_return, 2, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_mflows, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'u')
    );
    #else
    ECALL(ecall_oftable_mflows, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

void
SGX_table_mflows_set(uint8_t bridge_id, uint8_t table_id, unsigned int new_value){
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_uint8(&pfc, table_id) : &table_id;
    args[2] = async ? next_unsigned(&pfc, new_value) : &new_value;
    HCALL(ecall_oftable_mflows_set, async, NULL, 3, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_mflows_set),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(new_value, 'u')
    );
    #else
    ECALL(ecall_oftable_mflows_set, bridge_id, table_id, new_value);
    #endif
}

void
SGX_minimatch_expand(uint8_t bridge_id, struct cls_rule * ut_cr, struct match * dst){
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = ut_cr;
    args[2] = dst;
    HCALL(ecall_minimatch_expand, async, NULL, 3, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_minimatch_expand),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), PTR(dst, 'p')
    );
    #else
    ECALL(ecall_minimatch_expand, bridge_id, ut_cr, dst);
    #endif
}

unsigned int
SGX_cr_priority(uint8_t bridge_id, const struct cls_rule * ut_cr){
    unsigned int ecall_return;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = (struct cls_rule *) ut_cr;
    HCALL(ecall_cr_priority, async, &ecall_return, 2, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cr_priority, .has_return = true),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(ecall_return, 'u')
    );
    #else
    ECALL(ecall_cr_priority, &ecall_return, bridge_id, ut_cr);
    #endif
    return ecall_return;
}

void
SGX_cls_find_match_exactly(
    uint8_t bridge_id,
    uint8_t table_id,
    const struct match *target,
    unsigned int priority,
    struct cls_rule ** ut_cr)
{
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = (struct match *) target;
    args[3] = &priority;
    args[4] = ut_cr;
    HCALL(ecall_oftable_cls_find_match_exactly, async, NULL, 5, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_cls_find_match_exactly),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(target, 'p'), VAR(priority, 'u'), PTR(ut_cr, 'p')
    );
    #else
    ECALL(ecall_oftable_cls_find_match_exactly, bridge_id, table_id, target, priority, ut_cr);
    #endif
}

size_t
SGX_collect_rules_loose_c(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id, const struct match * match){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &ofproto_n_tables;
    args[2] = &table_id;
    args[3] = (struct match *) match;
    HCALL(ecall_collect_rules_loose_c, async, &n, 4, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_rules_loose_c, .has_return = true),
        VAR(bridge_id, ui8), VAR(ofproto_n_tables, 'd'), VAR(table_id, ui8), PTR(match, 'p'), VAR(n, 'u')
    );
    #else
    ECALL(ecall_collect_rules_loose_c, &n, bridge_id, ofproto_n_tables, table_id, match);
    #endif
    return n;
}

void
SGX_collect_rules_loose_r(uint8_t bridge_id,
                int ofproto_n_tables,
                struct cls_rule ** buf,
                int elem,
                uint8_t table_id,
                const struct match * match)
{
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &ofproto_n_tables;
    args[2] = buf;
    args[3] = &elem;
    args[4] = &table_id;
    args[5] = (struct match *) match;
    HCALL(ecall_collect_rules_loose_r, async, &n, 6, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_rules_loose_r, .has_return = true),
        VAR(bridge_id, ui8), VAR(ofproto_n_tables, 'd'), PTR(buf, 'p'), VAR(elem, 'd'), VAR(table_id, ui8), PTR(match, 'p'), VAR(n, 'u')
    );
    #else
    ECALL(ecall_collect_rules_loose_r, &n, bridge_id, ofproto_n_tables, buf, elem, table_id, match);
    #endif
}

size_t
SGX_collect_rules_strict_c(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id, const struct match * match, unsigned int priority){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &ofproto_n_tables;
    args[2] = &table_id;
    args[3] = (struct match *) match;
    args[4] = &priority;
    HCALL(ecall_collect_rules_strict_c, async, &n, 5, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_rules_strict_c, .has_return = true),
        VAR(bridge_id, ui8), VAR(ofproto_n_tables, 'd'), VAR(table_id, ui8), PTR(match, 'p'), VAR(priority, 'u'), VAR(n, 'u')
    );
    #else
    ECALL(ecall_collect_rules_strict_c, &n, bridge_id, ofproto_n_tables, table_id, match, priority);
    #endif
    return n;
}

void
SGX_collect_rules_strict_r(
    uint8_t bridge_id,
    int ofproto_n_tables,
    struct cls_rule ** buf,
    int elem,
    uint8_t table_id,
    const struct match * match,
    unsigned int priority)
{
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &ofproto_n_tables;
    args[2] = buf;
    args[3] = &elem;
    args[4] = &table_id;
    args[5] = (struct match *) match;
    args[6] = &priority;
    HCALL(ecall_collect_rules_strict_r, async, &n, 7, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_rules_strict_r, .has_return = true),
        VAR(bridge_id, ui8), VAR(ofproto_n_tables, 'd'), PTR(buf, 'p'), VAR(elem, 'd'), VAR(table_id, ui8), PTR(match, 'p'), VAR(priority, 'u'), VAR(n, 'u')
    );
    #else
    ECALL(ecall_collect_rules_strict_r, &n, bridge_id, ofproto_n_tables, buf, elem, table_id, match, priority);
    #endif
}

size_t
SGX_oftable_enable_eviction_c(uint8_t bridge_id, uint8_t table_id){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    HCALL(ecall_oftable_enable_eviction_c, async, &n, 2, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_enable_eviction_c, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(n, 'u')
    );
    #else
    ECALL(ecall_oftable_enable_eviction_c, &n, bridge_id, table_id);
    #endif
    return n;
}

// 24.2 Request
void
SGX_oftable_enable_eviction_r(uint8_t bridge_id, struct cls_rule ** buf, int elem, uint8_t table_id){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = buf;
    args[2] = &elem;
    args[3] = &table_id;
    HCALL(ecall_oftable_enable_eviction_r, async, &n, 4, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_enable_eviction_r, .has_return = true),
        VAR(bridge_id, ui8), PTR(buf), VAR(elem, 'd'), VAR(table_id, ui8), VAR(n, 'u')
    );
    #else
    ECALL(ecall_oftable_enable_eviction_r, &n, bridge_id, buf, elem, table_id);
    #endif
}

size_t
SGX_collect_ofmonitor_util_c(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id, const struct minimatch * match){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &ofproto_n_tables;
    args[2] = &table_id;
    args[3] = (struct minimatch *) match;
    HCALL(ecall_collect_ofmonitor_util_c, async, &n, 4, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_ofmonitor_util_c, .has_return = true),
        VAR(bridge_id, ui8), VAR(ofproto_n_tables, 'd'), VAR(table_id, ui8), PTR(match), VAR(n, 'u')
    );
    #else
    ECALL(ecall_collect_ofmonitor_util_c, &n, bridge_id, ofproto_n_tables, table_id, match);
    #endif
    return n;
}

void
SGX_collect_ofmonitor_util_r(uint8_t bridge_id, int ofproto_n_tables, struct cls_rule ** buf, int elem, uint8_t table_id,
  const struct minimatch * match){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &ofproto_n_tables;
    args[2] = buf;
    args[3] = &elem;
    args[4] = &table_id;
    args[5] = (struct minimatch *) match;
    HCALL(ecall_collect_ofmonitor_util_r, async, &n, 6, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_ofmonitor_util_r, .has_return = true),
        VAR(bridge_id, ui8), VAR(ofproto_n_tables, 'd'), PTR(buf), VAR(elem, 'd'), VAR(table_id, ui8), PTR(match), VAR(n, 'u')
    );
    #else
    ECALL(ecall_collect_ofmonitor_util_r, &n, bridge_id, ofproto_n_tables, buf, elem, table_id, match);
    #endif
}

// 25. One Part of Enable_eviction
void
SGX_oftable_enable_eviction(uint8_t bridge_id, uint8_t table_id, const struct mf_subfield * fields, size_t n_fields,
  uint32_t random_v, bool * no_change){
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = (struct mf_subfield *) fields;
    args[3] = &n_fields;
    args[4] = &random_v;
    args[5] = no_change;
    HCALL(ecall_oftable_enable_eviction, async, NULL, 6, args);*/
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_enable_eviction),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(fields), VAR(n_fields, 'u'), VAR(random_v, ui32), PTR(no_change)
    );
    #else
    ECALL(ecall_oftable_enable_eviction, bridge_id, table_id, fields, n_fields, random_v, no_change);
    #endif
}

void
SGX_oftable_disable_eviction(uint8_t bridge_id, uint8_t table_id){
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_uint8(&pfc, table_id) : &table_id;
    HCALL(ecall_oftable_disable_eviction, async, NULL, 2, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_disable_eviction), VAR(bridge_id, ui8), VAR(table_id, ui8));
    #else
    ECALL(ecall_oftable_disable_eviction, bridge_id, table_id);
    #endif
}

void
SGX_ofproto_destroy(uint8_t bridge_id){
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    HCALL(ecall_ofproto_destroy, async, NULL, 1, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_ofproto_destroy), VAR(bridge_id, ui8));
    #else
    ECALL(ecall_ofproto_destroy, bridge_id);
    #endif
}

unsigned int
SGX_total_rules(uint8_t bridge_id){
    unsigned int n_rules;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    HCALL(ecall_total_rules, async, &n_rules, 1, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_total_rules, .has_return = true), VAR(bridge_id, ui8), VAR(n_rules, 'u'));
    #else
    ECALL(ecall_total_rules, &n_rules, bridge_id);
    #endif
    return n_rules;
}

// 28 Copy the name of the table
void
SGX_table_name(uint8_t bridge_id, uint8_t table_id, char * buf, size_t len){
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = buf;
    args[3] = &len;
    HCALL(ecall_oftable_name, async, NULL, 4, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_name), VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(buf, 'p'), VAR(len, 'u'));
    #else
    ECALL(ecall_oftable_name, bridge_id, table_id, buf, len);
    #endif
}

// 29 loose_match
int
SGX_cls_rule_is_loose_match(uint8_t bridge_id, struct cls_rule * ut_cr, const struct minimatch * criteria){
    int result;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = ut_cr;
    args[2] = (struct minimatch *) criteria;
    HCALL(ecall_cls_rule_is_loose_match, async, &result, 3, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_cls_rule_is_loose_match, .has_return = true), VAR(bridge_id, ui8), PTR(ut_cr), PTR(criteria), VAR(result, 'd'));

    #else
    ECALL(ecall_cls_rule_is_loose_match, &result, bridge_id, ut_cr, criteria);
    #endif
    return result;
}

// 30. Dependencies for ofproto_flush__
size_t
SGX_flush_c(uint8_t bridge_id){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    HCALL(ecall_flush_c, async, &n, 1, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_flush_c, .has_return = true), VAR(bridge_id, ui8), VAR(n, 'u'));
    #else
    ECALL(ecall_flush_c, &n, bridge_id);
    #endif
    return n;
}

// 30.1
void
SGX_flush_r(uint8_t bridge_id, struct cls_rule ** buf, int elem){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = buf;
    args[2] = &elem;
    HCALL(ecall_flush_r, async, &n, 3, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_flush_r, .has_return = true), VAR(bridge_id, ui8), PTR(buf), VAR(elem, 'd'), VAR(n, 'u'));
    #else
    ECALL(ecall_flush_r, &n, bridge_id, buf, elem);
    #endif
}

// 31 Dependencies for ofproto_get_all_flows
size_t
SGX_flow_stats_c(uint8_t bridge_id){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    HCALL(ecall_flow_stats_c, async, &n, 1, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_flow_stats_c, .has_return = true), VAR(bridge_id, ui8), VAR(n, 'u'));
    #else
    ECALL(ecall_flow_stats_c, &n, bridge_id);
    #endif
    return n;
}

// 31.2 REQUEST
void
SGX_flow_stats_r(uint8_t bridge_id, struct cls_rule ** buf, int elem){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = buf;
    args[2] = &elem;
    HCALL(ecall_flow_stats_r, async, &n, 3, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_flow_stats_r, .has_return = true), VAR(bridge_id, ui8), PTR(buf), VAR(elem, 'd'), VAR(n, 'u'));
    #else
    ECALL(ecall_flow_stats_r, &n, bridge_id, buf, elem);
    #endif
}

// 33 Classifier_lookup
void
SGX_cls_lookup(uint8_t bridge_id, struct cls_rule ** ut_cr, uint8_t table_id, const struct flow *flow,
  struct flow_wildcards * wc) {
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = ut_cr;
    args[2] = &table_id;
    args[3] = (struct flow *) flow;
    args[4] = wc;
    HCALL(ecall_oftable_cls_lookup, async, NULL, 5, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_cls_lookup), VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(table_id, ui8), PTR(flow, 'p'), PTR(wc, 'p'));
    #else
    ECALL(ecall_oftable_cls_lookup, bridge_id, ut_cr, table_id, flow, wc);
    #endif
}

// Dependencies for destroy
size_t
SGX_dpif_destroy_c(uint8_t bridge_id){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    HCALL(ecall_dpif_destroy_c, async, &n, 1, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_dpif_destroy_c, .has_return = true), VAR(bridge_id, ui8), VAR(n, 'u'));
    #else
    ECALL(ecall_dpif_destroy_c, &n, bridge_id);;
    #endif
    return n;
}

// 2.
void
SGX_dpif_destroy_r(uint8_t bridge_id, struct cls_rule ** buf, int elem){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = buf;
    args[2] = &elem;
    HCALL(ecall_dpif_destroy_r, async, &n, 3, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_dpif_destroy_r, .has_return = true), VAR(bridge_id, ui8), PTR(buf), VAR(elem, 'd'), VAR(n, 'u'));
    #else
    ECALL(ecall_dpif_destroy_r, &n, bridge_id, buf, elem);
    #endif
}

unsigned int
SGX_cls_rule_format(uint8_t bridge_id, const struct cls_rule * ut_cr, struct match * megamatch){
    unsigned int priority;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = (struct cls_rule *) ut_cr;
    args[2] = megamatch;
    HCALL(ecall_cls_rule_format, async, &priority, 3, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_cls_rule_format, .has_return = true), VAR(bridge_id, ui8), PTR(ut_cr), PTR(megamatch), VAR(priority, 'u'));
    #else
    ECALL(ecall_cls_rule_format, &priority, bridge_id, ut_cr, megamatch);
    #endif
    return priority;
}

void
SGX_miniflow_expand(uint8_t bridge_id, struct cls_rule * ut_cr, struct flow * flow){
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = ut_cr;
    args[2] = flow;
    HCALL(ecall_miniflow_expand, async, NULL, 3, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_miniflow_expand), VAR(bridge_id, ui8), PTR(ut_cr, 'p'), PTR(flow, 'p'));
    #else
    ECALL(ecall_miniflow_expand, bridge_id, ut_cr, flow);
    #endif
}

uint32_t
SGX_rule_calculate_tag(uint8_t bridge_id, struct cls_rule * ut_cr, const struct flow * flow, uint8_t table_id){
    uint32_t hash;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = ut_cr;
    args[2] = (struct flow *) flow;
    args[3] = &table_id;
    HCALL(ecall_rule_calculate_tag, async, &hash, 4, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_rule_calculate_tag, .has_return = true), VAR(bridge_id, ui8), PTR(ut_cr, 'p'), PTR(flow, 'p'), VAR(table_id, ui8), VAR(hash, ui32));
    #else
    ECALL(ecall_rule_calculate_tag, &hash, bridge_id, ut_cr, flow, table_id);
    #endif
    return hash;
}


// 2.
int
SGX_table_update_taggable(uint8_t bridge_id, uint8_t table_id){
    int ecall_return;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    HCALL(ecall_oftable_update_taggable, async, &ecall_return, 2, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_update_taggable, .has_return = true), VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd'));
    #else
    ECALL(ecall_oftable_update_taggable, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

// 3.
int
SGX_is_sgx_other_table(uint8_t bridge_id, uint8_t table_id){
    int result;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    HCALL(ecall_oftable_is_other_table, async, &result, 2, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_is_other_table, .has_return = true), VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(result, 'd'));
    #else
    ECALL(ecall_oftable_is_other_table, &result, bridge_id, table_id);
    #endif
    return result;
}

// 4
uint32_t
SGX_rule_calculate_tag_s(uint8_t bridge_id, uint8_t table_id, const struct flow * flow){
    uint32_t hash;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = (struct flow *) flow;
    HCALL(ecall_rule_calculate_tag_s, async, &hash, 3, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_rule_calculate_tag_s, .has_return = true), VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(flow, 'p'), VAR(hash, ui32));
    #else
    ECALL(ecall_rule_calculate_tag_s, &hash, bridge_id, table_id, flow);
    #endif
    return hash;
}

void
sgx_oftable_check_hidden(uint8_t bridge_id){
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    HCALL(ecall_oftable_hidden_check, async, NULL, 1, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_hidden_check), VAR(bridge_id, ui8));
    #else
    ECALL(ecall_oftable_hidden_check, bridge_id);
    #endif
}

void
SGX_oftable_set_name(uint8_t bridge_id, uint8_t table_id, char * name) {
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_uint8(&pfc, table_id) : &table_id;
    args[2] = name;
    HCALL(ecall_oftable_set_name, async, NULL, 3, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_set_name), VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(name, 'p'));
    #else
    ECALL(ecall_oftable_set_name, bridge_id, table_id, name);
    #endif
}

// These functions are going to be used by ofopgroup_complete
uint16_t
SGX_minimask_get_vid_mask(uint8_t bridge_id, struct cls_rule * ut_cr){

    uint16_t ecall_return;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = ut_cr;
    HCALL(ecall_minimask_get_vid_mask, async, &ecall_return, 2, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_minimask_get_vid_mask, .has_return = true), VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(ecall_return, ui16));
    #else
    ECALL(ecall_minimask_get_vid_mask, &ecall_return, bridge_id, ut_cr);
    #endif
    return ecall_return;
}

uint16_t
SGX_miniflow_get_vid(uint8_t bridge_id, struct cls_rule * ut_cr){
    uint16_t result;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = ut_cr;
    HCALL(ecall_miniflow_get_vid, async, &result, 2, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_miniflow_get_vid, .has_return = true), VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(result, ui16));
    #else
    ECALL(ecall_miniflow_get_vid, &result, bridge_id, ut_cr);
    #endif
    return result;
}

// These functions are depencencies for ofproto_get_vlan_usage
// 1. Count
size_t
SGX_ofproto_get_vlan_usage_c(uint8_t bridge_id){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    HCALL(ecall_ofproto_get_vlan_c, async, &n, 1, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_ofproto_get_vlan_c, .has_return = true), VAR(bridge_id, ui8), VAR(n, 'd'));
    #else
    ECALL(ecall_ofproto_get_vlan_c, &n, bridge_id);
    #endif
    return n;
}

// 2. Allocate
void
SGX_ofproto_get_vlan_usage__r(uint8_t bridge_id, uint16_t * buf, int elem){
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = buf;
    args[2] = &elem;
    HCALL(ecall_ofproto_get_vlan_r, async, &n, 3, args);*/
    HCALL(CONFIG(.function_id = hotcall_ecall_ofproto_get_vlan_r, .has_return = true), VAR(bridge_id, ui8), PTR(buf, ui16), VAR(elem, 'd'), VAR(n, 'u'));
    #else
    ECALL(ecall_ofproto_get_vlan_r, &n, bridge_id, buf, elem);
    #endif
}

// Optimized calls

size_t
SGX_get_cls_rules(uint8_t bridge_id,
                  uint8_t table_id,
                  size_t start_index,
                  size_t end_index,
                  struct cls_rule ** buf,
                  size_t buf_size,
                  size_t *n_rules) {
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = &start_index;
    args[3] = &end_index;
    args[4] = buf;
    args[5] = &buf_size;
    args[6] = n_rules;
    HCALL(ecall_oftable_get_cls_rules, async, &n, 7, args);*/
    #else
    ECALL(
        ecall_oftable_get_cls_rules,
        &n,
        bridge_id,
        table_id,
        start_index,
        end_index,
        buf,
        buf_size,
        n_rules
    );
    #endif
}

size_t
SGX_ofproto_get_vlan_usage(uint8_t bridge_id,
                           size_t buf_size,
                           uint16_t *vlan_buffer,
                           size_t start_index,
                           size_t end_index,
                           size_t *n_vlan)
{
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &buf_size;
    args[2] = vlan_buffer;
    args[3] = &start_index;
    args[4] = &end_index;
    args[5] = n_vlan;
    HCALL(ecall_ofproto_get_vlan_usage, async, &n, 6, args);*/
    #else
    ECALL(ecall_ofproto_get_vlan_usage, &n, bridge_id, buf_size, vlan_buffer, start_index, end_index, n_vlan);
    #endif
    return n;
}

size_t
SGX_ofproto_flush(uint8_t bridge_id,
                  struct cls_rule **ut_crs,
                  uint32_t *hashes,
                  size_t buf_size,
                  size_t start_index,
                  size_t end_index,
                  size_t *n_rules) {
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = ut_crs;
    args[2] = hashes;
    args[3] = &buf_size;
    args[4] = &start_index;
    args[5] = &end_index;
    args[6] = n_rules;
    HCALL(ecall_ofproto_flush, async, &n, 7, args);*/
    #else
    ECALL(ecall_ofproto_flush, &n, bridge_id, ut_crs, hashes, buf_size, start_index, end_index, n_rules);
    #endif
    return n;
}


size_t
SGX_ofproto_evict(uint8_t bridge_id,
                  int ofproto_n_tables,
                  uint32_t *hashes,
                  struct cls_rule **ut_crs,
                  uint8_t *eviction_is_hidden,
                  size_t buf_size,
                  size_t *n_evictions)
{
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &ofproto_n_tables;
    args[2] = hashes;
    args[3] = ut_crs;
    args[4] = eviction_is_hidden;
    args[5] = &buf_size;
    args[6] = n_evictions;
    HCALL(ecall_ofproto_evict, async, &n, 7, args);*/
    #else
    ECALL(ecall_ofproto_evict, &n, bridge_id, ofproto_n_tables, hashes, ut_crs, eviction_is_hidden, buf_size, n_evictions);
    #endif
    return n;
}
static int ii = 0;
void
#ifdef ARG_OPT
SGX_add_flow(void **args_)
#elif ARG_OPT_2
SGX_add_flow(struct add_flow_args *args_)
#else
SGX_add_flow(uint8_t bridge_id,
			 uint8_t table_id,
			 struct cls_rule *cr,
             struct cls_rule **victim,
             struct cls_rule **evict,
			 struct match *match,
             uint32_t *evict_rule_hash,
             uint16_t *vid,
             uint16_t *vid_mask,
			 unsigned int priority,
			 uint16_t flags,
			 uint32_t rule_eviction_priority,
             struct cls_rule **pending_deletions,
             int n_pending,
             bool has_timeout,
             uint16_t *state,
             int *table_update_taggable, unsigned int *evict_priority)
#endif
 {

     #ifdef HOTCALL
     /*bool async = ASYNC(false);
     void **args = pfc.args[pfc.idx];

    // if(ii > 2) {
    //     *(uint8_t *) args_[15] |= (1 << 4);
         //*state |= (1 << 4);
    //     //int ret;
    //     //args[0] = args_[0];
    //     //HCALL(ecall_oftable_mflows, async, &ret, 2, args);
    //     return;
    // }
    // ii++;

     #ifdef ARG_OPT
     memcpy(args, args_, 8 * 17);
     #elif ARG_OPT_2
     args[0] = (void *) args_;
     #else
     args[0] = &bridge_id;
     args[1] = &table_id;
     args[2] = cr;
     args[3] = victim;
     args[4] = evict;
     args[5] = match;
     args[6] = evict_rule_hash;
     args[7] = vid;
     args[8] = vid_mask;
     args[9] = &priority;
     args[10] = &flags;
     args[11] = &rule_eviction_priority;
     args[12] = pending_deletions;
     args[13] = &n_pending;
     args[14] = &has_timeout;
     args[15] = state;
     args[16] = table_update_taggable;
     args[17] = evict_priority;
     #endif

     #ifdef ARG_OPT_2
     HCALL(ecall_add_flow, async, NULL, 1, args);*/
     /*#else
     HCALL(ecall_add_flow, async, NULL, 18, args);
     #endif*/
     #else
     ECALL(
         ecall_add_flow,
         bridge_id,
         table_id,
         cr,
         victim,
         evict,
         match,
         evict_rule_hash,
         vid,
         vid_mask,
         priority,
         flags,
         rule_eviction_priority,
         pending_deletions,
         n_pending,
         has_timeout,
         state,
         table_update_taggable,
         evict_priority
     );
     #endif
 }


/*table_overflow,
         is_rule_modifiable,
         is_rule_overlapping,
         is_deletion_pending,
         is_read_only,
         is_hidden,
         is_other_table, */

 size_t
 SGX_collect_rules_strict(uint8_t bridge_id,
                         uint8_t table_id,
                         int n_tables,
                         struct match *match,
                         unsigned int priority,
                         ovs_be64 cookie,
                         ovs_be64 cookie_mask,
                         uint16_t out_port,
                         struct cls_rule **cls_rule_buffer,
                         bool *ofproto_postpone,
                         size_t buffer_size)
{
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = &n_tables;
    args[3] = match;
    args[4] = &priority;
    args[5] = &cookie;
    args[6] = &cookie_mask;
    args[7] = &out_port;
    args[8] = cls_rule_buffer;
    args[9] = ofproto_postpone;
    args[10] = &buffer_size;
    HCALL(ecall_collect_rules_strict, async, &n, 11, args);*/
    #else
    ECALL(
        ecall_collect_rules_strict,
        &n,
        bridge_id,
        table_id,
        n_tables,
        match,
        priority,
        cookie,
        cookie_mask,
        out_port,
        cls_rule_buffer,
        ofproto_postpone,
        buffer_size
    );
    #endif
    return n;
}


size_t
SGX_collect_rules_loose(uint8_t bridge_id,
                       uint8_t table_id,
                       int ofproto_n_tables,
                       size_t start_index,
                       struct match *match,
                       struct cls_rule **cls_rule_buffer,
                       size_t buffer_size,
                       ovs_be64 cookie,
                       ovs_be64 cookie_mask,
                       uint16_t out_port,
                       bool *postpone,
                       size_t *n_rules)
{
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = &ofproto_n_tables;
    args[3] = &start_index;
    args[4] = match;
    args[5] = cls_rule_buffer;
    args[6] = &buffer_size;
    args[7] = &cookie;
    args[8] = &cookie_mask;
    args[9] = &out_port;
    args[10] = postpone;
    args[11] = n_rules;
    HCALL(ecall_collect_rules_loose, async, &n, 16, args);*/
    #else
    ECALL(
        ecall_collect_rules_loose,
        &n,
        bridge_id,
        table_id,
        ofproto_n_tables,
        start_index,
        match,
        cls_rule_buffer,
        buffer_size,
        cookie,
        cookie_mask,
        out_port,
        postpone,
        n_rules
    );
    #endif
    return n;
}
/*
void
SGX_delete_flows(uint8_t bridge_id,
				// uint8_t *rule_table_ids,
				 struct cls_rule **ut_crs,
				// bool *rule_is_hidden,
				 uint32_t *rule_hashes,
				 unsigned int *rule_priorities,
				 struct match *match,
                // int *table_update_taggable,
                // uint8_t *is_other_table,
                 size_t n)
 {
     #ifdef HOTCALL
     /*bool async = ASYNC(false);
     void **args = pfc.args[pfc.idx];
     args[0] = &bridge_id;
     //args[1] = rule_table_ids;
     args[1] = ut_crs;
     //args[3] = rule_is_hidden;
     args[2] = rule_hashes;
     args[3] = rule_priorities;
     args[4] = match;
    // args[7] = table_update_taggable;
    // args[8] = is_other_table;
     args[5] = &n;

     HCALL(ecall_delete_flows, async, NULL, 6, args);
    // #else
     ECALL(
         ecall_delete_flows,
         bridge_id,
        // rule_table_ids,
         ut_crs,
        // rule_is_hidden,
         rule_hashes,
         rule_priorities,
         match,
        // table_update_taggable,
        // is_other_table,
         n
     );
    // #endif
}*/

 size_t
 SGX_delete_flows_loose(uint8_t bridge_id,
                         uint8_t table_id,
                         int n_tables,
                         size_t offset,
                         struct match *match,
                         ovs_be64 cookie,
                         ovs_be64 cookie_mask,
                         uint16_t out_port,
                         struct cls_rule **cls_rule_buffer,
                         uint32_t *rule_hashes,
                         unsigned int *rule_priorities,
                         struct match *matches,
                         size_t buffer_size,
                         bool *rule_is_modifiable,
                         bool *rule_is_hidden,
                         bool *ofproto_postpone,
                         size_t *n_rules)
{
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = &n_tables;
    args[3] = &offset;
    args[4] = match;
    args[5] = &cookie;
    args[6] = &cookie_mask;
    args[7] = &out_port;
    args[8] = cls_rule_buffer;
    args[9] = rule_hashes;
    args[10] = rule_priorities;
    args[11] = matches;
    args[12] = &buffer_size;
    args[13] = rule_is_modifiable;
    args[14] = rule_is_hidden;
    args[15] = ofproto_postpone;
    args[16] = n_rules;
    HCALL(ecall_delete_flows_loose, async, &n, 17, args);*/
    #else
    ECALL(
        ecall_delete_flows_loose,
        &n,
        bridge_id,
        table_id,
        n_tables,
        offset,
        match,
        cookie,
        cookie_mask,
        out_port,
        cls_rule_buffer,
        rule_hashes,
        rule_priorities,
        matches,
        buffer_size,
        rule_is_modifiable,
        rule_is_hidden,
        ofproto_postpone,
        n_rules
    );
    #endif
    return n;
}

 size_t
 SGX_delete_flows_strict(uint8_t bridge_id,
                         uint8_t table_id,
                         int ofproto_n_tables,
                         struct match *match,
                         unsigned int priority,
                         ovs_be64 cookie,
                         ovs_be64 cookie_mask,
                         uint16_t out_port,
                         struct cls_rule **cls_rule_buffer,
                         uint32_t *rule_hashes,
                         unsigned int *rule_priorities,
                         struct match *matches,
                         bool *rule_is_modifiable,
                         bool *rule_is_hidden,
                         bool *ofproto_postpone,
                         size_t buffer_size)
 {
     size_t n;
     #ifdef HOTCALL
     /*bool async = ASYNC(false);
     void **args = pfc.args[pfc.idx];
     args[0] = &bridge_id;
     args[1] = &table_id;
     args[2] = &ofproto_n_tables;
     args[3] = match;
     args[4] = &priority;
     args[5] = &cookie;
     args[6] = &cookie_mask;
     args[7] = &out_port;
     args[8] = cls_rule_buffer;
     args[9] = rule_hashes;
     args[10] = rule_priorities;
     args[11] = matches;
     args[12] = rule_is_modifiable;
     args[13] = rule_is_hidden;
     args[14] = ofproto_postpone;
     args[15] = &buffer_size;

     HCALL(ecall_delete_flows_strict, async, &n, 16, args);*/
     #else
     ECALL(
         ecall_delete_flows_strict,
         &n,
         bridge_id,
         table_id,
         ofproto_n_tables,
         match,
         priority,
         cookie,
         cookie_mask,
         out_port,
         cls_rule_buffer,
         rule_hashes,
         rule_priorities,
         matches,
         rule_is_modifiable,
         rule_is_hidden,
         ofproto_postpone,
         buffer_size
     );
     #endif
     return n;
 }


size_t
SGX_modify_flows_strict(uint8_t bridge_id,
                         uint8_t table_id,
                         int ofproto_n_tables,
                         struct match *match,
                         unsigned int priority,
                         ovs_be64 cookie,
                         ovs_be64 cookie_mask,
                         uint16_t out_port,
                         struct cls_rule **cls_rule_buffer,
                         bool *rule_is_modifiable,
                         bool *rule_is_hidden,
                         bool *ofproto_postpone,
                         size_t buffer_size)
{
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = &ofproto_n_tables;
    args[3] = match;
    args[4] = &priority;
    args[5] = &cookie;
    args[6] = &cookie_mask;
    args[7] = &out_port;
    args[8] = cls_rule_buffer;
    args[9] = rule_is_modifiable;
    args[10] = rule_is_hidden;
    args[11] = ofproto_postpone;
    args[12] = &buffer_size;

    HCALL(ecall_modify_flows_strict, async, &n, 13, args);*/
    #else
    ECALL(
        ecall_modify_flows_strict,
        &n,
        bridge_id,
        table_id,
        ofproto_n_tables,
        match,
        priority,
        cookie,
        cookie_mask,
        out_port,
        cls_rule_buffer,
        rule_is_modifiable,
        rule_is_hidden,
        ofproto_postpone,
        buffer_size
    );
    #endif
    return n;
}


size_t
SGX_modify_flows_loose(uint8_t bridge_id,
						  uint8_t table_id,
						  int n_tables,
						  size_t offset,
						  struct match *match,
						  struct cls_rule **cr_rules,
						  size_t buffer_size,
						  ovs_be64 cookie,
						  ovs_be64 cookie_mask,
						  uint16_t out_port,
						  bool *rule_is_mod,
						  bool *rule_is_hidden,
						  bool *postpone,
						  size_t *n_rules)
{
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = &n_tables;
    args[3] = &offset;
    args[4] = match;
    args[5] = cr_rules;
    args[6] = &buffer_size;
    args[7] = &cookie;
    args[8] = &cookie_mask;
    args[9] = &out_port;
    args[10] = rule_is_mod;
    args[11] = rule_is_hidden;
    args[12] = postpone;
    args[13] = n_rules;
    HCALL(ecall_modify_flows_loose, async, &n, 14, args);*/
    #else
    ECALL(
        ecall_modify_flows_loose,
        &n,
        bridge_id,
        table_id,
        n_tables,
        offset,
        match,
        cr_rules,
        buffer_size,
        cookie,
        cookie_mask,
        out_port,
        rule_is_mod,
        rule_is_hidden,
        postpone,
        n_rules
    );
    #endif
    return n;
}

 void
 SGX_configure_table(uint8_t bridge_id,
                     uint8_t table_id,
                     char *name,
                     unsigned int max_flows,
                     struct mf_subfield *groups,
                     size_t n_groups,
                     uint32_t time_boot_msec,
                     bool *need_to_evict,
                     bool *is_read_only)
{
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = name;
    args[3] = &max_flows;
    args[4] = groups;
    args[5] = &n_groups;
    args[6] = &time_boot_msec;
    args[7] = need_to_evict;
    args[8] = is_read_only;
    HCALL(ecall_oftable_configure, async, NULL, 9, args);*/
    #else
    ECALL(
        ecall_oftable_configure,
        bridge_id,
        table_id,
        name,
        max_flows,
        groups,
        n_groups,
        time_boot_msec,
        need_to_evict,
        is_read_only
    );
    #endif
}

bool
SGX_need_to_evict(uint8_t bridge_id, uint8_t table_id) {
    bool evict;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    HCALL(ecall_need_to_evict, async, &evict, 2, args);*/
    #else
    ECALL(
        ecall_need_to_evict,
        &evict,
        bridge_id,
        table_id
    );
    #endif
    return evict;
}

size_t
SGX_collect_rules_loose_stats_request(uint8_t bridge_id,
                                      uint8_t table_id,
                                      int n_tables,
                                      size_t start_index,
                                      size_t buffer_size,
                                      struct match *match,
                                      struct cls_rule **ut_crs,
                                      struct match *matches,
                                      unsigned int *priorities,
                                      size_t *n_rules)
{
    size_t n;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &table_id;
    args[2] = &n_tables;
    args[3] = &start_index;
    args[4] = &buffer_size;
    args[5] = match;
    args[6] = ut_crs;
    args[7] = matches;
    args[8] = priorities;
    args[9] = n_rules;
    HCALL(ecall_collect_rules_loose_stats_request, async, &n, 10, args);*/
    #else
    ECALL(
        ecall_collect_rules_loose_stats_request,
        &n,
        bridge_id,
        table_id,
        n_tables,
        start_index,
        buffer_size,
        match,
        ut_crs,
        matches,
        priorities,
        n_rules
    );
    #endif
    return n;
}

void
SGX_ofproto_rule_send_removed(uint8_t bridge_id, struct cls_rule *cr, struct match *match, unsigned int *priority, bool *rule_is_hidden) {
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = cr;
    args[2] = match;
    args[3] = priority;
    args[4] = rule_is_hidden;
    HCALL(ecall_ofproto_rule_send_removed, async, NULL, 5, args);*/
    #else
    ECALL(
        ecall_ofproto_rule_send_removed,
        bridge_id,
        cr,
        match,
        priority,
        rule_is_hidden
    );
    #endif
}


void
SGX_remove_rules(uint8_t bridge_id, uint8_t *table_ids, struct cls_rule **ut_crs, bool *is_hidden, size_t n_rules) {
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = table_ids;
    args[2] = ut_crs;
    args[3] = is_hidden;
    args[4] = &n_rules;
    HCALL(ecall_oftable_remove_rules, async, NULL, 5, args);*/
    #else
    ECALL(
        ecall_oftable_remove_rules,
        bridge_id,
        table_ids,
        ut_crs,
        is_hidden,
        n_rules
    );
    #endif
}

void
SGX_cls_rules_format(uint8_t bridge_id, const struct cls_rule *ut_crs, struct match *megamatches, size_t n){
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = (struct cls_rule *) ut_crs;
    args[2] = megamatches;
    args[3] = &n;
    HCALL(ecall_cls_rules_format, async, NULL, 4, args);*/
    #else
    ECALL(
        ecall_cls_rules_format,
        bridge_id,
        ut_crs,
        megamatches,
        n
    );
    #endif
}

void
SGX_minimatch_expand_and_get_priority(uint8_t bridge_id, struct cls_rule *ut_cr, struct match *match, unsigned int *priority) {
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = ut_cr;
    args[2] = match;
    args[3] = priority;
    HCALL(ecall_minimatch_expand_and_get_priority, async, NULL, 4, args);*/
    #else
    ECALL(
        ecall_minimatch_expand_and_get_priority,
        bridge_id,
        ut_cr,
        match,
        priority
    );
    #endif
}

uint32_t
SGX_miniflow_expand_and_tag(uint8_t bridge_id, struct cls_rule *ut_cr, struct flow *flow, uint8_t table_id) {
    uint32_t res;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = ut_cr;
    args[2] = flow;
    args[3] = &table_id;
    HCALL(ecall_miniflow_expand_and_tag, async, &res, 4, args);*/
    #else
    ECALL(
        ecall_miniflow_expand_and_tag,
        &res,
        bridge_id,
        ut_cr,
        flow,
        table_id
    );
    #endif
    return res;

}

// FIX ASYNC NEW VALUE
void
SGX_set_evictable(uint8_t bridge_id, struct cls_rule *ut_cr, uint8_t new_value) {
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_voidptr(&pfc, ut_cr) : ut_cr;
    args[2] = async ? next_uint8(&pfc, new_value) : &new_value;
    HCALL(ecall_set_evictable, async, NULL, 3, args);*/
    #else
    ECALL(ecall_set_evictable, bridge_id, ut_cr, new_value);
    #endif
}

bool
SGX_is_evictable(uint8_t bridge_id, struct cls_rule *ut_cr) {
    bool evictable;
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = ut_cr;
    HCALL(ecall_is_evictable, async, &evictable, 2, args);*/
    #else
    ECALL(ecall_is_evictable, &evictable, bridge_id, ut_cr);
    #endif
    return evictable;
}

void
SGX_backup_and_set_evictable(uint8_t bridge_id, struct cls_rule *ut_cr, uint8_t new_value) {
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_voidptr(&pfc, ut_cr) : ut_cr;
    args[2] = async ? next_uint8(&pfc, new_value) : &new_value;
    HCALL(ecall_backup_and_set_evictable, async, NULL, 3, args);*/
    #else
    ECALL(ecall_backup_and_set_evictable, bridge_id, ut_cr, new_value);
    #endif

}

void
SGX_backup_evictable(uint8_t bridge_id, struct cls_rule *ut_cr) {
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_voidptr(&pfc, ut_cr) : ut_cr;
    HCALL(ecall_backup_evictable, async, NULL, 2, args);*/
    #else
    ECALL(ecall_backup_evictable, bridge_id, ut_cr);
    #endif
}

void
SGX_restore_evictable(uint8_t bridge_id, struct cls_rule *ut_cr) {

    #ifdef BATCHING
    // Check if backup action is still in ecall queue, if it is then we can simply discard both the backup ecall and this ecall.
    struct function_call *fcall;
    LIST_FOR_EACH (fcall, list_node, &sm_ctx.hcall.ecall_queue) {
        if(fcall->id == hotcall_ecall_backup_and_set_evictable) {
            list_remove(&fcall->list_node);
            return;
        }
    }
    #endif

    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_voidptr(&pfc, ut_cr) : ut_cr;
    HCALL(ecall_restore_evictable, async, NULL, 2, args);*/
    #else
    ECALL(ecall_restore_evictable, bridge_id, ut_cr);
    #endif
}

void
SGX_rule_update_used(uint8_t bridge_id, struct cls_rule *ut_cr, uint32_t eviction_rule_priority) {
    #ifdef HOTCALL
    /*bool async = ASYNC(true);
    void **args = pfc.args[pfc.idx];
    args[0] = async ? next_uint8(&pfc, bridge_id) : &bridge_id;
    args[1] = async ? next_voidptr(&pfc, ut_cr) : ut_cr;
    args[2] = async ? next_uint32(&pfc, eviction_rule_priority) : &eviction_rule_priority;
    HCALL(ecall_rule_update_used, async, NULL, 3, args);*/
    #else
    ECALL(ecall_rule_update_used, bridge_id, ut_cr, eviction_rule_priority);
    #endif
}

void
SGX_configure_tables(uint8_t bridge_id, int n_tables, uint32_t time_boot_msec, struct ofproto_table_settings *settings, bool *need_to_evict) {
    #ifdef HOTCALL
    /*bool async = ASYNC(false);
    void **args = pfc.args[pfc.idx];
    args[0] = &bridge_id;
    args[1] = &n_tables;
    args[2] = &time_boot_msec;
    args[3] = settings;
    args[4] = need_to_evict;
    HCALL(ecall_configure_tables, async, NULL, 5, args);*/
    #else
    ECALL(ecall_configure_tables, bridge_id, n_tables, time_boot_msec, settings, need_to_evict);
    #endif
}
