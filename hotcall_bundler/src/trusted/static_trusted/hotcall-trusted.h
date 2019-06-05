#ifndef _H_LIB_HOTCALL_TRUSTED_
#define _H_LIB_HOTCALL_TRUSTED_

#ifdef __cplusplus
extern "C" {
#endif

#include "hotcall_config.h"
#include "hotcall.h"

unsigned int for_loop_indices[3] = { 0 };
unsigned int for_loop_nesting = 0;
static struct hotcall_config *hotcall_config;

void
hotcall_register_config(struct hotcall_config *config);

static inline void
exclude_else_branch(uint8_t *exclude_list, int pos, unsigned int if_len, unsigned int else_len) {
    memset(exclude_list + pos + if_len + 1, 1, else_len);
}

static inline void
exclude_if_branch(uint8_t *exclude_list, int pos, unsigned int if_len) {
    memset(exclude_list + pos + 1, 1, if_len);
}

static inline void
exclude_rest(uint8_t *exclude_list, int pos, int then_branch_len, int else_branch_len, int len) {
    int exclude_start = pos + then_branch_len + else_branch_len + 1;
    memset(exclude_list + exclude_start, 1, len - exclude_start);
}


extern inline void
hotcall_handle_function(struct hotcall_function *fc) {
    void *tmp[fc->args.n_args];
    if(for_loop_nesting > 0) {
        for(int i = 0; i < fc->args.n_args; ++i) {
            tmp[i] = fc->args.args[i];
            fc->args.args[i] = ((int *) fc->args.args[i] + for_loop_indices[for_loop_nesting - 1]);
        }
    }
    hotcall_config->execute_function(fc);
    if(for_loop_nesting > 0) {
        for(int i = 0; i < fc->args.n_args; ++i) {
            fc->args.args[i] = ((int *) fc->args.args[i] + for_loop_indices[for_loop_nesting - 1]);
            fc->args.args[i] = tmp[i];
        }
    }
}

#ifdef __cplusplus
}
#endif

#endif
