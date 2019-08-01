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
#include "hotcall-bundler-untrusted.h"
#include "functions.h"
#include "hotcall-hash.h"


/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid        = 0;
static bool enclave_is_initialized = false;
static uint8_t bridge_counter = 0;

static struct shared_memory_ctx sm_ctx;

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


    #ifdef MEMOIZE
    sm_ctx.mem.max_n_function_caches = 256;
    sm_ctx.mem.function_cache_size[hotcall_ecall_oftable_is_readonly] = 10;
    sm_ctx.mem.function_cache_size[hotcall_ecall_oftable_mflows] = 10;
    sm_ctx.mem.function_cache_size[hotcall_ecall_is_eviction_fields_enabled] = 10;
    sm_ctx.mem.function_cache_size[hotcall_ecall_oftable_is_other_table] = 10;
    sm_ctx.mem.function_cache_size[hotcall_ecall_oftable_update_taggable] = 10;
    sm_ctx.mem.function_cache_size[hotcall_ecall_oftable_get_flags] = 10;
    sm_ctx.mem.function_cache_size[hotcall_ecall_cr_priority] = 10;
    sm_ctx.mem.function_cache_size[hotcall_ecall_oftable_cls_count] = 10;
    sm_ctx.mem.function_cache_size[hotcall_ecall_oftable_cls_lookup] = 25;
    sm_ctx.mem.function_cache_size[hotcall_ecall_cls_rule_hash] = 10;
    sm_ctx.mem.function_cache_size[hotcall_ecall_cls_rule_equal] = 10;
    sm_ctx.mem.function_cache_size[hotcall_ecall_minimask_get_vid_mask] = 10;
    sm_ctx.mem.function_cache_size[hotcall_ecall_miniflow_get_vid] = 10;
    #endif

    #ifdef HOTCALL
    if(this_bridge_id == 0) {
        hotcall_init(&sm_ctx, global_eid);
    }
    #endif

    return this_bridge_id;
}


uint32_t
cls_hash(const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id) {
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

void
SGX_readonly_set(uint8_t bridge_id, uint8_t table_id){
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, table_id };
    uint32_t hash = hcall_hash_words(tmp, 2, 0);
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_oftable_set_readonly,
            .memoize = { 0 },
            .memoize_invalidate = { .n_caches_to_invalidate = 1, .caches = {{ hotcall_ecall_oftable_is_readonly, .type = HASH, .invalidate_element = { .hash = hash }}}}
        ),
        VAR(bridge_id, ui8), VAR(table_id, ui8)
    );
    #elif HOTCALL
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_set_readonly), VAR(bridge_id, ui8), VAR(table_id, ui8));
    #elif SGX
    ECALL(ecall_oftable_set_readonly, bridge_id, table_id);
    #endif
}

int
SGX_istable_readonly(uint8_t bridge_id, uint8_t table_id){
    int ecall_return;
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, table_id };
    uint32_t hash = hcall_hash_words(tmp, 2, 0);
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_is_readonly, .has_return = true, .memoize = { .on = true, .return_type = 'd', .hash = hash }),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd'));

    #elif HOTCALL
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_is_readonly, .has_return = true), VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd'));
    #elif SGX
    ECALL(ecall_oftable_is_readonly, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

void
SGX_cls_rule_init(uint8_t bridge_id, struct cls_rule * o_cls_rule,
  const struct match * match, unsigned int priority){
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_rule_init),
        VAR(bridge_id, ui8), PTR(o_cls_rule, 'p'), PTR(match, 'p'), VAR(priority, 'u')
    );
    #elif SGX
    ECALL(ecall_cls_rule_init, bridge_id, o_cls_rule, match, priority);
    #endif

}

int
SGX_cr_rule_overlaps(uint8_t bridge_id, uint8_t table_id, struct cls_rule * ut_cr){
    int ecall_return;
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cr_rule_overlaps, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr, 'p'), VAR(ecall_return, 'd')
    );
    #elif SGX
    ECALL(ecall_cr_rule_overlaps, &ecall_return, bridge_id, table_id, ut_cr);
    #endif
    return ecall_return;
}

void
SGX_cls_rule_destroy(uint8_t bridge_id, struct cls_rule * ut_cr){
    #ifdef MEMOIZE
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_cls_rule_destroy
        ),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p')
    );
    #elif HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_rule_destroy),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p')
    );
    #elif SGX
    ECALL(ecall_cls_rule_destroy, bridge_id, ut_cr);
    #endif
}

uint32_t
SGX_cls_rule_hash(uint8_t bridge_id, const struct cls_rule * ut_cr, uint32_t basis){
    uint32_t ecall_return;
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, (uintptr_t) ut_cr, basis };
    uint32_t hash = hcall_hash_words(tmp, 3, 0);
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_rule_hash, .has_return = true, .memoize = { .on = true, .return_type = 'u', .hash = hash }),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(basis, ui32), VAR(ecall_return, ui32)
    );
    #elif HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_rule_hash, .has_return = true),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(basis, ui32), VAR(ecall_return, ui32)
    );
    #elif SGX
    ECALL(ecall_cls_rule_hash, &ecall_return, bridge_id, ut_cr, basis);
    #endif
    return ecall_return;
}

