#include "if.h"
#include "predicate.h"


static inline
int presedence(char c) {
    switch(c) {
        case '!':
            return 4;
        case '>': case '<': case '%':
            return 3;
        case '&': case '|':
            return 2;
        default:
            return 0;
    }
}

unsigned int
to_postfix(const char *condition_fmt, struct parameter *predicate_args, struct postfix_item *output) {
    char chs[128];
    unsigned int pos = 0, output_index = 0, variable_index = 0;
    char tmp;
    const char *input = condition_fmt;
    for(int i = 0; i < strlen(input); ++i) {

        if(input[i] >= 'a' && input[i] <= 'z') {
            output[output_index++] = (struct postfix_item) { input[i], &predicate_args[variable_index++] };
            continue;
        }

        switch(input[i]) {
            case '(': chs[pos++] = input[i]; break;
            case ')':
                while(pos > 0 && (tmp = chs[pos - 1]) != '(') {
                    output[output_index++] = (struct postfix_item) { tmp };
                    pos--;
                }
                if(pos > 0 && tmp == '(') {
                    pos--;
                }
                break;
            default:
                while(pos > 0 && chs[pos - 1] != '(' && presedence(input[i]) <= presedence(chs[pos - 1])) {
                    tmp = chs[--pos];
                    output[output_index++] = (struct postfix_item) { tmp };
                }
                chs[pos++] = input[i];
        }
    }

    while(pos > 0) {
        tmp = chs[--pos];
        output[output_index++].ch = tmp;
    }
    return output_index;
}


void
hotcall_handle_if(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    struct if_config *config = qi->call.tif.config;
    unsigned int loop_stack_pos = queue_ctx->loop_stack_pos, queue_position = *queue_ctx->queue_pos;
    unsigned int offset = loop_stack_pos > 0 ? queue_ctx->loop_stack[loop_stack_pos - 1].offset : 0;

    struct postfix_item ps[32];
    unsigned int postfix_len = to_postfix(config->predicate_fmt, qi->call.tif.params, ps);
    
    int res = evaluate_predicate(ps, postfix_len, hotcall_config, offset);
    if(res && config->else_branch_len > 0) {
        queue_ctx->else_len[queue_ctx->if_nesting] = config->else_branch_len;

    } else if(!res) {
        if(config->then_branch_len > 0) {
            *queue_ctx->queue_pos += config->then_branch_len;
        }
        if(config->return_if_false) {
            batch_status->exit_batch = true;
        }
    }
    queue_ctx->if_nesting++;
}
