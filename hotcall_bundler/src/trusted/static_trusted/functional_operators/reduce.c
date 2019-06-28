#include "reduce.h"
#include "predicate.h"
#include "parameter.h"

static inline void
combine_result(char op, struct parameter *accumulator, void *ret, int n) {

    if(n == 0) {
        switch(accumulator->value.variable.fmt) {
            case 'd': case 'b':
                *(int *) accumulator->value.variable.arg = *(int *) ret;
                break;
            case 'u':
                *(unsigned int *) accumulator->value.variable.arg = *(unsigned int *) ret;
                break;
            case ui8:
                *(uint8_t *) accumulator->value.variable.arg = *(uint8_t *) ret;
                break;
            case ui16:
                *(uint16_t *) accumulator->value.variable.arg = *(uint16_t *) ret;
                break;
            case ui32:
                *(uint32_t *) accumulator->value.variable.arg = *(uint32_t *) ret;
                break;
            default:
                SWITCH_DEFAULT_REACHED
        }
        return;
    }

    switch(op) {
        case '+':
            switch(accumulator->value.variable.fmt) {
                case 'd': case 'b':
                    *(int *) accumulator->value.variable.arg += *(int *) ret;
                    break;
                case 'u':
                    *(unsigned int *) accumulator->value.variable.arg += *(unsigned int *) ret;
                    break;
                case ui8:
                    *(uint8_t *) accumulator->value.variable.arg += *(uint8_t *) ret;
                    break;
                case ui16:
                    *(uint16_t *) accumulator->value.variable.arg += *(uint16_t *) ret;
                    break;
                case ui32:
                    *(uint32_t *) accumulator->value.variable.arg += *(uint32_t *) ret;
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
            }
            break;
        case '-':
            switch(accumulator->value.variable.fmt) {
                case 'd': case 'b':
                    *(int *) accumulator->value.variable.arg -= *(int *) ret;
                    break;
                case 'u':
                    *(unsigned int *) accumulator->value.variable.arg -= *(unsigned int *) ret;
                    break;
                case ui8:
                    *(uint8_t *) accumulator->value.variable.arg -= *(uint8_t *) ret;
                    break;
                case ui16:
                    *(uint16_t *) accumulator->value.variable.arg -= *(uint16_t *) ret;
                    break;
                case ui32:
                    *(uint32_t *) accumulator->value.variable.arg -= *(uint32_t *) ret;
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
            }
            break;
        case '&':
            switch(accumulator->value.variable.fmt) {
                case 'd': case 'b':
                    *(int *) accumulator->value.variable.arg = *(int *) accumulator->value.variable.arg && *(int *) ret;
                    break;
                case 'u':
                    *(unsigned int *) accumulator->value.variable.arg = *(unsigned int *) accumulator->value.variable.arg  && *(unsigned int *) ret;
                    break;
                case ui8:
                    *(uint8_t *) accumulator->value.variable.arg && *(uint8_t *) ret;
                    break;
                case ui16:
                    *(uint16_t *) accumulator->value.variable.arg && *(uint16_t *) ret;
                    break;
                case ui32:
                    *(uint32_t *) accumulator->value.variable.arg && *(uint32_t *) ret;
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
            }
            break;
        case '|':
            switch(accumulator->value.variable.fmt) {
                case 'd': case 'b':
                    *(int *) accumulator->value.variable.arg = *(int *) accumulator->value.variable.arg || *(int *) ret;
                    break;
                case 'u':
                    *(unsigned int *) accumulator->value.variable.arg =  *(unsigned int *) accumulator->value.variable.arg || *(unsigned int *) ret;
                    break;
                case ui8:
                    *(uint8_t *) accumulator->value.variable.arg || *(uint8_t *) ret;
                    break;
                case ui16:
                    *(uint16_t *) accumulator->value.variable.arg || *(uint16_t *) ret;
                    break;
                case ui32:
                    *(uint32_t *) accumulator->value.variable.arg || *(uint32_t *) ret;
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

    unsigned int n_params = re->config->n_params - 1;
    struct parameter *accumulator = &re->params[n_params];
    unsigned int in_len = *re->params[0].value.vector.len;
    // No function is used in reduce, we shall only combine the input elements.
    if(re->config->function_id == 255) {
        for(int i = 0; i < in_len; ++i) {
            switch(re->params[0].value.vector.fmt) {
                case 'd':
                    combine_result(re->config->op, accumulator, ((int *) re->params[0].value.vector.arg) + i, i);
                    break;
                case 'u':
                    combine_result(re->config->op, accumulator, ((unsigned int *) re->params[0].value.vector.arg) + i, i);
                    break;
                case 'b':
                    combine_result(re->config->op, accumulator, ((bool *) re->params[0].value.vector.arg) + i, i);
                    break;
                case ui8:
                    combine_result(re->config->op, accumulator, ((uint8_t *) re->params[0].value.vector.arg) + i, i);
                    break;
                case ui16:
                    combine_result(re->config->op, accumulator, ((uint16_t *) re->params[0].value.vector.arg) + i, i);
                    break;
                case ui32:
                    combine_result(re->config->op, accumulator, ((uint32_t *) re->params[0].value.vector.arg) + i, i);
                    break;
                default: SWITCH_DEFAULT_REACHED
            }
        }
        return;
    }

    int ret = 0;
    void *args[in_len][n_params];
    parse_arguments(re->params, in_len, n_params, args, 0);
    for(int i = 0; i < in_len; ++i) {
        hotcall_config->execute_function(re->config->function_id, args[i], &ret);
        combine_result(re->config->op, accumulator, &ret, i);
    }
}
