#ifndef _APP_H_
#define _APP_H_
#include <stdint.h>

/* API untrusted functions to trusted inside the enclave */
struct match;
struct cls_rule;
struct heap_node;
struct match;
struct mf_subfield;
struct minimatch;
struct flow;
struct flow_wildcards;


void SGX_async_test();


// Optimized calls

size_t SGX_get_cls_rules(uint8_t bridge_id,
						 uint8_t table_id,
						 size_t start_index,
						 size_t end_index,
						 struct cls_rule ** buf,
						 size_t buf_size,
						 size_t *n_rules);


size_t SGX_get_cls_rules_and_enable_eviction(uint8_t bridge_id,
											 uint8_t table_id,
											 size_t start_index,
											 size_t end_index,
											 struct cls_rule ** buf,
											 size_t buf_size,
											 size_t *n_rules,
										 	 const struct mf_subfield *fields,
										 	 size_t n_fields,
									 	 	 uint32_t random_v,
								 		 	 bool *no_change,
										 	 bool *is_eviction_fields_enabled);

void SGX_eviction_group_add_rules(uint8_t bridge_id,
								  uint8_t table_id,
								  size_t n,
								  struct cls_rule **cls_rules,
								  struct heap_node *evg_nodes,
								  uint32_t *rule_priorities,
								  uint32_t group_priority);

size_t
SGX_ofproto_get_vlan_usage(uint8_t bridge_id,
                         size_t default_buffer_size,
                         uint16_t *vlan_buffer,
                         size_t start_index,
                         size_t end_index,
                         size_t *n_vlan);

size_t
SGX_ofproto_flush(uint8_t bridge_id,
				  struct cls_rule **cls_rules,
				  uint32_t *hashes,
				  size_t default_buffer_size,
				  size_t start_index,
				  size_t end_index,
				  size_t *n_rules);

size_t
SGX_ofproto_evict(uint8_t bridge_id,
                int ofproto_n_tables,
				size_t start_index,
                uint32_t *hashes,
                struct cls_rule **cls_rules,
                size_t buf_size,
                size_t *n_evictions);

void
SGX_add_flow(uint8_t bridge_id,
			 uint8_t table_id,
			 struct cls_rule *cr,
			 struct cls_rule **victim,
			 struct cls_rule **evict,
			 struct match *match,
			 uint32_t *evict_rule_hash,
			 uint16_t *vid,
			 uint16_t *vid_mask,
			 unsigned int priority,
			 uint16_t flags,
			 uint32_t group_eviction_priority,
			 uint32_t rule_eviction_priority,
			 struct heap_node eviction_node,
			 struct cls_rule **pending_deletions,
			 int n_pending,
			 bool has_timeout,
			 bool *table_overflow,
			 bool *is_rule_modifiable,
			 bool *is_rule_overlapping,
			 bool *is_deletion_pending,
			 bool *is_read_only);

 size_t
 SGX_collect_rules_strict(uint8_t bridge_id,
	 					 uint8_t table_id,
						 int n_tables,
						 struct match *match,
						 unsigned int priority,
                         bool *rule_is_hidden_buffer,
                         struct cls_rule **cls_rule_buffer,
						 bool *rule_is_modifiable,
                         size_t buffer_size);




 size_t
 SGX_collect_rules_loose(uint8_t bridge_id,
                        uint8_t table_id,
                        int ofproto_n_tables,
                        size_t start_index,
                        struct match *match,
                        bool *rule_is_hidden_buffer,
                        struct cls_rule **cls_rule_buffer,
                        size_t buffer_size,
						bool *rule_is_modifiable,
                        size_t *n_rules);

void
SGX_delete_flows(uint8_t bridge_id,
				 uint8_t *rule_table_ids,
				 struct cls_rule **cls_rules,
				 bool *rule_is_hidden,
				 uint32_t *rule_hashes,
				 unsigned int *rule_priorities,
				 struct match *match, size_t n);


 void
 SGX_configure_table(uint8_t bridge_id,
                     uint8_t table_id,
                     char *name,
                     unsigned int max_flows,
                     struct mf_subfield *groups,
                     size_t n_groups,
                     bool *need_to_evict,
                     bool *is_read_only);

 bool
 SGX_need_to_evict(uint8_t bridge_id, uint8_t table_id);


 size_t
 SGX_collect_rules_loose_stats_request(uint8_t bridge_id,
                                       uint8_t table_id,
                                       int n_tables,
                                       size_t start_index,
                                       size_t buffer_size,
                                       struct match *match,
                                       struct cls_rule **cls_rules,
                                       struct match *matches,
                                       unsigned int *priorities,
                                       size_t *n_rules);


