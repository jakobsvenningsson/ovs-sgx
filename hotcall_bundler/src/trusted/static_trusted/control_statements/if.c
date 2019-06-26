#include "if.h"
#include "predicate.h"

bool
hotcall_handle_if(struct hotcall_if *tif, struct hotcall_config *hotcall_config, uint8_t *exclude_list, int pos, int exclude_list_len, int offset) {
    int res = evaluate_predicate(tif->config->postfix, tif->config->postfix_length, hotcall_config, offset);

    if(res && tif->config->else_branch_len > 0) {
        exclude_else_branch(exclude_list, pos, tif->config->then_branch_len, tif->config->else_branch_len);
    } else if(!res) {
        if(tif->config->then_branch_len > 0) {
            exclude_if_branch(exclude_list, pos, tif->config->then_branch_len);
        }
        if(tif->config->return_if_false) {
            exclude_rest(exclude_list, pos, tif->config->then_branch_len ,tif->config->else_branch_len, exclude_list_len);
        }
    }
}
