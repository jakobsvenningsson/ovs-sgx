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

void
ecall_offset_of(void **ptr, int offset) {
    *ptr = ((char *) *ptr) + offset;
}

static void
wrapper_ecall_evg_add_rule(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_evg_add_rule(
            *(uint8_t *) args[i][0],
            *(uint8_t *) args[i][1],
            (struct cls_rule *) args[i][2],
            (uint32_t *) args[i][3],
            *(uint32_t *) args[i][4]
        );
    }
}

static void
wrapper_ecall_rule_eviction_priority(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[i][n_params - 1]  = rule_eviction_priority(args[i][0], *(uint32_t *) args[i][1]);
    }
}

static void
wrapper_ecall_cls_rule_hash(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[i][n_params - 1]  = ecall_cls_rule_hash(*(uint8_t *) args[i][0], args[i][1], *(uint32_t *) args[i][2]);
    }
}

static void
wrapper_ecall_cls_remove(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_cls_remove(*(uint8_t *) args[i][0], *(uint8_t *) args[i][1], args[i][2]);
    }
}

static void
wrapper_ecall_evg_remove_rule(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_evg_remove_rule(*(uint8_t *) args[i][0], *(uint8_t *) args[i][1], args[i][2]);
    }
}

static void
wrapper_ecall_cr_priority(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(unsigned int *) args[i][n_params - 1] = ecall_cr_priority(*(uint8_t *) args[i][0], args[i][1]);
    }
}

static void
wrapper_ecall_oftable_is_other_table(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[i][n_params - 1] = ecall_oftable_is_other_table(*(uint8_t *) args[i][0], *(uint8_t *) args[i][1]);
    }
}

static void
wrapper_ecall_oftable_update_taggable(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[i][n_params - 1] = ecall_oftable_update_taggable(*(uint8_t *) args[i][0], *(uint8_t *) args[i][1]);
    }
}

static void
wrapper_ecall_oftable_get_flags(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[i][n_params - 1] = ecall_oftable_get_flags(*(uint8_t *) args[i][0], *(uint8_t *) args[i][1]);
    }
}

static void
wrapper_ecall_is_eviction_fields_enabled(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[i][n_params - 1] = ecall_is_eviction_fields_enabled(*(uint8_t *) args[i][0], *(uint8_t *) args[i][1]);
    }
}

static void
wrapper_ecall_oftable_cls_count(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[i][n_params - 1] = ecall_oftable_cls_count(*(uint8_t *) args[i][0], *(uint8_t *) args[i][1]);
    }
}

static void
wrapper_ecall_oftable_mflows(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[i][n_params - 1] = ecall_oftable_mflows(*(uint8_t *) args[i][0], *(uint8_t *) args[i][1]);
    }
}

#define CALL_TABLE_CAPACITY 256

void *call_table[CALL_TABLE_CAPACITY] = {
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
    [hotcall_ecall_oftable_mflows] = wrapper_ecall_oftable_mflows
};


void
batch_execute_function(uint8_t function_id, unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    void (*f)(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]);
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
        .execute_function_legacy = execute_function,
        .execute_function = batch_execute_function,
        .n_spinlock_jobs = 0,
        .ctx = &e_ctx
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
    sgx_table_cls_init(&e_ctx, bridge_id);
    sgx_table_dpif_init(&e_ctx, bridge_id, n_tables);


    #ifdef BATCH_ALLOCATION

    batch_allocator_init(&e_ctx.cr_ba, sizeof(struct sgx_cls_rule));
    batch_allocator_add_block(&e_ctx.cr_ba);

    batch_allocator_init(&e_ctx.evg_ba, sizeof(struct eviction_group));
    batch_allocator_add_block(&e_ctx.evg_ba);

    /*batch_allocator_init(&heap_ba, sizeof(struct heap_node));
    batch_allocator_add_block(&heap_ba);*/

    #endif



}
