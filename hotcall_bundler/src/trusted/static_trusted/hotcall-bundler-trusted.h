#ifndef _H_LIB_HOTCALL_TRUSTED_
#define _H_LIB_HOTCALL_TRUSTED_

#ifdef __cplusplus
extern "C" {
#endif

#include "hotcall_config.h"
#include "hotcall.h"


#define SWITCH_DEFAULT_REACHED printf("Default reached at %s %d\n", __FILE__, __LINE__);
#define MAX_LOOP_RECURSION 3

static struct hotcall_config *hotcall_config;

struct loop_stack_item {
    unsigned int body_len;
    unsigned int index;
    unsigned int n_iters;
    unsigned int offset;
};

struct batch_status {
    int error;
    bool exit_batch;
    bool terminate;
};

struct queue_context {
    unsigned int len;
    uint8_t *exclude_list;
    unsigned int loop_stack_pos;
    struct loop_stack_item loop_stack[MAX_LOOP_RECURSION];
    unsigned int *queue_pos;
    unsigned int else_len[MAX_LOOP_RECURSION];
    unsigned int if_nesting;
};

void
hotcall_register_config(struct hotcall_config *config);

#ifdef __cplusplus
}
#endif

#endif
