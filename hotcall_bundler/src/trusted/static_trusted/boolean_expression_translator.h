#ifndef _H_BOOLEAN_EXPRESSION_H
#define _H_BOOLEAN_EXPRESSION_H

#include "hotcall_config.h"
#include "hotcall_function.h"
#include "hotcall_utils.h"
#include <hotcall_if.h>
#include <stddef.h>

int
evaluate_postfix(struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config, int offset);


/*static inline void *
parse_argument_v3(const struct parameter *param, void *addr, int member_offset, unsigned int i) {
    if(param->type == VARIABLE_TYPE) { return (char * ) addr + member_offset; }
    else if(param->type == POINTER_TYPE) {

    } else if(param->type == VECTOR_TYPE) {

    } else {
        SWITCH_DEFAULT_REACHED
    }
        case POINTER_TYPE: return (param->value.pointer.dereference)
            ? ((char *) *((void **) addr)) + member_offset
            : (char *) addr + member_offset;
        case VECTOR_TYPE:

            break;
        default:
    }
}*/

static inline void *
parse_variable_argument(const struct variable_parameter *var_param) {
    return (char * ) var_param->arg + var_param->member_offset;
}

static inline void *
parse_pointer_argument(const struct pointer_parameter *ptr_param) {
    return (ptr_param->dereference)
        ? ((char *) *((void **) ptr_param->arg)) + ptr_param->member_offset
        : (char *) ptr_param->arg + ptr_param->member_offset;
}

static inline void *
parse_vector_argument(const struct vector_parameter *vec_param, unsigned int i) {
    if(vec_param->vector_offset) {
        i = *vec_param->vector_offset;
    }
    if(vec_param->dereference) {
        return ((char *) *((void **) vec_param->arg + i)) +  vec_param->member_offset;
    } else {
        switch(vec_param->fmt) {
            case 'p': return ((char *) ((void **) vec_param->arg + i)) + vec_param->member_offset;
            case 'd': return ((char *) ((int *) vec_param->arg + i)) + vec_param->member_offset;
            case 'b': return ((char *) ((bool *) vec_param->arg + i)) + vec_param->member_offset;
            case 'u': case ui8: case ui16: case ui32:
                return  ((char *) ((unsigned int *) vec_param->arg + i)) + + vec_param->member_offset;
            default: SWITCH_DEFAULT_REACHED
        }
    }
}

static inline void *
parse_argument_v2(const struct parameter *param, unsigned int i) {
    switch(param->type) {
        case VARIABLE_TYPE: return parse_variable_argument(&param->value.variable);
        case POINTER_TYPE: return parse_pointer_argument(&param->value.pointer);
        case VECTOR_TYPE: return parse_vector_argument(&param->value.vector, i);
    }
}

static inline void
parse_argument_v3(const struct parameter *param, unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params], int offset) {
    for(int j = 0; j < n_params; ++j) {
        switch(param[j].type) {
            case VARIABLE_TYPE:
                args[0][j] = parse_variable_argument(&param[j].value.variable);
                for(int i = 1; i < n_iters; ++i) args[i][j] = args[i - 1][j];
                break;
            case POINTER_TYPE:
                args[0][j] = parse_pointer_argument(&param[j].value.pointer);
                for(int i = 1; i < n_iters; ++i) args[i][j] = args[i - 1][j];
                break;
            case VECTOR_TYPE:
                for(int i = 0; i < n_iters; ++i) {
                    args[i][j] = parse_vector_argument(&param[j].value.vector, offset + i);
                }
                break;
        }
    }
}

static inline void
parse_function_arguments(const struct parameter *param, int n_params, int offset, void *args[n_params]) {
    for(int j = 0; j < n_params; ++j) {
        args[j] = parse_argument_v2(&param[j], offset);
    }
}

static inline void
parse_arguments(const struct parameter *param, int n_params, int n_iters, void *args[n_iters][n_params], int offset) {
    parse_argument_v3(param, n_iters, n_params, args, offset);
}

#endif