void
SGX_remove_rules(uint8_t bridge_id, uint8_t *table_ids, struct cls_rule **rules, bool *is_hidden, size_t n_rules);


void
SGX_ofproto_rule_send_removed(uint8_t bridge_id, struct cls_rule *cr, struct match *match, unsigned int *priority, bool *rule_is_hidden);

void
SGX_cls_rules_format(uint8_t bridge_id, const struct cls_rule *cls_rules, struct match *megamatches, size_t n);


void
SGX_minimatch_expand_and_get_priority(uint8_t bridge_id, struct cls_rule *ut_cr, struct match *match, unsigned int *priority);

uint32_t
SGX_miniflow_expand_and_tag(uint8_t bridge_id, struct cls_rule *ut_cr, struct flow *flow, uint8_t table_id);


//size_t
//SGX_ofproto_evict_get_rest(uint32_t *rule_hashes, struct cls_rule ** cls_rules, size_t buf_size);

int sgx_ofproto_init_tables(int n_tables);
void SGX_readonly_set(uint8_t bridge_id, uint8_t table_id);
int SGX_istable_readonly(uint8_t bridge_id, uint8_t table_id);
void SGX_cls_rule_init(uint8_t bridge_id, struct cls_rule * o_cls_rule,
		const struct match * match , unsigned int priority);
int SGX_cr_rule_overlaps(uint8_t bridge_id, uint8_t table_id,struct cls_rule * o_cls_rule);
void SGX_cls_rule_destroy(uint8_t bridge_id, struct cls_rule *o_cls_rule);
int SGX_cls_rule_equal(uint8_t bridge_id, const struct cls_rule *o_cls_rule_a,
		const struct cls_rule *o_cls_rule_b);
uint32_t SGX_cls_rule_hash(uint8_t bridge_id, const struct cls_rule *o_cls_rule, uint32_t basis);
void SGX_classifier_replace(uint8_t bridge_id, uint8_t table_id,struct cls_rule* o_cls_rule,struct cls_rule ** cls_rule_rtrn);
enum oftable_flags SGX_rule_get_flags (uint8_t bridge_id, uint8_t table_id);
int SGX_cls_count(uint8_t bridge_id, uint8_t table_id);
int SGX_eviction_fields_enable(uint8_t bridge_id, uint8_t table_id);
void SGX_table_mflows_set(uint8_t bridge_id, uint8_t table_id,unsigned int value);
void SGX_evg_add_rule(uint8_t bridge_id, uint8_t table_id, struct cls_rule *o_cls_rule,uint32_t priority,
		uint32_t rule_evict_prioriy,struct heap_node rule_evg_node);
void SGX_evg_group_resize(uint8_t bridge_id, uint8_t table_id,struct cls_rule *o_cls_rule,size_t priority, struct eviction_group *evg);
void SGX_evg_remove_rule(uint8_t bridge_id, uint8_t table_id,struct cls_rule *o_cls_rule);
void SGX_cls_remove(uint8_t bridge_id, uint8_t table_id,struct cls_rule *o_cls_rule);
void SGX_choose_rule_to_evict(uint8_t bridge_id, uint8_t table_id,struct cls_rule **o_cls_rule);
unsigned int SGX_table_mflows(uint8_t bridge_id, uint8_t table_id);
void SGX_choose_rule_to_evict_p(uint8_t bridge_id, uint8_t table_id, struct cls_rule **o_cls_rule, struct cls_rule *replacer);

