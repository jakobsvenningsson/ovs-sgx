#ifndef _H_HOTCALL_PRODUCER_H
#define _H_HOTCALL_PRODUCER_H

#include "common.h"
#include "stdbool.h"
#include <stdlib.h>
#include <string.h>
#include "enclave_u.h"

//void compile_arg_list(void **return_val, argument_list *arg_list, bool has_return, int n_args, ...);
//void make_hotcall(async_ecall * ctx, int function, argument_list * args, void * ret);

#define MAX_FCS 100
#define MAX_TS 100


struct preallocated_function_calls {
    struct function_call fcs[MAX_FCS];
    void *args[MAX_FCS][HOTCALL_MAX_ARG];
    size_t idx;
    size_t len;

    size_t idx_uint8;
    uint8_t uint8_ts[MAX_TS];

    size_t idx_sizet;
    size_t size_ts[MAX_TS];

    size_t idx_unsigned;
    unsigned int unsigned_ts[MAX_TS];

    size_t idx_void;
    void *void_ts[MAX_TS];

    size_t idx_uint32;
    uint32_t uint32_ts[MAX_TS];

    size_t idx_heap_node;
    struct heap_node heap_node_ts[MAX_TS];
};

extern struct preallocated_function_calls pfc;

void prepare_hotcall_function(struct hotcall *hcall, uint8_t function_id, char *fmt, bool async, bool has_return, int n_args, ...);

void make_hotcall(struct hotcall *hcall);

inline size_t
get_n_args(char *fmt) {
    size_t i = 0;
    for (i=0; fmt[i]; fmt[i]!=',' ? i++ : *fmt)
    return i;
}

inline
struct function_call * get_fcall(uint8_t f_id, void *ret, uint8_t n_args, void *args) {
    struct function_call *fcall = &pfc.fcs[pfc.idx++];
    if(pfc.idx == MAX_TS) {
        pfc.idx = 0;
    }
    fcall->id = f_id;
    fcall->args.n_args = n_args;
    fcall->return_value = ret;
    memcpy(fcall->args.args, args, fcall->args.n_args * sizeof(void *));
    return fcall;
}

inline size_t *
next_sizet(size_t x) {
    if(pfc.idx_sizet == MAX_TS) {
        pfc.idx_sizet = 0;
    }
    pfc.size_ts[pfc.idx_sizet] = x;
    return &pfc.size_ts[pfc.idx_sizet++];
}

inline uint8_t *
next_uint8(uint8_t x) {
    if(pfc.idx_uint8 == MAX_TS) {
        pfc.idx_uint8 = 0;
    }
    pfc.uint8_ts[pfc.idx_uint8] = x;
    return &pfc.uint8_ts[pfc.idx_uint8++];
}

inline unsigned int *
next_unsigned(unsigned int x) {
    if(pfc.idx_unsigned == MAX_TS) {
        pfc.idx_unsigned = 0;
    }
    pfc.unsigned_ts[pfc.idx_unsigned] = x;
    return &pfc.unsigned_ts[pfc.idx_unsigned++];
}

inline void *
next_voidptr(void *ptr) {
    if(pfc.idx_void == MAX_TS) {
        pfc.idx_void = 0;
    }
    pfc.void_ts[pfc.idx_void] = ptr;
    return pfc.void_ts[pfc.idx_void++];
}

inline void *
next_uint32(uint32_t x) {
    if(pfc.idx_uint32 == MAX_TS) {
        pfc.idx_uint32 = 0;
    }
    pfc.uint32_ts[pfc.idx_uint32] = x;
    return &pfc.uint32_ts[pfc.idx_uint32++];
}

inline void *
next_heapnode(struct heap_node h) {
    if(pfc.idx_heap_node == MAX_TS) {
        pfc.idx_heap_node = 0;
    }
    pfc.heap_node_ts[pfc.idx_heap_node] = h;
    return &pfc.heap_node_ts[pfc.idx_heap_node++];
}

#endif
