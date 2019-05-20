#ifndef _H_OFTABLE_
#define _H_OFTABLE_

void
sgx_table_dpif_init(uint8_t bridge_id, int n_tables);
void
oftable_init(struct oftable * table);
void
sgx_table_cls_init(uint8_t bridge_id);
void
sgx_oftable_destroy(uint8_t bridge_id, uint8_t table_id);

struct oftable *
next_matching_table(uint8_t bridge_id, int ofproto_n_tables, const struct oftable * table, uint8_t table_id);
struct oftable *
first_matching_table(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id);
struct oftable *
next_visible_table(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id);
#ifdef ARG_OPT_2
void
ecall_add_flow_2(struct add_flow_args *args);
#endif


void
delete_flows(uint8_t bridge_id,
                 struct cls_rule **cls_rules,
                 uint32_t *rule_hashes,
                 unsigned int *rule_priorities,
                 struct match *match,
                 size_t n);

#endif
