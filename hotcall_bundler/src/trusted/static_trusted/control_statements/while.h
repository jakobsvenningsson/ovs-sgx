#ifndef _H_TRUSTED_HOTCALL_WHILE_
#define _H_TRUSTED_HOTCALL_WHILE_

#include "for.h"
#include <string.h>
#include "hotcall_while.h"
#include "hotcall_config.h"

static inline unsigned int
hotcall_handle_while_begin(struct hotcall_while_start *while_s, struct hotcall_config *hotcall_config, struct loop_stack_item *loop_stack, unsigned int loop_stack_pos, int n, uint8_t *exclude_list) {
    if(!evaluate_predicate(while_s->config->postfix, while_s->config->postfix_length, hotcall_config, (loop_stack_pos > 0 ? loop_stack[loop_stack_pos - 1].offset : 0 ))) {
        memset(exclude_list + n, 1, while_s->config->body_len + 2);
        loop_stack_pos--;
        while_s->config->loop_in_process = false;
        return loop_stack_pos;
    }

    if(!while_s->config->loop_in_process) {
        while_s->config->loop_in_process = true;
        loop_stack[loop_stack_pos].body_len =  while_s->config->body_len;
        loop_stack[loop_stack_pos].offset = (loop_stack_pos > 0) ? loop_stack[loop_stack_pos - 1].offset : 0;
        loop_stack_pos++;
    } else {
        if(while_s->config->iter_vectors) {
            loop_stack[loop_stack_pos - 1].offset++;
        }
        memset(exclude_list + n, 0, loop_stack[loop_stack_pos - 1].body_len + 2);
    }

    return loop_stack_pos;
}

#endif
