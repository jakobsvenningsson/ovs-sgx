#ifndef _H_HOTCALL_FOR
#define _H_HOTCALL_FOR

#include "hotcall_function.h"
#include "hotcall.h"


#define _BEGIN_FOR(SM_CTX, ID, CONFIG) \
    struct for_config CAT2(FOR_CONFIG_,ID) = CONFIG; \
    hotcall_bundle_for_begin(SM_CTX, &CAT2(FOR_CONFIG_,ID))


#define BEGIN_FOR(SM_CTX, CONFIG) \
    _BEGIN_FOR(SM_CTX, UNIQUE_ID, CONFIG)


#define END_FOR(SM_CTX) \
    hotcall_bundle_for_end(SM_CTX)


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
