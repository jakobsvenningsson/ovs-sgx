#ifndef _H_CLS_RULE_
#define _H_CLS_RULE_

#include "enclave.h"

void
sgx_cls_rule_init_i(uint8_t bridge_id, struct cls_rule * cls_rule, const struct match * match, unsigned int priority);
/*struct sgx_cls_rule *
node_insert(uint8_t bridge_id, uint32_t hash);*/
void
node_delete(uint8_t bridge_id, struct cls_rule * out);
struct sgx_cls_rule *
sgx_rule_from_ut_cr(uint8_t bridge_id, const struct cls_rule * out);
bool
is_rule_hidden(uint8_t bridge_id, struct cls_rule *ut_cr);
bool
is_rule_modifiable(uint8_t bridge_id, uint8_t table_id);
#endif