int
SGX_cls_rule_equal(uint8_t bridge_id, const struct cls_rule * ut_cr_a, const struct cls_rule * ut_cr_b){
    int ecall_return;
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, (uintptr_t) ut_cr_a, (uintptr_t) ut_cr_b };
    uint32_t hash = hcall_hash_words(tmp, 3, 0);
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_rule_equal, .has_return = true, .memoize = { .on = true, .return_type = 'd', .hash = hash }),
        VAR(bridge_id, ui8), PTR(ut_cr_a, 'p'), PTR(ut_cr_b, 'p'), VAR(ecall_return, 'd')
    );
    #elif HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_rule_equal, .has_return = true),
        VAR(bridge_id, ui8), PTR(ut_cr_a, 'p'), PTR(ut_cr_b, 'p'), VAR(ecall_return, 'd')
    );
    #elif SGX
    ECALL(ecall_cls_rule_equal, &ecall_return, bridge_id, ut_cr_a, ut_cr_b);
    #endif
    return ecall_return;
}

struct cls_rule *
SGX_classifier_replace(uint8_t bridge_id, uint8_t table_id, struct cls_rule * ut_cr_insert){
    struct cls_rule *ut_cr_remove;
    #ifdef MEMOIZE
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_oftable_classifier_replace,
            .has_return = true,
            .memoize_invalidate = {
                .n_caches_to_invalidate = 2,
                .caches = {
                    { hotcall_ecall_oftable_cls_count, .type = CLEAR_CACHE },
                    { hotcall_ecall_oftable_cls_lookup, .type = CLEAR_CACHE }
                }
            }
        ),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr_insert, 'p'), VAR(ut_cr_remove, 'p')
    );
    #elif HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_classifier_replace, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr_insert, 'p'), VAR(ut_cr_remove, 'p')
    );
    #elif SGX
    ECALL(ecall_oftable_classifier_replace, &ut_cr_remove, bridge_id, table_id, ut_cr_insert);
    #endif
    return ut_cr_remove;
}

enum oftable_flags
SGX_rule_get_flags(uint8_t bridge_id, uint8_t table_id){
    enum oftable_flags ecall_return;
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, table_id };
    uint32_t hash = hcall_hash_words(tmp, 2, 0);
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_get_flags, .has_return = true, .memoize = { .on = true, .return_type = 'd', .hash = hash }),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd'));

    #elif HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_get_flags, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd')
    );
    #elif SGX
    ECALL(ecall_oftable_get_flags, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

int
SGX_cls_count(uint8_t bridge_id, uint8_t table_id){
    int ecall_return;
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, table_id };
    uint32_t hash = hcall_hash_words(tmp, 2, 0);
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_cls_count, .has_return = true, .memoize = { .on = true, .return_type = 'd', .hash = hash }),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd'));

    #elif HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_cls_count, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd')
    );
    #elif SGX
    ECALL(ecall_oftable_cls_count, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

int
SGX_eviction_fields_enable(uint8_t bridge_id, uint8_t table_id){
    int ecall_return;
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, table_id };
    uint32_t hash = hcall_hash_words(tmp, 2, 0);
    HCALL(
        CONFIG(.function_id = hotcall_ecall_is_eviction_fields_enabled, .has_return = true, .memoize = { .on = true, .return_type = 'd', .hash = hash }),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd')
    );
    #elif HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_is_eviction_fields_enabled, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd')
    );
    #elif SGX
    ECALL(ecall_is_eviction_fields_enabled, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

void
SGX_evg_add_rule(uint8_t bridge_id, uint8_t table_id, struct cls_rule * ut_cr, uint32_t priority,
  uint32_t rule_evict_prioriy){
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_evg_add_rule),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr, 'p'), VAR(priority, ui32), VAR(rule_evict_prioriy, ui32)
    );
    #elif SGX
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
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_evg_remove_rule
        ),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr, 'p')
    );
    #elif SGX
    ECALL(ecall_evg_remove_rule, bridge_id, table_id, ut_cr);
    #endif
}

void
SGX_cls_remove(uint8_t bridge_id, uint8_t table_id, struct cls_rule * ut_cr){
    #ifdef MEMOIZE
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_cls_remove,
            .memoize_invalidate = { .n_caches_to_invalidate = 1, .caches = {
                { hotcall_ecall_oftable_cls_count, .type = CLEAR_CACHE },
                { hotcall_ecall_oftable_cls_lookup, .type = VALUE, .invalidate_element = { .hash = (uintptr_t) ut_cr }}
            }}
        ),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr, 'p')
    );
    #elif HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_remove),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr, 'p')
    );
    #elif SGX
    ECALL(ecall_cls_remove, bridge_id, table_id, ut_cr);
    #endif
}

void
SGX_choose_rule_to_evict(uint8_t bridge_id, uint8_t table_id, struct cls_rule ** ut_cr_victim){
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_choose_rule_to_evict),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr_victim, 'p')
    );
    #elif SGX
    ECALL(ecall_choose_rule_to_evict, bridge_id, table_id, ut_cr_victim);
    #endif
}

