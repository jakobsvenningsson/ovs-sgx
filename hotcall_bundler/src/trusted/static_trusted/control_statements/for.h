#ifndef _H_TRUSTED_HOTCALL_FOR_
#define _H_TRUSTED_HOTCALL_FOR_

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <hotcall_for.h>

struct loop_stack_item {
    unsigned int body_len;
    unsigned int index;
    unsigned int n_iters;
    unsigned int offset;
};

static inline unsigned int
update_loop_body_vector_variables(struct loop_stack_item *loop_stack, unsigned int loop_stack_pos) {
    unsigned int offset = 0, power = 0;
    for(int k = loop_stack_pos; k >= 0; --k) {
        offset += loop_stack[k].index * pow(loop_stack[k].n_iters, power++);
    }
    return offset;
}


static inline unsigned int
hotcall_handle_for_begin(struct hotcall_for_start *for_s, struct loop_stack_item *loop_stack, unsigned int loop_stack_pos, uint8_t *exclude_list, int n) {
    unsigned int n_iters = *for_s->config->n_iters;
    if(n_iters == 0) {
        memset(exclude_list + n, 1, for_s->config->body_len + 2);
    } else {
        loop_stack[loop_stack_pos] = (struct loop_stack_item) {
            .body_len = for_s->config->body_len,
            .index = 0,
            .n_iters = n_iters,
        };
        loop_stack[loop_stack_pos].offset = update_loop_body_vector_variables(loop_stack, loop_stack_pos);
        loop_stack_pos++;
    }
    return loop_stack_pos;
}

static inline unsigned int
hotcall_handle_for_end(struct loop_stack_item *loop_stack, unsigned int loop_stack_pos, int *n, uint8_t *exclude_list) {
    if(++loop_stack[loop_stack_pos - 1].index < loop_stack[loop_stack_pos - 1].n_iters) {
        *n -= (loop_stack[loop_stack_pos - 1].body_len + 1);
        memset(exclude_list + *n + 1, 0, loop_stack[loop_stack_pos - 1].body_len);
        loop_stack[loop_stack_pos - 1].offset++;
        //goto continue_loop;
    } else {
        loop_stack_pos--;
    }
    return loop_stack_pos;
}

#endif
