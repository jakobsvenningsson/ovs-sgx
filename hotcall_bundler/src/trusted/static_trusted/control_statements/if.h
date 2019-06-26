#ifndef _H_TRUSTED_HOTCALL_IF_
#define _H_TRUSTED_HOTCALL_IF_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <hotcall_if.h>
#include <hotcall_config.h>


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

bool
hotcall_handle_if(struct hotcall_if *tif, struct hotcall_config *hotcall_config, uint8_t *exclude_list, int pos, int exclude_list_len, int offset);

#endif