void
SGX_choose_rule_to_evict_p(uint8_t bridge_id, uint8_t table_id, struct cls_rule **ut_cr_victim, struct cls_rule *ut_cr_replacer){
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_choose_rule_to_evict_p),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(ut_cr_victim, 'p'), PTR(ut_cr_replacer, 'p')
    );
    #elif SGX
    ECALL(ecall_choose_rule_to_evict_p, bridge_id, table_id, ut_cr_victim, ut_cr_replacer);
    #endif
}

unsigned int
SGX_table_mflows(uint8_t bridge_id, uint8_t table_id){
    unsigned int ecall_return;
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, table_id };
    uint32_t hash = hcall_hash_words(tmp, 2, 0);
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_mflows, .has_return = true, .memoize = { .on = true, .return_type = 'u', .hash = hash }),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'u'));

    #elif HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_mflows, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'u')
    );
    #elif SGX
    ECALL(ecall_oftable_mflows, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

void
SGX_table_mflows_set(uint8_t bridge_id, uint8_t table_id, unsigned int new_value){
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, table_id };
    uint32_t hash = hcall_hash_words(tmp, 2, 0);
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_oftable_mflows_set,
            .memoize = { 0 },
            .memoize_invalidate = { .n_caches_to_invalidate = 1, .caches = {{ hotcall_ecall_oftable_mflows, .type = HASH, .invalidate_element = { .hash = hash }}}}
        ),
        VAR(bridge_id, ui8),
        VAR(table_id, ui8),
        VAR(new_value, 'u')
    );
    #elif HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_mflows_set),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(new_value, 'u')
    );
    #elif SGX
    ECALL(ecall_oftable_mflows_set, bridge_id, table_id, new_value);
    #endif
}

void
SGX_minimatch_expand(uint8_t bridge_id, struct cls_rule * ut_cr, struct match * dst){
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_minimatch_expand),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), PTR(dst, 'p')
    );
    #elif SGX
    ECALL(ecall_minimatch_expand, bridge_id, ut_cr, dst);
    #endif
}

unsigned int
SGX_cr_priority(uint8_t bridge_id, const struct cls_rule * ut_cr){
    unsigned int ecall_return;
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, (uintptr_t) ut_cr };
    uint32_t hash = hcall_hash_words(tmp, 2, 0);
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cr_priority, .has_return = true, .memoize = { .on = true, .return_type = 'u', .hash = hash }),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(ecall_return, 'u'));

    #elif HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cr_priority, .has_return = true),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(ecall_return, 'u')
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_cls_find_match_exactly),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(target, 'p'), VAR(priority, 'u'), PTR(ut_cr, 'p')
    );
    #elif SGX
    ECALL(ecall_oftable_cls_find_match_exactly, bridge_id, table_id, target, priority, ut_cr);
    #endif
}

size_t
SGX_collect_rules_loose_c(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id, const struct match * match){
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_rules_loose_c, .has_return = true),
        VAR(bridge_id, ui8), VAR(ofproto_n_tables, 'd'), VAR(table_id, ui8), PTR(match, 'p'), VAR(n, 'u')
    );
    #elif SGX
    ECALL(ecall_collect_rules_loose_c, &n, bridge_id, ofproto_n_tables, table_id, match);
    #endif
    return n;
}

size_t
SGX_collect_rules_loose_r(uint8_t bridge_id,
                int ofproto_n_tables,
                struct cls_rule ** buf,
                int elem,
                uint8_t table_id,
                const struct match * match)
{
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_rules_loose_r, .has_return = true),
        VAR(bridge_id, ui8), VAR(ofproto_n_tables, 'd'), PTR(buf, 'p'), VAR(elem, 'd'), VAR(table_id, ui8), PTR(match, 'p'), VAR(n, 'u')
    );
    #elif SGX
    ECALL(ecall_collect_rules_loose_r, &n, bridge_id, ofproto_n_tables, buf, elem, table_id, match);
    #endif
    return n;
}

size_t
SGX_collect_rules_strict_c(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id, const struct match * match, unsigned int priority){
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_rules_strict_c, .has_return = true),
        VAR(bridge_id, ui8), VAR(ofproto_n_tables, 'd'), VAR(table_id, ui8), PTR(match, 'p'), VAR(priority, 'u'), VAR(n, 'u')
    );
    #elif SGX
    ECALL(ecall_collect_rules_strict_c, &n, bridge_id, ofproto_n_tables, table_id, match, priority);
    #endif
    return n;
}

size_t
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_rules_strict_r, .has_return = true),
        VAR(bridge_id, ui8), VAR(ofproto_n_tables, 'd'), PTR(buf, 'p'), VAR(elem, 'd'), VAR(table_id, ui8), PTR(match, 'p'), VAR(priority, 'u'), VAR(n, 'u')
    );
    #elif SGX
    ECALL(ecall_collect_rules_strict_r, &n, bridge_id, ofproto_n_tables, buf, elem, table_id, match, priority);
    #endif
    return n;
}

unsigned int
SGX_oftable_enable_eviction_c(uint8_t bridge_id, uint8_t table_id){
    unsigned int n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_enable_eviction_c, .has_return = true),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(n, 'u')
    );
    #elif SGX
    ECALL(ecall_oftable_enable_eviction_c, &n, bridge_id, table_id);
    #endif
    return n;
}