void SGX_minimatch_expand(uint8_t bridge_id, struct cls_rule *o_cls_rule,struct match *dst);
unsigned int SGX_cr_priority(uint8_t bridge_id, const struct cls_rule *o_cls_rule);
void sgx_oftable_check_hidden(uint8_t bridge_id);
void SGX_cls_find_match_exactly(uint8_t bridge_id, uint8_t table_id,
		const struct match *target,
		unsigned int priority,struct cls_rule **o_cls_rule);

size_t SGX_collect_rules_loose_c(uint8_t bridge_id, int ofproto_n_tables,uint8_t table_id,const struct match *match);
void SGX_collect_rules_loose_r(uint8_t bridge_id, int ofproto_n_tables,struct cls_rule **buf,int elem,uint8_t table_id,const struct match *match);
size_t SGX_collect_rules_strict_c(uint8_t bridge_id, int ofproto_n_tables,uint8_t table_id,const struct match *match,unsigned int priority);
void SGX_collect_rules_strict_r(uint8_t bridge_id, int ofproto_n_tables,struct cls_rule **buf,int elem,uint8_t table_id,const struct match *match,unsigned int priority);






size_t SGX_ccfe_c(uint8_t bridge_id, uint8_t table_id);
void SGX_ccfe_r(uint8_t bridge_id, struct cls_rule **buf,int elem,uint8_t table_id);
void SGX_oftable_enable_eviction(uint8_t bridge_id, uint8_t table_id,const struct mf_subfield *fields,size_t n_fields,uint32_t random_v, bool *no_change);
void SGX_oftable_disable_eviction(uint8_t bridge_id, uint8_t table_id);

void SGX_ofproto_destroy(uint8_t bridge_id);
unsigned int SGX_total_rules(uint8_t bridge_id);
void SGX_table_name(uint8_t bridge_id, uint8_t table_id,char *buf,size_t len);
size_t SGX_collect_ofmonitor_util_c(uint8_t bridge_id, int ofproto_n_tables,uint8_t table_id,const struct minimatch *match);
void SGX_collect_ofmonitor_util_r(uint8_t bridge_id, int ofproto_n_tables,struct cls_rule **buf,int elem,uint8_t table_id,const struct minimatch *match);
int SGX_cls_rule_is_loose_match(uint8_t bridge_id, struct cls_rule *o_cls_rule,const struct minimatch *criteria);
size_t SGX_fet_ccfes_c(uint8_t bridge_id);
void SGX_fet_ccfes_r(uint8_t bridge_id, struct cls_rule **buf,int elem);

size_t SGX_fet_ccfe_c(uint8_t bridge_id);
void SGX_fet_ccfe_r(uint8_t bridge_id, struct cls_rule **buf,int elem);

void SGX_cls_lookup(uint8_t bridge_id, struct cls_rule **o_cls_rule,uint8_t table_id,const struct flow *flow,
		struct flow_wildcards *wc);

size_t SGX_desfet_ccfes_c(uint8_t bridge_id);
void SGX_desfet_ccfes_r(uint8_t bridge_id, struct cls_rule **buf,int elem);
unsigned int SGX_cls_rule_format(uint8_t bridge_id, const struct cls_rule *o_cls_rule,struct match *megamatch);
void SGX_miniflow_expand(uint8_t bridge_id, struct cls_rule *o_cls_rule,struct flow *flow);
uint32_t SGX_rule_calculate_tag(uint8_t bridge_id, struct cls_rule *o_cls_rule,const struct flow *flow,uint8_t table_id);


int SGX_table_update_taggable(uint8_t bridge_id, uint8_t table_id);
int SGX_is_sgx_other_table(uint8_t bridge_id, uint8_t table_id);
uint32_t SGX_rule_calculate_tag_s(uint8_t bridge_id, uint8_t table_id,const struct flow *flow);
void SGX_oftable_set_name(uint8_t bridge_id, uint8_t table_id, char *name);
uint16_t SGX_minimask_get_vid_mask(uint8_t bridge_id, struct cls_rule *o_cls_rule);
uint16_t SGX_miniflow_get_vid(uint8_t bridge_id, struct cls_rule *o_cls_rule);


size_t SGX_ofproto_get_vlan_usage_c(uint8_t bridge_id);
void SGX_ofproto_get_vlan_usage__r(uint8_t bridge_id, uint16_t *buf,int elem);





#endif /* !_APP_H_ */
