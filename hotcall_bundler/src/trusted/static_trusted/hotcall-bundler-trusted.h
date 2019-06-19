#ifndef _H_LIB_HOTCALL_TRUSTED_
#define _H_LIB_HOTCALL_TRUSTED_

#ifdef __cplusplus
extern "C" {
#endif

#include "hotcall_config.h"

#define SWITCH_DEFAULT_REACHED printf("Default reached at %s %d\n", __FILE__, __LINE__);
#define MAX_LOOP_RECURSION 3

struct loop_stack_item {
    unsigned int body_len;
    unsigned int index;
    unsigned int n_iters;
    unsigned int offset;
};

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


#ifdef __cplusplus
}
#endif

#endif