// 24.2 Request
unsigned int
SGX_oftable_enable_eviction_r(uint8_t bridge_id, struct cls_rule ** buf, int elem, uint8_t table_id, int *n_cr_rules){
    unsigned int n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_enable_eviction_r, .has_return = true),
        VAR(bridge_id, ui8), PTR(buf), VAR(elem, 'd'), VAR(table_id, ui8), PTR(n_cr_rules, 'd'), VAR(n, 'u')
    );
    #elif SGX
    ECALL(ecall_oftable_enable_eviction_r, &n, bridge_id, buf, elem, table_id, n_cr_rules);
    #endif
    return n;
}

size_t
SGX_collect_ofmonitor_util_c(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id, const struct minimatch * match){
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_ofmonitor_util_c, .has_return = true),
        VAR(bridge_id, ui8), VAR(ofproto_n_tables, 'd'), VAR(table_id, ui8), PTR(match), VAR(n, 'u')
    );
    #elif SGX
    ECALL(ecall_collect_ofmonitor_util_c, &n, bridge_id, ofproto_n_tables, table_id, match);
    #endif
    return n;
}

size_t
SGX_collect_ofmonitor_util_r(uint8_t bridge_id, int ofproto_n_tables, struct cls_rule ** buf, int elem, uint8_t table_id,
  const struct minimatch * match){
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_ofmonitor_util_r, .has_return = true),
        VAR(bridge_id, ui8), VAR(ofproto_n_tables, 'd'), PTR(buf), VAR(elem, 'd'), VAR(table_id, ui8), PTR(match), VAR(n, 'u')
    );
    #elif SGX
    ECALL(ecall_collect_ofmonitor_util_r, &n, bridge_id, ofproto_n_tables, buf, elem, table_id, match);
    #endif
    return n;
}

// 25. One Part of Enable_eviction
void
SGX_oftable_enable_eviction(uint8_t bridge_id, uint8_t table_id, const struct mf_subfield * fields, size_t n_fields,
  uint32_t random_v, bool * no_change){
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_oftable_enable_eviction
        ),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(fields), VAR(n_fields, 'u'), VAR(random_v, ui32), PTR(no_change)
    );
    #elif SGX
    ECALL(ecall_oftable_enable_eviction, bridge_id, table_id, fields, n_fields, random_v, no_change);
    #endif
}

void
SGX_oftable_disable_eviction(uint8_t bridge_id, uint8_t table_id){
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_oftable_disable_eviction
        ),
        VAR(bridge_id, ui8), VAR(table_id, ui8));
    #elif SGX
    ECALL(ecall_oftable_disable_eviction, bridge_id, table_id);
    #endif
}

void
SGX_ofproto_destroy(uint8_t bridge_id){
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_ofproto_destroy
        ),
        VAR(bridge_id, ui8));
    #elif SGX
    ECALL(ecall_ofproto_destroy, bridge_id);
    #endif
}

unsigned int
SGX_total_rules(uint8_t bridge_id){
    unsigned int n_rules;
    #ifdef HOTCALL
    HCALL(CONFIG(.function_id = hotcall_ecall_total_rules, .has_return = true), VAR(bridge_id, ui8), VAR(n_rules, 'u'));
    #elif SGX
    ECALL(ecall_total_rules, &n_rules, bridge_id);
    #endif
    return n_rules;
}

// 28 Copy the name of the table
void
SGX_table_name(uint8_t bridge_id, uint8_t table_id, char * buf, size_t len){
    #ifdef HOTCALL
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_name), VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(buf, 'p'), VAR(len, 'u'));
    #elif SGX
    ECALL(ecall_oftable_name, bridge_id, table_id, buf, len);
    #endif
}

// 29 loose_match
int
SGX_cls_rule_is_loose_match(uint8_t bridge_id, struct cls_rule * ut_cr, const struct minimatch * criteria){
    int result;
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_cls_rule_is_loose_match,
            .has_return = true
        ), VAR(bridge_id, ui8), PTR(ut_cr), PTR(criteria), VAR(result, 'd'));
    #elif SGX
    ECALL(ecall_cls_rule_is_loose_match, &result, bridge_id, ut_cr, criteria);
    #endif
    return result;
}

// 30. Dependencies for ofproto_flush__
size_t
SGX_flush_c(uint8_t bridge_id){
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_flush_c,
            .has_return = true
        ),
        VAR(bridge_id, ui8), VAR(n, 'u'));
    #elif SGX
    ECALL(ecall_flush_c, &n, bridge_id);
    #endif
    return n;
}

// 30.1
void
SGX_flush_r(uint8_t bridge_id, struct cls_rule ** buf, int elem){
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_flush_r,
            .has_return = true
        ),
        VAR(bridge_id, ui8), PTR(buf), VAR(elem, 'd'), VAR(n, 'u'));
    #elif SGX
    ECALL(ecall_flush_r, &n, bridge_id, buf, elem);
    #endif
}

