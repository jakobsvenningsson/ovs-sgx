#ifndef _H_TRUSTED_HOTCALL_PARAMETER_
#define _H_TRUSTED_HOTCALL_PARAMETER_

#include "hotcall_function.h"
#include "hotcall_utils.h"

static inline void *
parse_variable_argument(const struct variable_parameter *var_param) {
    return (char *) var_param->arg + var_param->member_offset;
}

static inline void *
parse_pointer_argument(const struct pointer_parameter *ptr_param) {
    return (ptr_param->dereference)
        ? ((char *) *((void **) ptr_param->arg)) + ptr_param->member_offset
        : (char *) ptr_param->arg + ptr_param->member_offset;
}

static inline void
parse_vector_argument(const struct vector_parameter *vec_param, unsigned int offset, unsigned int param_index, unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {

    // User has specified a custom offset.
    /*if(vec_param->vector_offset) {
        offset = *vec_param->vector_offset;
    }*/

    if(vec_param->dereference) {
        for(int i = 0; i < n_iters; ++i) {
            args[i][param_index] = ((char *) *((void **) vec_param->arg + offset + i)) + vec_param->member_offset;
        }
        return;
    }

    /*unsigned int elem_size = 4;
    switch(vec_param->fmt) {
        case 'p': elem_size = sizeof(void *); break;
        case 'd': elem_size = sizeof(int); break;
        case 'b': elem_size = sizeof(bool); break;
        case 'u': elem_size = sizeof(unsigned int); break;
        case ui8: elem_size = sizeof(uint8_t); break;
        case ui16: elem_size = sizeof(uint16_t); break;
        case ui32: elem_size = sizeof(uint32_t); break;
        default: SWITCH_DEFAULT_REACHED
    }
*/
    //unsigned int elem_size = 4;
    switch(vec_param->fmt) {
        case 'p':
            for(int i = 0; i < n_iters; ++i) {
                args[i][param_index] = (((char *) vec_param->arg) + (offset + i) * sizeof(void *)) + vec_param->member_offset;
            }
            break;
        case 'd':
            for(int i = 0; i < n_iters; ++i) {
                args[i][param_index] = (((char *) vec_param->arg) + (offset + i) * sizeof(int)) + vec_param->member_offset;
            }
            break;
        case 'b':
            for(int i = 0; i < n_iters; ++i) {
                args[i][param_index] = (((char *) vec_param->arg) + (offset + i) * sizeof(bool)) + vec_param->member_offset;
            }
            break;
        case 'u':
            for(int i = 0; i < n_iters; ++i) {
                args[i][param_index] = (((char *) vec_param->arg) + (offset + i) * sizeof(unsigned int)) + vec_param->member_offset;
            }
            break;
        case ui8:
            for(int i = 0; i < n_iters; ++i) {
                args[i][param_index] = (((char *) vec_param->arg) + (offset + i) * sizeof(uint8_t)) + vec_param->member_offset;
            }
            break;
        case ui16:
            for(int i = 0; i < n_iters; ++i) {
                args[i][param_index] = (((char *) vec_param->arg) + (offset + i) * sizeof(uint16_t)) + vec_param->member_offset;
            }
            break;
        case ui32:
            for(int i = 0; i < n_iters; ++i) {
                args[i][param_index] = (((char *) vec_param->arg) + (offset + i) * sizeof(uint32_t)) + vec_param->member_offset;
            }
            break;
        default: SWITCH_DEFAULT_REACHED
    }

/*
    for(int i = 0; i < n_iters; ++i) {
        args[i][param_index] = (((char *) vec_param->arg) + (offset + i) * elem_size) + vec_param->member_offset;
    }*/

}

static inline void
parse_function_arguments(const struct parameter *param, int n_params, int offset, void *args[1][n_params]) {
    for(int j = 0; j < n_params; ++j) {
        switch(param[j].type) {
            case VARIABLE_TYPE: args[0][j] = parse_variable_argument(&param[j].value.variable); break;
            case POINTER_TYPE: args[0][j] = parse_pointer_argument(&param[j].value.pointer); break;
            case VECTOR_TYPE: parse_vector_argument(&param[j].value.vector, offset, j, 1, n_params, args); break;
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
                //param[j].value.vector.vector_offset ? *param[j].value.vector.vector_offset : offset
                parse_vector_argument(&param[j].value.vector, offset, j, n_iters, n_params, args);
                break;
        }
    }
}

#endif
