#ifndef _H_BOOLEAN_EXPRESSION_H
#define _H_BOOLEAN_EXPRESSION_H

#include "hotcall_config.h"
#include "hotcall_function.h"
#include "hotcall_utils.h"

enum postfix_item_type { OPERAND, OPERATOR };
struct postfix_item {
        char ch;
        enum postfix_item_type type;
        struct parameter *elem;
};

void
to_postfix(const char *condition_fmt, struct parameter *predicate_args, struct postfix_item *output, unsigned int *output_length);

int
evaluate_variable(struct postfix_item *operand_item, struct hotcall_config *hotcall_config, int offset);

int
evaluate_postfix(struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config, int offset);


static inline void * apply_loop_offset(void *arg, char fmt, unsigned int offset) {
    printf("apply_loop_offset %c %u %p %p\n", fmt, offset, arg, (void **) arg + offset);
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
parse_argument(const struct parameter *param, unsigned int offset) {
    //void *arg;
    char fmt;
    //unsigned int _offset = 0, _member_offset = 0;
    switch(param->type) {
        case VARIABLE_TYPE:
            printf("VARIABLE_TYPE 1\n");
            return apply_loop_offset(param->value.variable.arg, param->value.variable.fmt, 0);
        //arg = param->value.variable.arg; fmt = param->value.variable.fmt; break;
        case VECTOR_TYPE: return apply_loop_offset(param->value.vector.arg, param->value.vector.fmt, offset);
            //arg = (void **) param->value.vector.arg; //param->value.vector.dereference ? *(void **) param->value.vector.arg : param->value.vector.arg;
            //fmt = param->value.vector.fmt;
            //_offset = offset;
            //break;
        case POINTER_TYPE: return apply_loop_offset(param->value.pointer.arg, param->value.pointer.fmt, offset);
            //arg = (void **) param->value.pointer.arg;// param->value.pointer.dereference ? *(void **) param->value.pointer.arg : param->value.pointer.arg;
            //fmt = param->value.pointer.fmt;
            //break;
        case STRUCT_TYPE:
            /*printf("STRUCT_TYPE %d\n", offset);
            switch(param->value.struct_.member_fmt) {
                case 'p': return ((char *) apply_loop_offset(param->value.struct_.arg, 'p', offset)) + param->value.struct_.member_offset;
                default:
                    return ((char *) param->value.struct_.arg) + offset * param->value.struct_.struct_size + param->value.struct_.member_offset;

            }*/
            //return ((char *) param->value.struct_.arg offset)) + param->value.struct_.member_offset;
            /*printf("structt ype\n");
            arg = ((char *) *((void **) param->value.struct_.arg + offset)) + param->value.struct_.member_offset;
            fmt = param->value.struct_.member_fmt;
            break;*/
        case POINTER_TYPE_v2:
        {
            printf("POINTER_TYPE_v2\n");
            bool dereference = param->value.pointer_v2.dereference;
            switch(param->value.pointer_v2.arg->type) {
                case VARIABLE_TYPE:
                    fmt = param->value.pointer_v2.arg->value.variable.fmt;
                    if(dereference) {
                        return *(void **) apply_loop_offset(param->value.pointer_v2.arg->value.variable.arg, fmt, offset);
                    } else {
                        void *p = apply_loop_offset(param->value.pointer_v2.arg->value.variable.arg, fmt, offset);
                        return p;
                    }
                    /*return dereference
                        ? *(void **) apply_loop_offset(param->value.pointer_v2.arg->value.variable.arg, fmt, offset)
                        : apply_loop_offset(param->value.pointer_v2.arg->value.variable.arg, fmt, offset);*/
                /*case STRUCT_TYPE:
                    fmt = param->value.pointer_v2.arg->value.struct_.member_fmt;
                    return dereference
                        ? (char *) *((void **) param->value.pointer_v2.arg->value.struct_.arg + offset) + param->value.pointer_v2.arg->value.struct_.member_offset
                        : ((char *) apply_loop_offset(param->value.pointer_v2.arg->value.struct_.arg, 'p', offset))  + param->value.pointer_v2.arg->value.struct_.member_offset;*/
                default: SWITCH_DEFAULT_REACHED
            }
            //arg = parse_argument(param->value.pointer_v2.arg, offset)
        }
        case VECTOR_TYPE_v2:
            return parse_argument(param->value.vector_v2.arg, offset);
            //fmt = 'd';
            //switch(param->value.vector_v2.arg->type) {

                /*)
                /*case STRUCT_TYPE:
                    printf("STRUCT_TYPE\n");
                    fmt = param->value.vector_v2.arg->value.struct_.member_fmt;
                    arg = ((char *) ((void **) param->value.vector_v2.arg->value.struct_.arg + offset)) + param->value.vector_v2.arg->value.struct_.member_offset;
                    break;
                default: SWITCH_DEFAULT_REACHED*/
            //}
            //_offset = offset;
            break;
        default: SWITCH_DEFAULT_REACHED
    }
    /*printf("after\n");
    switch(fmt) {
        case 'p': return ((char *) (((void **) arg) + _offset) + _member_offset);
        case 'd': return ((char *) (((int *) arg) + _offset) + _member_offset);
        case 'b': return ((char *) (((bool *) arg) + _offset) + _member_offset);
        case 'u': case ui8: case ui16: case ui32:
            return ((char *) (((unsigned int *) arg) + _offset) + _member_offset);
        default: SWITCH_DEFAULT_REACHED
    }*/
}

static inline void *
parse_argument_1(const struct parameter *param, int addr_modifications[], int n_modifications, int loop_offset) {
    switch(param->type) {
        case VARIABLE_TYPE:
            if(!n_modifications) return param->value.variable.arg;
            void *addr = param->value.variable.arg;
            for(int i = 0; i < n_modifications; ++i) {
                if(addr_modifications[i] == -1) addr = *(void **) addr;
                else addr += addr_modifications[i];
            }
            return addr;
        case POINTER_TYPE: return param->value.pointer.arg;
        case STRUCT_TYPE:
            addr_modifications[n_modifications] = param->value.struct_.member_offset;
            return parse_argument_1(param->value.struct_.arg, addr_modifications, n_modifications + 1, loop_offset);
        case POINTER_TYPE_v2:
        {
            bool dereference = param->value.pointer_v2.dereference;
            if(dereference) {
                addr_modifications[n_modifications] = -1;
                return parse_argument_1(param->value.pointer_v2.arg, addr_modifications, n_modifications + 1, loop_offset);
            } else {
                return parse_argument_1(param->value.pointer_v2.arg, addr_modifications, n_modifications, loop_offset);
            }
        }
        case VECTOR_TYPE_v2:
            switch(param->value.vector_v2.arg->type) {
                case POINTER_TYPE_v2:
                    addr_modifications[n_modifications] = loop_offset * sizeof(void *);
                    return parse_argument_1(param->value.vector_v2.arg, addr_modifications, n_modifications + 1, 0);
                case STRUCT_TYPE:
                    addr_modifications[n_modifications] = loop_offset * param->value.vector_v2.arg->value.struct_.struct_size;
                    return parse_argument_1(param->value.vector_v2.arg, addr_modifications, n_modifications + 1, 0);
                case VARIABLE_TYPE:
                    switch(param->value.vector_v2.arg->value.variable.fmt) {
                        case 'p':
                            addr_modifications[n_modifications] = loop_offset * sizeof(void *);
                            parse_argument_1(param->value.vector_v2.arg, addr_modifications, n_modifications + 1, 0);
                            break;
                        case 'd':
                            addr_modifications[n_modifications] = loop_offset * sizeof(int);
                            parse_argument_1(param->value.vector_v2.arg, addr_modifications, n_modifications + 1, 0);
                            break;
                        case 'u':
                            addr_modifications[n_modifications] = loop_offset * sizeof(unsigned int);
                            parse_argument_1(param->value.vector_v2.arg, addr_modifications, n_modifications + 1, 0);
                            break;
                        default: SWITCH_DEFAULT_REACHED
                    }
                    break;
                default: SWITCH_DEFAULT_REACHED
            }
            break;
        default: SWITCH_DEFAULT_REACHED
    }
}

/*static inline void *
parse_argument_1(const struct parameter *param, void *arg) {
    switch(param->type) {
        case VARIABLE_TYPE: return apply_loop_offset(param->value.variable.arg, param->value.variable.fmt, apply_offset ? offset : 0);
        case VECTOR_TYPE: return apply_loop_offset(param->value.vector.arg, param->value.vector.fmt, offset);
        case POINTER_TYPE: return apply_loop_offset(param->value.pointer.arg, param->value.pointer.fmt,  apply_offset ? offset : 0);
        case STRUCT_TYPE:
            switch(param->value.struct_.member_fmt) {
                case 'p': return ((char *) apply_loop_offset(param->value.struct_.arg, 'p',  apply_offset ? offset : 0)) + param->value.struct_.member_offset;
                default:
                    return ((char *) param->value.struct_.arg) + offset * param->value.struct_.struct_size + param->value.struct_.member_offset;

            }
        case POINTER_TYPE_v2:
        {
            bool dereference = param->value.pointer_v2.dereference;
            char fmt;
            switch(param->value.pointer_v2.arg->type) {
                case VARIABLE_TYPE:
                    fmt = param->value.pointer_v2.arg->value.variable.fmt;
                    if(dereference) {
                        return *(void **) apply_loop_offset(param->value.pointer_v2.arg->value.variable.arg, fmt, apply_offset ? offset : 0);
                    } else {
                        return apply_loop_offset(param->value.pointer_v2.arg->value.variable.arg, fmt,  apply_offset ? offset : 0);
                    }
                case STRUCT_TYPE:
                    fmt = param->value.pointer_v2.arg->value.struct_.member_fmt;
                    return dereference
                        ? (char *) *((void **) parse_argument_1(param->value.pointer_v2.arg, offset, apply_offset))  + param->value.pointer_v2.arg->value.struct_.member_offset
                        : ((char *) apply_loop_offset(param->value.pointer_v2.arg->value.struct_.arg, 'p',  (apply_offset ? offset : 0)))  + param->value.pointer_v2.arg->value.struct_.member_offset;
                default: SWITCH_DEFAULT_REACHED
            }
        }
        case VECTOR_TYPE_v2:
            return parse_argument_1(param->value.vector_v2.arg, offset, true);
            break;
        default: SWITCH_DEFAULT_REACHED
    }
}*/

/*
static inline void *
parse_argument_1(const struct parameter *param, unsigned int offset, int member_offset) {
    void *arg;
    char fmt;
    unsigned int _offset = 0;
    switch(param->type) {
        case VARIABLE_TYPE: arg = param->value.variable.arg; fmt = param->value.variable.fmt; break;
        case VECTOR_TYPE:
            arg = param->value.vector.dereference ? *((void **) param->value.vector.arg + offset) : param->value.vector.arg;
            fmt = param->value.vector.fmt;
            _offset = offset;
            break;
        case POINTER_TYPE:
            arg = param->value.pointer.dereference ? *(void **) param->value.pointer.arg : param->value.pointer.arg;
            fmt = param->value.pointer.fmt;
            break;
        default: SWITCH_DEFAULT_REACHED
    }
    printf("offset %d %c\n", _offset, fmt);
    switch(fmt) {
        case 'p': return ((char *) arg);
        case 'd': return ((int *) arg) + _offset;
        case 'b': return ((bool *) arg) + _offset;
        case 'u': case ui8: case ui16: case ui32:
            return ((unsigned int *) arg) + _offset;
        default: SWITCH_DEFAULT_REACHED
    }
}*/


static inline void *
load_variable_argument(const struct variable_parameter *var) {
    switch(var->fmt) {
        case 'd': return ((int *) var->arg);
        case 'b': return ((bool *) var->arg);
        case 'u': case ui8: case ui16: case ui32:
            return ((unsigned int *) var->arg);
        default: SWITCH_DEFAULT_REACHED
    }
}


static inline void *
load_vector_argument(const struct vector_parameter *vec, unsigned int offset) {
    switch(vec->fmt) {
        case 'd': return ((int *) vec->arg) + offset;
        case 'b': return ((bool *) vec->arg) + offset;
        case 'u': case ui8: case ui16: case ui32:
            return ((unsigned int *) vec->arg) + offset;
        default: SWITCH_DEFAULT_REACHED
    }
}

#endif