// 31 Dependencies for ofproto_get_all_flows
size_t
SGX_flow_stats_c(uint8_t bridge_id){
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_flow_stats_c,
            .has_return = true
        ),
        VAR(bridge_id, ui8), VAR(n, 'u'));
    #elif SGX
    ECALL(ecall_flow_stats_c, &n, bridge_id);
    #endif
    return n;
}

// 31.2 REQUEST
size_t
SGX_flow_stats_r(uint8_t bridge_id, struct cls_rule ** buf, int elem){
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_flow_stats_r,
            .has_return = true
        ),
        VAR(bridge_id, ui8), PTR(buf), VAR(elem, 'd'), VAR(n, 'u'));
    #elif SGX
    ECALL(ecall_flow_stats_r, &n, bridge_id, buf, elem);
    #endif
    return n;
}

// 33 Classifier_lookup
struct cls_rule *
SGX_cls_lookup(uint8_t bridge_id, uint8_t table_id, const struct flow *flow,
  struct flow_wildcards * wc) {
    struct cls_rule *ut_cr;
    #ifdef MEMOIZE1
    uint32_t hash = cls_hash(flow, wc, bridge_id, table_id);
    uint32_t hash_writeback;
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_oftable_cls_lookup,
            .has_return = true,
            .memoize = { .on = true, .return_type = 'p', .manual_update = true, .hash = hash }
        ),
        VAR(bridge_id, ui8),
        VAR(table_id, ui8),
        PTR(flow, 'p'),
        PTR(wc, 'p'),
        VAR(hash_writeback, 'u'),
        VAR(ut_cr, 'p')
    );
    #elif HOTCALL
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_cls_lookup, .has_return = true), VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(flow, 'p'), PTR(wc, 'p'), VAR(ut_cr, 'p'));
    #elif SGX
    ECALL(ecall_oftable_cls_lookup, &ut_cr, bridge_id, table_id, flow, wc);
    #endif
    return ut_cr;
}

// Dependencies for destroy
size_t
SGX_dpif_destroy_c(uint8_t bridge_id){
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_dpif_destroy_c,
            .has_return = true
        ),
        VAR(bridge_id, ui8), VAR(n, 'u'));
    #elif SGX
    ECALL(ecall_dpif_destroy_c, &n, bridge_id);;
    #endif
    return n;
}

// 2.
void
SGX_dpif_destroy_r(uint8_t bridge_id, struct cls_rule ** buf, int elem){
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_dpif_destroy_r, .has_return = true
        ),
        VAR(bridge_id, ui8), PTR(buf), VAR(elem, 'd'), VAR(n, 'u'));
    #elif SGX
    ECALL(ecall_dpif_destroy_r, &n, bridge_id, buf, elem);
    #endif
}

unsigned int
SGX_cls_rule_format(uint8_t bridge_id, const struct cls_rule * ut_cr, struct match *megamatch){
    unsigned int priority;
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_cls_rule_format,
            .has_return = true
        ),
        VAR(bridge_id, ui8), PTR(ut_cr), PTR(megamatch), VAR(priority, 'u'));
    #elif SGX
    ECALL(ecall_cls_rule_format, &priority, bridge_id, ut_cr, megamatch);
    #endif
    return priority;
}

void
SGX_miniflow_expand(uint8_t bridge_id, struct cls_rule * ut_cr, struct flow * flow){
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_miniflow_expand
        ),
    VAR(bridge_id, ui8), PTR(ut_cr, 'p'), PTR(flow, 'p'));
    #elif SGX
    ECALL(ecall_miniflow_expand, bridge_id, ut_cr, flow);
    #endif
}

uint32_t
SGX_rule_calculate_tag(uint8_t bridge_id, struct cls_rule * ut_cr, const struct flow * flow, uint8_t table_id){
    uint32_t hash;
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_rule_calculate_tag,
            .has_return = true
        ),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), PTR(flow, 'p'), VAR(table_id, ui8), VAR(hash, ui32));
    #elif SGX
    ECALL(ecall_rule_calculate_tag, &hash, bridge_id, ut_cr, flow, table_id);
    #endif
    return hash;
}


// 2.
int
SGX_table_update_taggable(uint8_t bridge_id, uint8_t table_id){
    int ecall_return;
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, table_id };
    uint32_t hash = hcall_hash_words(tmp, 2, 0);
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_oftable_update_taggable,
            .has_return = true,
            .memoize = { .on = true, .return_type = 'd', .hash = hash }
            //.memoize_invalidate = { .n_caches_to_invalidate = 1, .caches = {{ hotcall_ecall_oftable_cls_lookup, .type = CLEAR_CACHE }}}
        ),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd'));

    #elif HOTCALL
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_update_taggable, .has_return = true), VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd'));
    #elif SGX
    ECALL(ecall_oftable_update_taggable, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

// 3.
int
SGX_is_sgx_other_table(uint8_t bridge_id, uint8_t table_id){
    int ecall_return;
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, table_id };
    uint32_t hash = hcall_hash_words(tmp, 2, 0);
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_oftable_is_other_table,
            .has_return = true,
            .memoize = { .on = true, .return_type = 'd', .hash = hash }
        ),
        VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd'));

    #elif HOTCALL
    HCALL(CONFIG(.function_id = hotcall_ecall_oftable_is_other_table, .has_return = true), VAR(bridge_id, ui8), VAR(table_id, ui8), VAR(ecall_return, 'd'));
    #elif SGX
    ECALL(ecall_oftable_is_other_table, &ecall_return, bridge_id, table_id);
    #endif
    return ecall_return;
}

