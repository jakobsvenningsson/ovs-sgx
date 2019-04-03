#include <string.h>
#include <assert.h>
#include <libgen.h>
#include "sgx-utils.h"
#include "enclave_u.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "app.h"
#include <stdint.h>
#include <stdbool.h>

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;
static bool enclave_is_initialized = false;
static int bridge_counter = 0;
//1. Creation and Initialization of tables
int
sgx_ofproto_init_tables(int n_tables) {
	printf("INSIDE SGX START......\n");
	int ecall_return;
	if (!enclave_is_initialized){
	    if(initialize_enclave(&global_eid) < 0){
	      return -1;
	    }
	    enclave_is_initialized = true;
	}
  int this_bridge_id = bridge_counter++;
  ecall_ofproto_init_tables(global_eid, this_bridge_id, n_tables);
  return this_bridge_id;
}

void
SGX_readonly_set(int bridge_id, int table_id) {
  //printf("SGX_readonly_set\n");
	ecall_readonly_set(global_eid, bridge_id, table_id);
  //printf("end\n");

}

int
SGX_istable_readonly(int bridge_id, uint8_t table_id){
  //printf("SGX_istable_readonly\n");
	int ecall_return;
	ecall_istable_readonly(global_eid,&ecall_return, bridge_id, table_id);
  //printf("end\n");
	return ecall_return;
}

void
SGX_cls_rule_init(int bridge_id, struct cls_rule * o_cls_rule,
		const struct match * match , unsigned int priority){
  //printf("SGX_cls_rule_init\n");
	ecall_cls_rule_init(global_eid, bridge_id, o_cls_rule,match,priority);
  //printf("end\n");

}

//4. SGX Classifier_rule_overlap
int
SGX_cr_rule_overlaps(int bridge_id, int table_id,struct cls_rule * o_cls_rule){
  //printf("SGX_cr_rule_overlaps\n");

	int ecall_return;
	ecall_cr_rule_overlaps(global_eid,&ecall_return, bridge_id, table_id,o_cls_rule);
  //printf("end\n");

	return ecall_return;
}

//5. SGX_CLS_RULE_DESTROY
void SGX_cls_rule_destroy(int bridge_id, struct cls_rule *o_cls_rule){
  //printf("SGX_cls_rule_destroy\n");

	ecall_cls_rule_destroy(global_eid, bridge_id, o_cls_rule);
  //printf("end\n");

}

//6. cls_rule_hash
uint32_t SGX_cls_rule_hash(int bridge_id, const struct cls_rule *o_cls_rule, uint32_t basis){
  //printf("SGX_cls_rule_hash\n");

	int ecall_return;
	ecall_cls_rule_hash(global_eid,&ecall_return, bridge_id, o_cls_rule,basis);
  //printf("end\n");

	return ecall_return;
}
//7. cls_rule_equal
int SGX_cls_rule_equal(int bridge_id, const struct cls_rule *o_cls_rule_a,
		const struct cls_rule *o_cls_rule_b){
      //printf("SGX_cls_rule_equal\n");

	int ecall_return;
	ecall_cls_rule_equal(global_eid,&ecall_return, bridge_id, o_cls_rule_a,o_cls_rule_b);
  //printf("end\n");

	return ecall_return;
}

//8. classifier_replace
void SGX_classifier_replace(int bridge_id, int table_id,struct cls_rule* o_cls_rule,struct cls_rule ** cls_rule_rtrn){
  //printf("SGX_classifier_replace\n");

	ecall_classifier_replace(global_eid, bridge_id, table_id,o_cls_rule,cls_rule_rtrn);
  //printf("end\n");

}

//9 rule_get_flags
enum oftable_flags SGX_rule_get_flags (int bridge_id, int table_id){
  //printf("SGX_rule_get_flags\n");

	enum oftable_flags m;
	ecall_rule_get_flags(global_eid, &m, bridge_id, table_id);
  //printf("end\n");

	return m;
}
//10. classifier count of cls_rules
int SGX_cls_count(int bridge_id, int table_id){
  //printf("SGX_cls_count\n");

	int ecall_return;
	ecall_cls_count(global_eid,&ecall_return, bridge_id, table_id);
  //printf("end\n");

	return ecall_return;
}

