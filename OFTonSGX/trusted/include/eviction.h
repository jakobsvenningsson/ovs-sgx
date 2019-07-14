#ifndef _H_EVICTION_
#define _H_EVICTION_

static struct eviction_group *
sgx_evg_find(struct ovs_enclave_ctx *e_ctx, uint8_t bridge_id, uint8_t table_id, uint32_t evg_id, uint32_t priority);
static uint32_t
eviction_group_hash_rule(struct ovs_enclave_ctx *e_ctx, uint8_t bridge_id, uint8_t table_id, struct cls_rule * cls_rule);
void
sgx_evg_destroy(struct ovs_enclave_ctx *e_ctx, uint8_t bridge_id, uint8_t table_id, struct eviction_group * evg);
void
choose_rule_to_evict(struct ovs_enclave_ctx *e_ctx, uint8_t bridge_id, uint8_t table_id, struct sgx_cls_rule ** o_cls_rule, struct sgx_cls_rule **exclude_rules, size_t n_excluded);
void
sgx_evg_group_resize(struct ovs_enclave_ctx *e_ctx, uint8_t bridge_id, uint8_t table_id, struct cls_rule * o_cls_rule, size_t priority, struct eviction_group * evg);
uint32_t
eviction_group_priority(size_t n_rules);

#endif