// 4
uint32_t
SGX_rule_calculate_tag_s(uint8_t bridge_id, uint8_t table_id, const struct flow * flow){
    uint32_t hash;
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_rule_calculate_tag_s,
            .has_return = true
        ),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(flow, 'p'), VAR(hash, ui32));
    #elif SGX
    ECALL(ecall_rule_calculate_tag_s, &hash, bridge_id, table_id, flow);
    #endif
    return hash;
}

void
sgx_oftable_check_hidden(uint8_t bridge_id){
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_oftable_hidden_check
        ),
        VAR(bridge_id, ui8));
    #elif SGX
    ECALL(ecall_oftable_hidden_check, bridge_id);
    #endif
}

void
SGX_oftable_set_name(uint8_t bridge_id, uint8_t table_id, char * name) {
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_oftable_set_name
        ),
        VAR(bridge_id, ui8), VAR(table_id, ui8), PTR(name, 'p'));
    #elif SGX
    ECALL(ecall_oftable_set_name, bridge_id, table_id, name);
    #endif
}

// These functions are going to be used by ofopgroup_complete
uint16_t
SGX_minimask_get_vid_mask(uint8_t bridge_id, struct cls_rule * ut_cr){
    uint16_t ecall_return;
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, (uintptr_t) ut_cr };
    uint32_t hash = hcall_hash_words(tmp, 2, 0);
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_minimask_get_vid_mask,
            .has_return = true,
            .memoize = { .on = true, .return_type = ui16, .hash = hash}
        ),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(ecall_return, ui16));
    #elif HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_minimask_get_vid_mask,
            .has_return = true
        ),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(ecall_return, ui16));
    #elif SGX
    ECALL(ecall_minimask_get_vid_mask, &ecall_return, bridge_id, ut_cr);
    #endif
    return (uint16_t) ecall_return;
}

uint16_t
SGX_miniflow_get_vid(uint8_t bridge_id, struct cls_rule * ut_cr){
    uint16_t ecall_return;
    #ifdef MEMOIZE
    uint32_t tmp[] = { bridge_id, (uintptr_t) ut_cr };
    uint32_t hash = hcall_hash_words(tmp, 2, 0);
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_miniflow_get_vid,
            .has_return = true,
            .memoize = { .on = true, .return_type = ui16, .hash = hash}
        ),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(ecall_return, ui16));
    #elif HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_miniflow_get_vid,
            .has_return = true
        ),
        VAR(bridge_id, ui8), PTR(ut_cr, 'p'), VAR(ecall_return, ui16));
    #elif SGX
    ECALL(ecall_miniflow_get_vid, &ecall_return, bridge_id, ut_cr);
    #endif
    return ecall_return;
}

// These functions are depencencies for ofproto_get_vlan_usage
// 1. Count
size_t
SGX_ofproto_get_vlan_usage_c(uint8_t bridge_id){
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_ofproto_get_vlan_c, .has_return = true
        ),
        VAR(bridge_id, ui8), VAR(n, 'd'));
    #elif SGX
    ECALL(ecall_ofproto_get_vlan_c, &n, bridge_id);
    #endif
    return n;
}