//11. is eviction_fields in the table with table_id enabled?
int SGX_eviction_fields_enable(int bridge_id, int table_id){
  //printf("SGX_eviction_fields_enable\n");

	int result;
	ecall_eviction_fields_enable(global_eid,&result, bridge_id, table_id);
  //printf("end\n");

	return result;
}

//12.Add a rule to a eviction group
size_t SGX_evg_add_rule(int bridge_id, int table_id, struct cls_rule *o_cls_rule,uint32_t priority,
		uint32_t rule_evict_prioriy,struct heap_node rule_evg_node){
      //printf("SGX_evg_add_rule\n");


	size_t result;
	ecall_evg_add_rule(global_eid,&result, bridge_id, table_id,o_cls_rule,priority,
			rule_evict_prioriy,rule_evg_node);
      //printf("end\n");

	return result;
}

//13. void ecall_evg_group_resize
void SGX_evg_group_resize(int bridge_id, int table_id,struct cls_rule *o_cls_rule,size_t priority, struct eviction_group *evg){
  //printf("SGX_evg_group_resize\n");

	ecall_evg_group_resize(global_eid, bridge_id, table_id,o_cls_rule,priority, evg);
  //printf("end\n");

}

//14. Remove the evict group where a rule belongs to
int SGX_evg_remove_rule(int bridge_id, int table_id,struct cls_rule *o_cls_rule){
  //printf("SGX_evg_remove_rule\n");

	int result;
	ecall_evg_remove_rule(global_eid,&result, bridge_id, table_id,o_cls_rule);
  //printf("end\n");

	return result;
}

//15. Removes a cls_rule from the classifier
void SGX_cls_remove(int bridge_id, int table_id,struct cls_rule *o_cls_rule){
  //printf("SGX_cls_remove\n");

	ecall_cls_remove(global_eid, bridge_id, table_id, o_cls_rule);
  //printf("end\n");

}

//16. SGX choose a cls_rule to evict from table
void SGX_choose_rule_to_evict(int bridge_id, int table_id,struct cls_rule **o_cls_rule){
  //printf("SGX_choose_rule_to_evict\n");

	ecall_choose_rule_to_evict(global_eid, bridge_id, table_id, o_cls_rule);
  //printf("end\n");

}

//17.
struct cls_rule * SGX_choose_rule_to_evict_p(int bridge_id, int table_id, struct cls_rule **o_cls_rule, struct cls_rule *replacer){
  //printf("SGX_choose_rule_to_evict_p\n");
  ////printf("%d %d %p %p \n", bridge_id, table_id, *o_cls_rule, replacer);
  struct cls_rule *tmp;
	ecall_choose_rule_to_evict_p(global_eid, &tmp, bridge_id, table_id, o_cls_rule, replacer);
  //printf("end\n");
  return tmp;

}

//18 returns table max flow
unsigned int SGX_table_mflows(int bridge_id, int table_id){
  //printf("SGX_table_mflows\n");

	unsigned int result;
	ecall_table_mflows(global_eid,&result, bridge_id, table_id);
  //printf("end\n");

	return result;
}

//19 set table max flow to value
void SGX_table_mflows_set(int bridge_id, int table_id,unsigned int value){
  //printf("SGX_table_mflows_set\n");

	ecall_table_mflows_set(global_eid, bridge_id, table_id,value);
  //printf("end\n");

}

//19 minimatch_expand
void SGX_minimatch_expand(int bridge_id, struct cls_rule *o_cls_rule,struct match *dst){
  //printf("SGX_minimatch_expand\n");


	ecall_minimatch_expand(global_eid, bridge_id, o_cls_rule,dst);
  //printf("end\n");

}

//20. cls_rule priority
unsigned int SGX_cr_priority(int bridge_id, const struct cls_rule *o_cls_rule){
  //printf("SGX_cr_priority\n");

	unsigned result;
	ecall_cr_priority(global_eid, &result, bridge_id, o_cls_rule);
  //printf("end\n");

	return result;
}

