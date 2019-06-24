#ifndef _H_BOOLEAN_EXPRESSION_H
#define _H_BOOLEAN_EXPRESSION_H

#include "hotcall_config.h"
#include "hotcall_function.h"
#include "hotcall_utils.h"
#include <hotcall_if.h>
#include <stddef.h>

int
evaluate_postfix(struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config, int offset);


static inline void * apply_loop_offset(void *arg, char fmt, unsigned int offset) {
    switch(fmt) {
        case 'p': return (void **) arg + offset;
        case 'd': return (int *) arg + offset;
        case 'b': return (bool *) arg + offset;
        case 'u': case ui8: case ui16: case ui32:
            return (unsigned int *) arg + offset;
        default: SWITCH_DEFAULT_REACHED
    }
}

static inline void *
parse_argument_v2(const struct parameter *param, unsigned int i) {
    switch(param->type) {
        case VARIABLE_TYPE: return (char * ) param->value.variable.arg + param->value.variable.member_offset;
        case POINTER_TYPE: return (param->value.pointer.dereference)
            ? ((char *) *((void **) param->value.pointer.arg)) + param->value.pointer.member_offset
            : (char *) param->value.pointer.arg + param->value.pointer.member_offset;
        case VECTOR_TYPE:
            if(param->value.vector.dereference) {
                return ((char *) *((void **) param->value.vector.arg + i)) +  param->value.vector.member_offset;
            }
            switch(param->value.vector.fmt) {
                case 'p': return ((char *) ((void **) param->value.vector.arg + i)) +  param->value.vector.member_offset;
                case 'd': return ((char *) ((int *) param->value.vector.arg + i)) +  param->value.vector.member_offset;
                case 'b': return ((char *) ((bool *) param->value.vector.arg + i)) +  param->value.vector.member_offset;
                case 'u': case ui8: case ui16: case ui32: return ((char *) ((unsigned int *) param->value.vector.arg + i)) +  param->value.vector.member_offset;
                default: SWITCH_DEFAULT_REACHED
            }
            break;
        default: SWITCH_DEFAULT_REACHED
    }
}

static inline void
parse_function_arguments(const struct parameter *param, int n_params, int offset, void *args[n_params]) {
    for(int j = 0; j < n_params; ++j) {
        args[j] = parse_argument_v2(&param[j], offset);
    }
}

static inline void
parse_arguments(const struct parameter *param, int n_params, int n_iters, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        for(int j = 0; j < n_params; ++j) {
            if(i > 0 && param[j].type == VARIABLE_TYPE) args[i][j] = args[i - 1][j];
            else args[i][j] = parse_argument_v2(&param[j], i);
        }
    }
}

#endif
