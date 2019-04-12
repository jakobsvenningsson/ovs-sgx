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

//for testing
struct rul *out;
///

// Optimized calls

size_t SGX_get_cls_rules(int bridge_id,
						 int table_id,
						 size_t start_index,
						 size_t end_index,
						 struct cls_rule ** buf,
						 size_t buf_size,
						 size_t *n_rules);


size_t SGX_get_cls_rules_and_enable_eviction(int bridge_id,
											 int table_id,
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

void SGX_eviction_group_add_rules(int bridge_id,
								  int table_id,
								  size_t n,
								  struct cls_rule **cls_rules,
								  struct heap_node *evg_nodes,
								  uint32_t *rule_priorities,
								  uint32_t group_priority);

size_t
SGX_ofproto_get_vlan_usage(int bridge_id,
                         size_t default_buffer_size,
                         uint16_t *vlan_buffer,
                         size_t start_index,
                         size_t end_index,
                         size_t *n_vlan);

size_t
SGX_ofproto_flush(int bridge_id,
				  struct cls_rule **cls_rules,
				  uint32_t *hashes,
				  size_t default_buffer_size,
				  size_t start_index,
				  size_t end_index,
				  size_t *n_rules);

size_t
SGX_ofproto_evict(int bridge_id,
                int ofproto_n_tables,
				bool *pendings,
				struct cls_rule **all_cls_rules,
				size_t m,
                uint32_t *hashes,
                struct cls_rule **cls_rules,
                size_t buf_size,
                size_t *n_evictions);

void
SGX_add_flow(int bridge_id,
			 int table_id,
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
 SGX_collect_rules_strict(int bridge_id,
	 					 int table_id,
						 int n_tables,
						 struct match *match,
						 unsigned int priority,
                         bool *rule_is_hidden_buffer,
                         struct cls_rule **cls_rule_buffer,
						 bool *rule_is_modifiable,
                         size_t buffer_size);




 size_t
 SGX_collect_rules_loose(int bridge_id,
                        int table_id,
                        int ofproto_n_tables,
                        size_t start_index,
                        struct match *match,
                        bool *rule_is_hidden_buffer,
                        struct cls_rule **cls_rule_buffer,
                        size_t buffer_size,
						bool *rule_is_modifiable,
                        size_t *n_rules);

void
SGX_delete_flows(int bridge_id,
				 int *rule_table_ids,
				 struct cls_rule **cls_rules,
				 bool *rule_is_hidden,
				 uint32_t *rule_hashes,
				 unsigned int *rule_priorities,
				 struct match *match, size_t n);


 void
 SGX_configure_table(int bridge_id,
                     int table_id,
                     char *name,
                     unsigned int max_flows,
                     struct mf_subfield *groups,
                     size_t n_groups,
                     bool *need_to_evict,
                     bool *is_read_only);

 bool
 SGX_need_to_evict(int bridge_id, int table_id);

int sgx_ofproto_init_tables(int n_tables);
void SGX_readonly_set(int bridge_id, int table_id);
int SGX_istable_readonly(int bridge_id, uint8_t table_id);
void SGX_cls_rule_init(int bridge_id, struct cls_rule * o_cls_rule,
		const struct match * match , unsigned int priority);
int SGX_cr_rule_overlaps(int bridge_id, int table_id,struct cls_rule * o_cls_rule);
void SGX_cls_rule_destroy(int bridge_id, struct cls_rule *o_cls_rule);
int SGX_cls_rule_equal(int bridge_id, const struct cls_rule *o_cls_rule_a,
		const struct cls_rule *o_cls_rule_b);
uint32_t SGX_cls_rule_hash(int bridge_id, const struct cls_rule *o_cls_rule, uint32_t basis);
void SGX_classifier_replace(int bridge_id, int table_id,struct cls_rule* o_cls_rule,struct cls_rule ** cls_rule_rtrn);
enum oftable_flags SGX_rule_get_flags (int bridge_id, int table_id);
int SGX_cls_count(int bridge_id, int table_id);
int SGX_eviction_fields_enable(int bridge_id, int table_id);
void SGX_table_mflows_set(int bridge_id, int table_id,unsigned int value);
size_t SGX_evg_add_rule(int bridge_id, int table_id, struct cls_rule *o_cls_rule,uint32_t priority,
		uint32_t rule_evict_prioriy,struct heap_node rule_evg_node);
void SGX_evg_group_resize(int bridge_id, int table_id,struct cls_rule *o_cls_rule,size_t priority, struct eviction_group *evg);
int SGX_evg_remove_rule(int bridge_id, int table_id,struct cls_rule *o_cls_rule);
void SGX_cls_remove(int bridge_id, int table_id,struct cls_rule *o_cls_rule);
void SGX_choose_rule_to_evict(int bridge_id, int table_id,struct cls_rule **o_cls_rule);
unsigned int SGX_table_mflows(int bridge_id, int table_id);
struct cls_rule * SGX_choose_rule_to_evict_p(int bridge_id, int table_id, struct cls_rule **o_cls_rule, struct cls_rule *replacer);

void SGX_minimatch_expand(int bridge_id, struct cls_rule *o_cls_rule,struct match *dst);
unsigned int SGX_cr_priority(int bridge_id, const struct cls_rule *o_cls_rule);
void sgx_oftable_check_hidden(int bridge_id);
void SGX_cls_find_match_exactly(int bridge_id, int table_id,
		const struct match *target,
		unsigned int priority,struct cls_rule **o_cls_rule);

int SGX_femt_ccfe_c(int bridge_id, int ofproto_n_tables,uint8_t table_id,const struct match *match);
void SGX_femt_ccfe_r(int bridge_id, int ofproto_n_tables,struct cls_rule **buf,int elem,uint8_t table_id,const struct match *match);
int SGX_ecall_femt_c(int bridge_id, int ofproto_n_tables,uint8_t table_id,const struct match *match,unsigned int priority);
void SGX_ecall_femt_r(int bridge_id, int ofproto_n_tables,struct cls_rule **buf,int elem,uint8_t table_id,const struct match *match,unsigned int priority);






int SGX_ccfe_c(int bridge_id, int table_id);
void SGX_ccfe_r(int bridge_id, struct cls_rule **buf,int elem,int table_id);
void SGX_oftable_enable_eviction(int bridge_id, int table_id,const struct mf_subfield *fields,size_t n_fields,uint32_t random_v, bool *no_change);
void SGX_oftable_disable_eviction(int bridge_id, int table_id);

void SGX_ofproto_destroy(int bridge_id);
unsigned int SGX_total_rules(int bridge_id);
void SGX_table_name(int bridge_id, int table_id,char *buf,size_t len);
int SGX_collect_ofmonitor_util_c(int bridge_id, int ofproto_n_tables,int table_id,const struct minimatch *match);
void SGX_collect_ofmonitor_util_r(int bridge_id, int ofproto_n_tables,struct cls_rule **buf,int elem,int table_id,const struct minimatch *match);
int SGX_cls_rule_is_loose_match(int bridge_id, struct cls_rule *o_cls_rule,const struct minimatch *criteria);
int SGX_fet_ccfes_c(int bridge_id);
void SGX_fet_ccfes_r(int bridge_id, struct cls_rule **buf,int elem);

int SGX_fet_ccfe_c(int bridge_id);
void SGX_fet_ccfe_r(int bridge_id, struct cls_rule **buf,int elem);

void SGX_cls_lookup(int bridge_id, struct cls_rule **o_cls_rule,int table_id,const struct flow *flow,
		struct flow_wildcards *wc);

unsigned int SGX_cls_rule_priority(int bridge_id, struct cls_rule *o_cls_rule);

int SGX_desfet_ccfes_c(int bridge_id);
void SGX_desfet_ccfes_r(int bridge_id, struct cls_rule **buf,int elem);
unsigned int SGX_cls_rule_format(int bridge_id, const struct cls_rule *o_cls_rule,struct match *megamatch);
void SGX_miniflow_expand(int bridge_id, struct cls_rule *o_cls_rule,struct flow *flow);
uint32_t SGX_rule_calculate_tag(int bridge_id, struct cls_rule *o_cls_rule,const struct flow *flow,int table_id);


int SGX_table_update_taggable(int bridge_id, uint8_t table_id);
int SGX_is_sgx_other_table(int bridge_id, int id);
uint32_t SGX_rule_calculate_tag_s(int bridge_id, int id,const struct flow *flow);
void SGX_oftable_set_name(int bridge_id, int table_id, char *name);
uint16_t SGX_minimask_get_vid_mask(int bridge_id, struct cls_rule *o_cls_rule);
uint16_t SGX_miniflow_get_vid(int bridge_id, struct cls_rule *o_cls_rule);


int SGX_ofproto_get_vlan_usage_c(int bridge_id);
void SGX_ofproto_get_vlan_usage__r(int bridge_id, uint16_t *buf,int elem);





#endif /* !_APP_H_ */