//21  classifier find match exactly
void SGX_cls_find_match_exactly(int bridge_id, int table_id,
		const struct match *target,
		unsigned int priority,struct cls_rule **o_cls_rule){
      //printf("SGX_cls_find_match_exactly\n");


	ecall_cls_find_match_exactly(global_eid, bridge_id, table_id,target,priority,o_cls_rule);
  //printf("end\n");

}

//22. SGX FOR_EACH_MATCHING_TABLE + CLS_CURSOR_FOR_EACH (count and request

//22.1 Count
int SGX_femt_ccfe_c(int bridge_id, int ofproto_n_tables,uint8_t table_id,const struct match *match){
  //printf("SGX_femt_ccfe_c\n");

	int result;
	ecall_femt_ccfe_c(global_eid,&result, bridge_id, ofproto_n_tables,table_id,match);
  //printf("end\n");

	return result;
}

//22.2 Request
void SGX_femt_ccfe_r(int bridge_id, int ofproto_n_tables,struct cls_rule **buf,int elem,uint8_t table_id,const struct match *match){
  //printf("SGX_femt_ccfe_r\n");

	ecall_femt_ccfe_r(global_eid, bridge_id, ofproto_n_tables,buf,elem,table_id,match);
  //printf("end\n");

}

//23. SGX FOR_EACH_MATCHING_TABLE get the rules

//23.1 Count
int SGX_ecall_femt_c(int bridge_id, int ofproto_n_tables,uint8_t table_id,const struct match *match,unsigned int priority){
  //printf("SGX_ecall_femt_c\n");

	int buf_size;
    ecall_femt_c(global_eid,&buf_size, bridge_id, ofproto_n_tables,table_id,match,priority);
    return buf_size;
    //printf("end\n");

}

//23.2 Request
void SGX_ecall_femt_r(int bridge_id, int ofproto_n_tables,struct cls_rule **buf,int elem,uint8_t table_id,const struct match *match,unsigned int priority)
{
  //printf("SGX_ecall_femt_r\n");

	ecall_femt_r(global_eid, bridge_id, ofproto_n_tables,buf,elem,table_id,match,priority);
  //printf("end\n");

}

//24 CLS_CURSOR_FOR_EACH
//24.1 Count
int SGX_ccfe_c(int bridge_id, int table_id){
  //printf("SGX_ccfe_c\n");

	int buffer_size;
	ecall_ccfe_c(global_eid,&buffer_size, bridge_id, table_id);
  //printf("end\n");

	return buffer_size;
}
//24.2 Request
void SGX_ccfe_r(int bridge_id, struct cls_rule **buf,int elem,int table_id){
  //printf("SGX_ccfe_r\n");

	ecall_ccfe_r(global_eid, bridge_id, buf,elem,table_id);
  //printf("end\n");

}

int SGX_collect_ofmonitor_util_c(int bridge_id, int ofproto_n_tables,int table_id,const struct minimatch *match){
  //printf("SGX_collect_ofmonitor_util_c\n");

	int count;
	ecall_collect_ofmonitor_util_c(global_eid,&count, bridge_id, ofproto_n_tables,table_id,match);
  //printf("end\n");

	return count;
}

void SGX_collect_ofmonitor_util_r(int bridge_id, int ofproto_n_tables,struct cls_rule **buf,int elem,int table_id,const struct minimatch *match){
  //printf("SGX_collect_ofmonitor_util_r\n");

 ecall_collect_ofmonitor_util_r(global_eid, bridge_id, ofproto_n_tables,buf,elem,table_id,match);
 //printf("end\n");

}



//25. One Part of Enable_eviction
void SGX_oftable_enable_eviction(int bridge_id, int table_id,const struct mf_subfield *fields,size_t n_fields,uint32_t random_v, bool *no_change){
  //printf("SGX_oftable_enable_eviction\n");

	ecall_oftable_enable_eviction(global_eid, bridge_id, table_id,fields,n_fields,random_v, no_change);
  //printf("end\n");

}

