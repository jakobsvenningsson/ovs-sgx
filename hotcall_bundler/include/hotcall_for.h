#ifndef _H_HOTCALL_FOR
#define _H_HOTCALL_FOR

#include "hotcall_utils.h"
#include "hotcall_function.h"

#define _BEGIN_FOR(SM_CTX, ID, CONFIG) \
    struct for_config CAT2(FOR_CONFIG_,ID) = CONFIG; \
    if(!(SM_CTX)->hcall.batch.ignore_hcalls) { \
        hotcall_enqueue_item(SM_CTX, QUEUE_ITEM_TYPE_FOR_BEGIN, &CAT2(FOR_CONFIG_,ID), NULL);\
    }


#define BEGIN_FOR(CONFIG) \
    _BEGIN_FOR(_sm_ctx, UNIQUE_ID, CONFIG)


#define END_FOR() \
    if(!(_sm_ctx)->hcall.batch.ignore_hcalls) { \
        hotcall_enqueue_item(_sm_ctx, QUEUE_ITEM_TYPE_FOR_END, NULL, NULL);\
    }


struct for_config {
    unsigned int *n_iters;
    void *iter[5];
    unsigned int n_iter_variables;
    void *tmp_storage[5];
    unsigned int body_len;
    bool loop_in_process;
};

struct hotcall_for_start {
    struct for_config *config;
};

#endif