// 2. Allocate
void
SGX_ofproto_get_vlan_usage__r(uint8_t bridge_id, uint16_t * buf, int elem){
    size_t n;
    #ifdef HOTCALL
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_ofproto_get_vlan_r,
            .has_return = true
        ),
        VAR(bridge_id, ui8), PTR(buf, ui16), VAR(elem, 'd'), VAR(n, 'u'));
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_get_cls_rules, .has_return = true),
        VAR(bridge_id, ui8),
        VAR(table_id, ui8),
        VAR(start_index, 'u'),
        VAR(end_index, 'u'),
        PTR(buf),
        VAR(buf_size, 'u'),
        PTR(n_rules),
        VAR(n, 'u')
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_ofproto_get_vlan_usage, .has_return = true),
        VAR(bridge_id, ui8),
        VAR(buf_size, 'u'),
        PTR(vlan_buffer),
        VAR(start_index, 'u'),
        VAR(end_index, 'u'),
        PTR(n_vlan),
        VAR(n, 'u')
    );
    #elif SGX
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
    #ifdef MEMOIZE111
    HCALL(
        CONFIG(
            .function_id = hotcall_ecall_ofproto_flush,
            .has_return = true
            //.memoize_invalidate = { .n_caches_to_invalidate = 1, .caches = {{ hotcall_ecall_oftable_cls_lookup, .type = CLEAR_CACHE }}}
        ),
        VAR(bridge_id, ui8),
        PTR(ut_crs),
        PTR(hashes),
        VAR(buf_size, 'u'),
        VAR(start_index, 'u'),
        VAR(end_index, 'u'),
        PTR(n_rules),
        VAR(n, 'u')
    );
    #elif HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_ofproto_flush, .has_return = true),
        VAR(bridge_id, ui8),
        PTR(ut_crs),
        PTR(hashes),
        VAR(buf_size, 'u'),
        VAR(start_index, 'u'),
        VAR(end_index, 'u'),
        PTR(n_rules),
        VAR(n, 'u')
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_ofproto_evict, .has_return = true),
        VAR(bridge_id, ui8),
        VAR(ofproto_n_tables, 'd'),
        PTR(hashes),
        PTR(ut_crs),
        PTR(eviction_is_hidden),
        VAR(buf_size, 'u'),
        PTR(n_evictions),
        VAR(n, 'u')
    );
    #elif SGX
    ECALL(ecall_ofproto_evict, &n, bridge_id, ofproto_n_tables, hashes, ut_crs, eviction_is_hidden, buf_size, n_evictions);
    #endif
    return n;
}
void
SGX_add_flow(uint8_t bridge_id,
			 uint8_t table_id,
			 struct cls_rule *cr,
             struct cls_rule **victim,
             struct cls_rule **evict,
			 struct match *match,
             uint32_t *evict_rule_hash,
            // uint16_t *vid,
            // uint16_t *vid_mask,
			 unsigned int priority,
			 uint16_t flags,
			 uint32_t rule_eviction_priority,
             struct cls_rule **pending_deletions,
             int n_pending,
             //bool has_timeout,
             uint16_t *state,
            //int *table_update_taggable,
             unsigned int *evict_priority)
 {

     #ifdef HOTCALL

     HCALL(
         CONFIG(.function_id = hotcall_ecall_add_flow),
         VAR(bridge_id, ui8),
         VAR(table_id, ui8),
         PTR(cr),
         PTR(victim),
         PTR(evict),
         PTR(match),
         PTR(evict_rule_hash),
         VAR(priority, 'u'),
         VAR(flags, ui16),
         VAR(rule_eviction_priority, ui32),
         PTR(pending_deletions),
         VAR(n_pending, 'd'),
         PTR(state),
         PTR(evict_priority)
     );
     #elif SGX
     ECALL(
         ecall_add_flow,
         bridge_id,
         table_id,
         cr,
         victim,
         evict,
         match,
         evict_rule_hash,
         priority,
         flags,
         rule_eviction_priority,
         pending_deletions,
         n_pending,
         state,
         evict_priority
     );
     #endif
 }

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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_rules_strict, .has_return = true),
        VAR(bridge_id, ui8),
        VAR(table_id, ui8),
        VAR(n_tables, 'd'),
        PTR(match),
        VAR(priority, 'u'),
        VAR(cookie, ui64),
        VAR(cookie_mask, ui64),
        VAR(out_port, ui16),
        PTR(cls_rule_buffer),
        PTR(ofproto_postpone),
        VAR(buffer_size, 'u'),
        VAR(n, 'u')
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_rules_loose, .has_return = true),
        VAR(bridge_id, ui8),
        VAR(table_id, ui8),
        VAR(ofproto_n_tables, 'd'),
        VAR(start_index, 'u'),
        PTR(match),
        PTR(cls_rule_buffer),
        VAR(buffer_size, 'u'),
        VAR(cookie, ui64),
        VAR(cookie_mask, ui64),
        VAR(out_port, ui16),
        PTR(postpone),
        PTR(n_rules),
        VAR(n, 'u')
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_delete_flows_loose, .has_return = true),
        VAR(bridge_id, ui8),
        VAR(table_id, ui8),
        VAR(n_tables, 'd'),
        VAR(offset, 'u'),
        PTR(match),
        VAR(cookie, ui64),
        VAR(cookie_mask, ui64),
        VAR(out_port, ui16),
        PTR(cls_rule_buffer),
        PTR(rule_hashes),
        PTR(rule_priorities),
        PTR(matches),
        VAR(buffer_size, 'u'),
        PTR(rule_is_modifiable),
        PTR(rule_is_hidden),
        PTR(ofproto_postpone),
        PTR(n_rules),
        VAR(n, 'u')
    );
    #elif SGX
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
     HCALL(
         CONFIG(.function_id = hotcall_ecall_delete_flows_strict, .has_return = true),
         VAR(bridge_id, ui8),
         VAR(table_id, ui8),
         VAR(ofproto_n_tables, 'd'),
         PTR(match),
         VAR(priority, 'u'),
         VAR(cookie, ui64),
         VAR(cookie_mask, ui64),
         VAR(out_port, ui16),
         PTR(cls_rule_buffer),
         PTR(rule_hashes),
         PTR(rule_priorities),
         PTR(matches),
         PTR(rule_is_modifiable),
         PTR(rule_is_hidden),
         PTR(ofproto_postpone),
         VAR(buffer_size, 'u'),
         VAR(n, 'u')
     );
     #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_modify_flows_strict, .has_return = true),
        VAR(bridge_id, ui8),
        VAR(table_id, ui8),
        VAR(ofproto_n_tables, 'd'),
        PTR(match),
        VAR(priority, 'u'),
        VAR(cookie, ui64),
        VAR(cookie_mask, ui64),
        VAR(out_port, ui16),
        PTR(cls_rule_buffer),
        PTR(rule_is_modifiable),
        PTR(rule_is_hidden),
        PTR(ofproto_postpone),
        VAR(buffer_size, 'u'),
        VAR(n, 'u')
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_modify_flows_loose, .has_return = true),
        VAR(bridge_id, ui8),
        VAR(table_id, ui8),
        VAR(n_tables, 'd'),
        VAR(offset, 'u'),
        PTR(match),
        PTR(cr_rules),
        VAR(buffer_size, 'u'),
        VAR(cookie, ui64),
        VAR(cookie_mask, ui64),
        VAR(out_port, ui16),
        PTR(rule_is_mod),
        PTR(rule_is_hidden),
        PTR(postpone),
        PTR(n_rules),
        VAR(n, 'u')
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_configure),
        VAR(bridge_id, ui8),
        VAR(table_id, ui8),
        PTR(name),
        VAR(max_flows, 'u'),
        PTR(groups),
        VAR(n_groups, 'u'),
        VAR(time_boot_msec, 'u'),
        PTR(need_to_evict),
        PTR(is_read_only)
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_need_to_evict, .has_return = true),
        VAR(bridge_id, ui8),
        VAR(table_id, ui8),
        VAR(evict, 'b')
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_collect_rules_loose_stats_request, .has_return = true),
        VAR(bridge_id, ui8),
        VAR(table_id, ui8),
        VAR(n_tables, 'u'),
        VAR(start_index, 'u'),
        VAR(buffer_size, 'u'),
        PTR(match),
        PTR(ut_crs),
        PTR(matches),
        PTR(priorities),
        PTR(n_rules),
        VAR(n, 'u')
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_ofproto_rule_send_removed),
        VAR(bridge_id, ui8),
        PTR(cr),
        PTR(match),
        PTR(priority),
        PTR(rule_is_hidden)
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_oftable_remove_rules),
        VAR(bridge_id, ui8),
        PTR(table_ids),
        PTR(ut_crs),
        PTR(is_hidden),
        VAR(n_rules, 'u')
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_cls_rules_format),
        VAR(bridge_id, ui8),
        PTR(ut_crs),
        PTR(megamatches),
        VAR(n, 'u')
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_minimatch_expand_and_get_priority),
        VAR(bridge_id, ui8),
        PTR(ut_cr),
        PTR(match),
        PTR(priority)
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_miniflow_expand_and_tag, .has_return = true),
        VAR(bridge_id, ui8),
        PTR(ut_cr),
        PTR(flow),
        VAR(table_id, ui8),
        VAR(res, 'u')
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_set_evictable),
        VAR(bridge_id, ui8),
        PTR(ut_cr),
        VAR(new_value, ui8)
    );
    #elif SGX
    ECALL(ecall_set_evictable, bridge_id, ut_cr, new_value);
    #endif
}