//25.1
void SGX_oftable_disable_eviction(int bridge_id, int table_id){
  //printf("SGX_oftable_disable_eviction\n");

	ecall_oftable_disable_eviction(global_eid, bridge_id, table_id);
  //printf("end\n");

}

//26 oftable destroy
void SGX_ofproto_destroy(int bridge_id){
  //printf("SGX_ofproto_destroy\n");

	ecall_ofproto_destroy(global_eid, bridge_id);
  //printf("end\n");

}

//27 Count total number of rules
unsigned int SGX_total_rules(int bridge_id){
  //printf("SGX_total_rules\n");

	unsigned int n_rules;
	ecall_total_rules(global_eid, &n_rules, bridge_id);
  //printf("end\n");

	return n_rules;
}

//28 Copy the name of the table
void SGX_table_name(int bridge_id, int table_id,char *buf,size_t len){
  //printf("SGX_table_name\n");

 ecall_table_name(global_eid, bridge_id, table_id,buf,len);
 //printf("end\n");

}

//29 loose_match
int SGX_cls_rule_is_loose_match(int bridge_id, struct cls_rule *o_cls_rule,const struct minimatch *criteria){
  //printf("SGX_cls_rule_is_loose_match\n");

	int result;
 	ecall_cls_rule_is_loose_match(global_eid,&result, bridge_id, o_cls_rule,criteria);
  //printf("end\n");

 	return result;
}

//30. Dependencies for ofproto_flush__
int SGX_fet_ccfes_c(int bridge_id){
  //printf("SGX_fet_ccfes_c\n");

	int count;
	ecall_fet_ccfes_c(global_eid,&count, bridge_id);
  //printf("end\n");

	return count;
}

//30.1
void SGX_fet_ccfes_r(int bridge_id, struct cls_rule **buf,int elem){
  //printf("SGX_fet_ccfes_r\n");

	ecall_fet_ccfes_r(global_eid, bridge_id, buf,elem);
  //printf("end\n");

}

//31 Dependencies for ofproto_get_all_flows
int SGX_fet_ccfe_c(int bridge_id){
  //printf("SGX_fet_ccfe_c\n");

	int count;
	ecall_fet_ccfe_c(global_eid, &count, bridge_id);
  //printf("end\n");

	return count;
}

//31.2 REQUEST
void SGX_fet_ccfe_r(int bridge_id, struct cls_rule **buf,int elem){
  //printf("SGX_fet_ccfe_r\n");

	ecall_fet_ccfe_r(global_eid, bridge_id, buf,elem);
  //printf("end\n");

}

//33 Classifier_lookup
void SGX_cls_lookup(int bridge_id, struct cls_rule **o_cls_rule,int table_id,const struct flow *flow,
		struct flow_wildcards *wc){
      //printf("SGX_cls_lookup\n");

  ecall_cls_lookup(global_eid, bridge_id, o_cls_rule, table_id,flow,wc);
  //printf("end\n");

}

//34. CLS_RULE priority
unsigned int SGX_cls_rule_priority(int bridge_id, struct cls_rule *o_cls_rule){
  //printf("SGX_cls_rule_priority\n");

	unsigned int priority;
	ecall_cls_rule_priority(global_eid,&priority, bridge_id, o_cls_rule);
  //printf("end\n");

	return priority;
}

//Dependencies for destroy
int SGX_desfet_ccfes_c(int bridge_id){
  //printf("SGX_desfet_ccfes_c\n");

	int count;
	ecall_desfet_ccfes_c(global_eid,&count, bridge_id);
  //printf("end\n");

	return count;
}

//2.
void SGX_desfet_ccfes_r(int bridge_id, struct cls_rule **buf,int elem){
  //printf("SGX_desfet_ccfes_r\n");

	ecall_desfet_ccfes_r(global_eid, bridge_id, buf,elem);
  //printf("end\n");

}

//37. CLS_RULE_DEPENDENCIES
unsigned int SGX_cls_rule_format(int bridge_id, const struct cls_rule *o_cls_rule,struct match *megamatch){
  //printf("SGX_cls_rule_format\n");

	unsigned int priority;
	ecall_cls_rule_format(global_eid,&priority, bridge_id, o_cls_rule,megamatch);
  //printf("end\n");

	return priority;
}

