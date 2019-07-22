#ifndef _H_TRUSTED_HOTCALL_PARAMETER_
#define _H_TRUSTED_HOTCALL_PARAMETER_

#include "hotcall_function.h"
#include "hotcall_utils.h"
#include <string.h>

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
parse_vector_argument(const struct vector_parameter *vec_param, unsigned int offset, unsigned int n_iters, void *args[n_iters]) {

    unsigned int member_offset = vec_param->member_offset;
    //char *arg = (char Ävec_param->arg;
    // User has specified a custom offset.
    if(vec_param->vector_offset) {
        offset = *vec_param->vector_offset;
        if(vec_param->dereference) {
            args[0] = ((char *) *((void **) vec_param->arg + offset)) + member_offset;
            return;
        }
        switch(vec_param->fmt) {
            case 'p':
                args[0] = (((char *) vec_param->arg) + (offset) * sizeof(void *)) + member_offset;
                break;
            case 'd':
                args[0] = (((char *) vec_param->arg) + (offset) * sizeof(int)) + member_offset;
                break;
            case 'b':
                args[0] = (((char *) vec_param->arg) + (offset) * sizeof(bool)) + member_offset;
                break;
            case 'u':
                args[0] = (((char *) vec_param->arg) + (offset) * sizeof(unsigned int)) + member_offset;
                break;
            case ui8:
                args[0] = (((char *) vec_param->arg) + (offset) * sizeof(uint8_t)) + member_offset;
                break;
            case ui16:
                args[0] = (((char *) vec_param->arg) + (offset) * sizeof(uint16_t)) + member_offset;
                break;
            case ui32:
                args[0] = (((char *) vec_param->arg) + (offset) * sizeof(uint32_t)) + member_offset;
                break;
            case ui64:
                args[0] = (((char *) vec_param->arg) + (offset) * sizeof(uint64_t)) + member_offset;
                break;
            default: SWITCH_DEFAULT_REACHED
        }
    } else {
        if(vec_param->dereference) {
            for(int i = 0; i < n_iters; ++i) {
                args[i] = ((char *) *((void **) vec_param->arg + offset + i)) + member_offset;
            }
            return;
        }
        switch(vec_param->fmt) {
            case 'p':
                for(int i = 0; i < n_iters; ++i) {
                    args[i] = (((char *) vec_param->arg) + (offset + i) * sizeof(void *)) + member_offset;
                }
                break;
            case 'd':
                for(int i = 0; i < n_iters; ++i) {
                    args[i] = (((char *) vec_param->arg) + (offset + i) * sizeof(int)) + member_offset;
                }
                break;
            case 'b':
                for(int i = 0; i < n_iters; ++i) {
                    args[i] = (((char *) vec_param->arg) + (offset + i) * sizeof(bool)) + member_offset;
                }
                break;
            case 'u':
                for(int i = 0; i < n_iters; ++i) {
                    args[i] = (((char *) vec_param->arg) + (offset + i) * sizeof(unsigned int)) + member_offset;
                }
                break;
            case ui8:
                for(int i = 0; i < n_iters; ++i) {
                    args[i] = (((char *) vec_param->arg) + (offset + i) * sizeof(uint8_t)) + member_offset;
                }
                break;
            case ui16:
                for(int i = 0; i < n_iters; ++i) {
                    args[i] = (((char *) vec_param->arg) + (offset + i) * sizeof(uint16_t)) + member_offset;
                }
                break;
            case ui32:
                for(int i = 0; i < n_iters; ++i) {
                    args[i] = (((char *) vec_param->arg) + (offset + i) * sizeof(uint32_t)) + member_offset;
                }
                break;
            case ui64:
                for(int i = 0; i < n_iters; ++i) {
                    args[i] = (((char *) vec_param->arg) + (offset + i) * sizeof(uint64_t)) + member_offset;
                }
                break;
            default: SWITCH_DEFAULT_REACHED
        }
    }
}

static inline void
parse_function_arguments(const struct parameter *param, int n_params, int offset, void *args[n_params][1]) {
    for(int j = 0; j < n_params; ++j) {
        switch(param[j].type) {
            case VARIABLE_TYPE: args[j][0] = parse_variable_argument(&param[j].value.variable); break;
            case POINTER_TYPE: args[j][0] = parse_pointer_argument(&param[j].value.pointer); break;
            case VECTOR_TYPE: parse_vector_argument(&param[j].value.vector, offset, 1, args[j]); break;
        }
    }
}

static inline void
parse_arguments(const struct parameter *param, unsigned int n_iters, unsigned int n_params, void *args[n_params][n_iters], int offset) {
    for(int j = 0; j < n_params; ++j) {
        switch(param[j].type) {
            case VARIABLE_TYPE:
                args[j][0] = parse_variable_argument(&param[j].value.variable);
                for(int i = 1; i < n_iters; ++i) args[j][i] = args[j][i - 1];
                break;
            case POINTER_TYPE:
                args[j][0] = parse_pointer_argument(&param[j].value.pointer);
                for(int i = 1; i < n_iters; ++i) args[j][i] = args[j][i - 1];
                break;
            case VECTOR_TYPE:
                parse_vector_argument(&param[j].value.vector, offset, n_iters, args[j]);
                break;
        }
    }
}

#endif
