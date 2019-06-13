#ifndef _H_LIB_HOTCALL_TRUSTED_
#define _H_LIB_HOTCALL_TRUSTED_

#ifdef __cplusplus
extern "C" {
#endif

#include "hotcall_config.h"
#include "hotcall.h"

#define SWITCH_DEFAULT_REACHED printf("Default reached at %s %d\n", __FILE__, __LINE__);
#define MAX_LOOP_RECURSION 3

struct loop_stack_item {
    unsigned int body_len;
    unsigned int index;
    bool first_iteration;
    bool has_calculated_length;
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

extern inline void *
parse_argument(const struct parameter *param, unsigned int offset) {
    void *arg;
    char fmt;
    unsigned int _offset = 0;
    switch(param->type) {
        case VARIABLE_TYPE: arg = param->value.variable.arg; fmt = param->value.variable.fmt; break;
        case VECTOR_TYPE:   arg = param->value.vector.arg; fmt = param->value.vector.fmt; _offset = offset; break;
        default: SWITCH_DEFAULT_REACHED
    }
    switch(fmt) {
        case 'd': return ((int *) arg) + _offset;
        case 'u': return ((unsigned int *) arg) + _offset;
        case 'b': return ((bool *) arg) + _offset;
        default: SWITCH_DEFAULT_REACHED
    }
}


extern inline void *
load_variable_argument(const struct variable_parameter *var) {
    switch(var->fmt) {
        case 'd': return ((int *) var->arg);
        case 'u': return ((unsigned int *) var->arg);
        case 'b': return ((bool *) var->arg);
        default: SWITCH_DEFAULT_REACHED
    }
}


extern inline void *
load_vector_argument(const struct vector_parameter *vec, unsigned int offset) {
    switch(vec->fmt) {
        case 'd': return ((int *) vec->arg) + offset;
        case 'u': return ((unsigned int *) vec->arg) + offset;
        case 'b': return ((bool *) vec->arg) + offset;
        default: SWITCH_DEFAULT_REACHED
    }
}


#ifdef __cplusplus
}
#endif

#endif
