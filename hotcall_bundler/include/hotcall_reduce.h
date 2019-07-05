#ifndef _H_HOTCALL_REDUCE
#define _H_HOTCALL_REDUCE

#include "hotcall_utils.h"
#include "hotcall_function.h"

#define _REDUCE(SM_CTX, ID, CONFIG, ...) \
    struct parameter CAT2(REDUCE_ARG_,ID)[] = { \
        __VA_ARGS__\
    }; \
    struct reduce_config CAT2(REDUCE_CONFIG_,ID) = CONFIG;\
    struct ecall_queue_item CAT2(QUEUE_ITEM_, ID) = { 0 }; \
    CAT2(REDUCE_CONFIG_,ID).n_params = sizeof(CAT2(REDUCE_ARG_,ID))/sizeof(struct parameter);\
    hotcall_enqueue_item(SM_CTX, QUEUE_ITEM_TYPE_REDUCE, &CAT2(REDUCE_CONFIG_,ID), CAT2(REDUCE_ARG_, ID), &CAT2(QUEUE_ITEM_, ID))

#define REDUCE(CONFIG, ...) \
    _REDUCE(_sm_ctx, UNIQUE_ID, CONFIG, __VA_ARGS__)

struct reduce_config {
    const uint8_t function_id;
    const char op;
    unsigned int n_params;
};

struct hotcall_reduce {
    struct parameter *params;
    struct reduce_config *config;
};

#endif
