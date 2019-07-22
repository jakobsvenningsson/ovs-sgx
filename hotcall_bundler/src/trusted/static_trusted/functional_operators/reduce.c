#include "reduce.h"
#include "predicate.h"
#include "parameter.h"

static inline void
combine_result(char op, char fmt, void *accumulator, void *ret, int n) {

    if(n == 0) {
        switch(fmt) {
            case 'd': case 'b':
                *(int *) accumulator = *(int *) ret;
                break;
            case 'u':
                *(unsigned int *) accumulator= *(unsigned int *) ret;
                break;
            case ui8:
                *(uint8_t *) accumulator = *(uint8_t *) ret;
                break;
            case ui16:
                *(uint16_t *) accumulator = *(uint16_t *) ret;
                break;
            case ui32:
                *(uint32_t *) accumulator = *(uint32_t *) ret;
                break;
            default:
                SWITCH_DEFAULT_REACHED
        }
        return;
    }

    switch(op) {
        case '+':
            switch(fmt) {
                case 'd': case 'b':
                    *(int *) accumulator += *(int *) ret;
                    break;
                case 'u':
                    *(unsigned int *) accumulator += *(unsigned int *) ret;
                    break;
                case ui8:
                    *(uint8_t *) accumulator += *(uint8_t *) ret;
                    break;
                case ui16:
                    *(uint16_t *) accumulator += *(uint16_t *) ret;
                    break;
                case ui32:
                    *(uint32_t *) accumulator += *(uint32_t *) ret;
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
            }
            break;
        case '-':
            switch(fmt) {
                case 'd': case 'b':
                    *(int *) accumulator -= *(int *) ret;
                    break;
                case 'u':
                    *(unsigned int *) accumulator -= *(unsigned int *) ret;
                    break;
                case ui8:
                    *(uint8_t *) accumulator -= *(uint8_t *) ret;
                    break;
                case ui16:
                    *(uint16_t *) accumulator -= *(uint16_t *) ret;
                    break;
                case ui32:
                    *(uint32_t *) accumulator -= *(uint32_t *) ret;
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
            }
            break;
        case '&':
            switch(fmt) {
                case 'd': case 'b':
                    *(int *) accumulator = *(int *) accumulator && *(int *) ret;
                    break;
                case 'u':
                    *(unsigned int *) accumulator = *(unsigned int *) accumulator  && *(unsigned int *) ret;
                    break;
                case ui8:
                    *(uint8_t *) accumulator && *(uint8_t *) ret;
                    break;
                case ui16:
                    *(uint16_t *) accumulator && *(uint16_t *) ret;
                    break;
                case ui32:
                    *(uint32_t *) accumulator && *(uint32_t *) ret;
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
            }
            break;
        case '|':
            switch(fmt) {
                case 'd': case 'b':
                    *(int *) accumulator = *(int *) accumulator || *(int *) ret;
                    break;
                case 'u':
                    *(unsigned int *) accumulator =  *(unsigned int *) accumulator || *(unsigned int *) ret;
                    break;
                case ui8:
                    *(uint8_t *) accumulator || *(uint8_t *) ret;
                    break;
                case ui16:
                    *(uint16_t *) accumulator || *(uint16_t *) ret;
                    break;
                case ui32:
                    *(uint32_t *) accumulator|| *(uint32_t *) ret;
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
            }
            break;
        default:
            SWITCH_DEFAULT_REACHED
    }
}

void
hotcall_handle_reduce(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {

    struct hotcall_reduce *re = &qi->call.re;

    unsigned int n_params = re->config->n_params;
    //struct parameter *accumulator = &re->params[n_params - 1];
    int accumulator = 0;

    unsigned int in_len = *re->params[0].value.vector.len;
    // No function is used in reduce, we shall only combine the input elements.
    if(re->config->function_id == 255) {
        for(int i = 0; i < in_len; ++i) {
            switch(re->params[0].value.vector.fmt) {
                case 'd':
                    combine_result(re->config->op, 'd', &accumulator, ((int *) re->params[0].value.vector.arg) + i, i);
                    break;
                case 'u':
                    combine_result(re->config->op, 'u', &accumulator, ((unsigned int *) re->params[0].value.vector.arg) + i, i);
                    break;
                case 'b':
                    combine_result(re->config->op, 'b', &accumulator, ((bool *) re->params[0].value.vector.arg) + i, i);
                    break;
                case ui8:
                    combine_result(re->config->op, ui8, &accumulator, ((uint8_t *) re->params[0].value.vector.arg) + i, i);
                    break;
                case ui16:
                    combine_result(re->config->op, ui16, &accumulator, ((uint16_t *) re->params[0].value.vector.arg) + i, i);
                    break;
                case ui32:
                    combine_result(re->config->op, ui32, &accumulator, ((uint32_t *) re->params[0].value.vector.arg) + i, i);
                    break;
                default: SWITCH_DEFAULT_REACHED
            }
        }
        *(int *) re->params[n_params - 1].value.variable.arg = accumulator;
        return;
    }

    void *args[n_params][1];
    for(int i = 0; i < in_len; ++i) {
        parse_function_arguments(re->params, n_params, i, args);
        hotcall_config->execute_function(re->config->function_id, 1, n_params, args);
        combine_result(re->config->op, re->params[0].value.vector.fmt, &accumulator, args[n_params - 1][0], i);
    }
    *(int *) re->params[n_params - 1].value.variable.arg = accumulator;
}
