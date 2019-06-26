#ifndef _H_TRUSTED_HOTCALL_PARAMETER_
#define _H_TRUSTED_HOTCALL_PARAMETER_

#include "hotcall_function.h"
#include "hotcall_utils.h"
/*
void
parse_arguments(const struct parameter *param, unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params], int offset);*/

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

static inline void
parse_function_arguments(const struct parameter *param, int n_params, int offset, void *args[n_params]) {
    for(int j = 0; j < n_params; ++j) {
        switch(param[j].type) {
            case VARIABLE_TYPE: args[j] = parse_variable_argument(&param[j].value.variable); break;
            case POINTER_TYPE: args[j] = parse_pointer_argument(&param[j].value.pointer); break;
            case VECTOR_TYPE: args[j] = parse_vector_argument(&param[j].value.vector, offset); break;
        }
    }
}

static inline void
parse_arguments(const struct parameter *param, unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params], int offset) {
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



#endif