//38 miniflow_expand inside the enclave
//This functions copies from the enclave information into the struct flow.
void SGX_miniflow_expand(int bridge_id, struct cls_rule *o_cls_rule,struct flow *flow){
  //printf("SGX_miniflow_expand\n");

	ecall_miniflow_expand(global_eid, bridge_id, o_cls_rule,flow);
  //printf("end\n");

}

//39. Rule_calculate tag this needs to check the result and if not zero
//Calculate the tag_create deterministics
uint32_t SGX_rule_calculate_tag(int bridge_id, struct cls_rule *o_cls_rule,const struct flow *flow,int table_id){
  //printf("SGX_rule_calculate_tag\n");

	uint32_t hash;
	ecall_rule_calculate_tag(global_eid,&hash, bridge_id, o_cls_rule,flow,table_id);
  //printf("end\n");

	return hash;
}

//This Functions are used for the table_dpif in ofproto_dpif {

//1.
void SGX_table_dpif_init(int bridge_id, int n_tables){
  //printf("SGX_table_dpif_init\n");

	ecall_SGX_table_dpif(global_eid, bridge_id, n_tables);
  //printf("end\n");

}

//2.
int SGX_table_update_taggable(int bridge_id, uint8_t table_id){
  //printf("SGX_table_update_taggable\n");

	int todo;
	ecall_table_update_taggable(global_eid,&todo, bridge_id, table_id);
  //printf("end\n");

	return todo;
}

//3.
int SGX_is_sgx_other_table(int bridge_id, int id){
  //printf("SGX_is_sgx_other_table\n");

	int result;
	ecall_is_sgx_other_table(global_eid,&result, bridge_id, id);
  //printf("end\n");

	return result;
}

//4
uint32_t SGX_rule_calculate_tag_s(int bridge_id, int table_id, const struct flow *flow){
  //printf("SGX_rule_calculate_tag_s\n");

	uint32_t hash;
	ecall_rule_calculate_tag_s(global_eid, &hash, bridge_id, table_id, flow);
  //printf("end\n");

	return hash;
}

void sgx_oftable_check_hidden(int bridge_id){
  //printf("sgx_oftable_check_hidden\n");

	ecall_hidden_tables_check(global_eid, bridge_id);
  //printf("end\n");

}

void SGX_oftable_set_name(int bridge_id, int table_id, char *name){
  //printf("SGX_oftable_set_name\n");

	ecall_oftable_set_name(global_eid, bridge_id, table_id, name);
  //printf("end\n");

}

//These functions are going to be used by ofopgroup_complete
uint16_t SGX_minimask_get_vid_mask(int bridge_id, struct cls_rule *o_cls_rule){
  //printf("SGX_minimask_get_vid_mask\n");

	uint16_t result;
	ecall_minimask_get_vid_mask(global_eid,&result, bridge_id, o_cls_rule);
  //printf("end\n");

	return result;
}

uint16_t SGX_miniflow_get_vid(int bridge_id, struct cls_rule *o_cls_rule){
  //printf("SGX_miniflow_get_vid\n");

	uint16_t result;
	 ecall_miniflow_get_vid(global_eid,&result, bridge_id, o_cls_rule);
   //printf("end\n");

	 return result;
}

//These functions are depencencies for ofproto_get_vlan_usage
//1. Count
int SGX_ofproto_get_vlan_usage_c(int bridge_id){
  //printf("SGX_ofproto_get_vlan_usage_c\n");

	int count;
	ecall_ofproto_get_vlan_c(global_eid,&count, bridge_id);
  //printf("end\n");

	return count;
}

//2. Allocate
void SGX_ofproto_get_vlan_usage__r(int bridge_id, uint16_t *buf,int elem){
  //printf("SGX_ofproto_get_vlan_usage__r\n");

	ecall_ofproto_get_vlan_r(global_eid, bridge_id, buf,elem);
  //printf("end\n");
}