bool
SGX_is_evictable(uint8_t bridge_id, struct cls_rule *ut_cr) {
    bool evictable;
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_is_evictable, .has_return = true),
        VAR(bridge_id, ui8),
        PTR(ut_cr),
    );
    #elif SGX
    ECALL(ecall_is_evictable, &evictable, bridge_id, ut_cr);
    #endif
    return evictable;
}

void
SGX_backup_and_set_evictable(uint8_t bridge_id, struct cls_rule *ut_cr, uint8_t new_value) {
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_backup_and_set_evictable),
        VAR(bridge_id, ui8),
        PTR(ut_cr),
        VAR(new_value, ui8),
    );
    #elif SGX
    ECALL(ecall_backup_and_set_evictable, bridge_id, ut_cr, new_value);
    #endif

}

void
SGX_backup_evictable(uint8_t bridge_id, struct cls_rule *ut_cr) {
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_backup_evictable),
        VAR(bridge_id, ui8),
        PTR(ut_cr)
    );
    #elif SGX
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
    HCALL(
        CONFIG(.function_id = hotcall_ecall_restore_evictable),
        VAR(bridge_id, ui8),
        PTR(ut_cr)
    );
    #elif SGX
    ECALL(ecall_restore_evictable, bridge_id, ut_cr);
    #endif
}

void
SGX_rule_update_used(uint8_t bridge_id, struct cls_rule *ut_cr, uint32_t eviction_rule_priority) {
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_rule_update_used),
        VAR(bridge_id, ui8),
        PTR(ut_cr),
        VAR(eviction_rule_priority, 'u')
    );
    #elif SGX
    ECALL(ecall_rule_update_used, bridge_id, ut_cr, eviction_rule_priority);
    #endif
}

void
SGX_configure_tables(uint8_t bridge_id, int n_tables, uint32_t time_boot_msec, struct ofproto_table_settings *settings, bool *need_to_evict) {
    #ifdef HOTCALL
    HCALL(
        CONFIG(.function_id = hotcall_ecall_configure_tables),
        VAR(bridge_id, ui8),
        VAR(n_tables, 'd'),
        VAR(time_boot_msec, 'u'),
        PTR(settings),
        PTR(need_to_evict)
    );
    #elif SGX
    ECALL(ecall_configure_tables, bridge_id, n_tables, time_boot_msec, settings, need_to_evict);
    #endif
}
