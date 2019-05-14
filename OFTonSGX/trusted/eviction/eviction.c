#include "enclave_t.h"
#include "enclave.h"
#include "eviction.h"
#include "cls-rule.h"
#include "enclave-batch-allocator.h"

// Global data structures
extern struct oftable * SGX_oftables[100];
extern struct SGX_table_dpif * SGX_table_dpif[100];
extern int SGX_n_tables[100];
extern struct sgx_cls_table * SGX_hmap_table[100];

extern const struct batch_allocator evg_ba;

// Vanilla ECALLS

int
ecall_is_eviction_fields_enabled(uint8_t bridge_id, uint8_t table_id){
    return SGX_oftables[bridge_id][table_id].eviction_fields ? true : false;
}

void
ecall_evg_add_rule(uint8_t bridge_id, uint8_t table_id, struct cls_rule * o_cls_rule, uint32_t priority,
  uint32_t rule_evict_prioriy,
  struct heap_node rule_evg_node){
    struct sgx_cls_rule * sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, o_cls_rule);
    struct eviction_group * evg;

    evg = sgx_evg_find(bridge_id, table_id, eviction_group_hash_rule(bridge_id, table_id,
          &sgx_cls_rule->cls_rule), priority);
    sgx_cls_rule->evict_group   = evg;
    sgx_cls_rule->rule_evg_node = rule_evg_node;
    heap_insert_ovs(&evg->rules, &sgx_cls_rule->rule_evg_node, rule_evict_prioriy);
    size_t n_rules      = (size_t) heap_count_ovs(&evg->rules);
    size_t new_priority = (MIN(UINT16_MAX, n_rules) << 16) | random_uint16();
    sgx_evg_group_resize(bridge_id, table_id, o_cls_rule, new_priority, evg);
}

void
ecall_evg_remove_rule(uint8_t bridge_id, uint8_t table_id, struct cls_rule * o_cls_rule){
    struct sgx_cls_rule * sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, o_cls_rule);

    if (!sgx_cls_rule->evict_group) {
        return;
    }
    struct eviction_group * evg = sgx_cls_rule->evict_group;
    sgx_cls_rule->evict_group = NULL;
    heap_remove_ovs(&evg->rules, &sgx_cls_rule->rule_evg_node);
    // Remove heap if its empty, i.e. no eviction groups
    if (heap_is_empty_ovs(&evg->rules)) {
        sgx_evg_destroy(bridge_id, table_id, evg);
        return;
    }
    // Resize heap to reflect changes in eviction group size
    size_t n_rules = (size_t) heap_count_ovs(&evg->rules);
    uint16_t size  = MIN(UINT16_MAX, n_rules);
    size_t p       = (size << 16) | random_uint16();
    sgx_evg_group_resize(bridge_id, table_id, o_cls_rule, p, evg);
    return;
}

void
ecall_choose_rule_to_evict(uint8_t bridge_id, uint8_t table_id, struct cls_rule ** o_cls_rule){
    *o_cls_rule = NULL;
    if (!SGX_oftables[bridge_id][table_id].eviction_fields) {
        return;
    }
    struct eviction_group * evg;
    HEAP_FOR_EACH(evg, size_node, &SGX_oftables[bridge_id][table_id].eviction_groups_by_size){
        struct sgx_cls_rule * sgx_cls_rule;

        HEAP_FOR_EACH(sgx_cls_rule, rule_evg_node, &evg->rules){
            if (sgx_cls_rule->evictable) {
                *o_cls_rule = sgx_cls_rule->o_cls_rule;
                return;
            }
        }
    }
}

// Choose and return a rule to evict from table, without including the rule itself
void
ecall_choose_rule_to_evict_p(uint8_t bridge_id, uint8_t table_id, struct cls_rule ** ut_cr, struct cls_rule * replacer){
    struct sgx_cls_rule * sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, replacer);
    bool was_evictable;

    was_evictable = sgx_cls_rule->evictable;
    // Make rule we are inserting immune to eviction.
    sgx_cls_rule->evictable = false;
    struct cls_rule * tmp = NULL;
    ecall_choose_rule_to_evict(bridge_id, table_id, &tmp);
    // Put back old eviction status
    sgx_cls_rule->evictable = was_evictable;
    //return tmp ? tmp : NULL;

    *ut_cr = tmp ? tmp : NULL;
}

void
ecall_oftable_disable_eviction(uint8_t bridge_id, uint8_t table_id){
    if (SGX_oftables[bridge_id][table_id].eviction_fields) {
        struct eviction_group * evg, * next;
        HMAP_FOR_EACH_SAFE(evg, next, id_node,
          &SGX_oftables[bridge_id][table_id].eviction_groups_by_id)
        {
            sgx_evg_destroy(bridge_id, table_id, evg);
        }

        hmap_destroy(&SGX_oftables[bridge_id][table_id].eviction_groups_by_id);
        heap_destroy_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size);

        free(SGX_oftables[bridge_id][table_id].eviction_fields);
        SGX_oftables[bridge_id][table_id].eviction_fields   = NULL;
        SGX_oftables[bridge_id][table_id].n_eviction_fields = 0;
    }
}

void
ecall_oftable_enable_eviction(uint8_t bridge_id, uint8_t table_id, const struct mf_subfield * fields, size_t n_fields,
  uint32_t random_v,
  bool * no_change){
    if (SGX_oftables[bridge_id][table_id].eviction_fields &&
      n_fields == SGX_oftables[bridge_id][table_id].n_eviction_fields &&
      (!n_fields ||
          !memcmp(fields, SGX_oftables[bridge_id][table_id].eviction_fields,
              n_fields * sizeof *fields)))
    {
        *no_change = true;
        return;
    }

    ecall_oftable_disable_eviction(bridge_id, table_id);


    SGX_oftables[bridge_id][table_id].n_eviction_fields = n_fields;
    SGX_oftables[bridge_id][table_id].eviction_fields         = xmemdup(fields, n_fields * sizeof *fields);
    SGX_oftables[bridge_id][table_id].eviction_group_id_basis = random_v;

    hmap_init(&SGX_oftables[bridge_id][table_id].eviction_groups_by_id, NULL);
    heap_init_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size);
}

size_t
ecall_oftable_enable_eviction_c(uint8_t bridge_id, uint8_t table_id){
    return ecall_oftable_enable_eviction_r(bridge_id, NULL, -1, table_id);
}

size_t
ecall_oftable_enable_eviction_r(uint8_t bridge_id, struct cls_rule ** buf, int elem, uint8_t table_id){
    struct cls_cursor cursor;
    struct sgx_cls_rule * rule;
    size_t p = 0;
    bool count_only = buf == NULL ? true : false;

    cls_cursor_init(&cursor, &SGX_oftables[bridge_id][table_id].cls, NULL);
    CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
        if (rule) {
            if(count_only) {
                p++;
                continue;
            }

            if (p > elem) {
                // overflow: this needs to be handle.
                return p;
            }
            buf[p] = rule->o_cls_rule;
            p++;
        }
    }
    return p;
}

// Optimized ECALLS

size_t
ecall_ofproto_evict(uint8_t bridge_id,
                    int ofproto_n_tables,
                    size_t start_index,
                    uint32_t *rule_hashes,
                    struct cls_rule ** cls_rules,
                    size_t buf_size,
                    size_t *n_evictions)
{

    size_t n = 0, total_nr_evictions = 0;

    size_t size = start_index + buf_size;
    uint32_t rule_prios[size];
    uint32_t group_prios[size];
    uint8_t table_ids[size];

    struct cls_rule *skip_rules[start_index];

    for(size_t table_id = 0; table_id < ofproto_n_tables; table_id++){

        if(!ecall_is_eviction_fields_enabled(bridge_id, table_id)) {
            continue;
        }

        int rules_in_table = ecall_oftable_cls_count(bridge_id, table_id);
        int max_flows = ecall_oftable_mflows(bridge_id, table_id);
        total_nr_evictions += (rules_in_table > max_flows ? rules_in_table - max_flows : 0);
        int count = ecall_oftable_cls_count(bridge_id, table_id);

        while(count > max_flows){
            struct cls_rule *tmp;
            ecall_choose_rule_to_evict(bridge_id, table_id, &tmp);
            if(!tmp) {
                break;
            }

            count--;

            if(n >= start_index) {
                rule_hashes[n - start_index] = ecall_cls_rule_hash(bridge_id, tmp, table_id);
                cls_rules[n - start_index] = tmp;
            } else {
                skip_rules[n] = tmp;
            }

            struct sgx_cls_rule * sgx_cls_rule;
            sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, tmp);
            table_ids[n] = table_id;
            group_prios[n] = sgx_cls_rule->evict_group->size_node.priority;
            rule_prios[n] = sgx_cls_rule->rule_evg_node.priority;
            n++;

            ecall_evg_remove_rule(bridge_id, table_id, tmp);
        }
    }

    for(int i = 0; i < n; ++i) {
        struct sgx_cls_rule * sgx_cls_rule;
        if(i >= start_index) {
            sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, cls_rules[i - start_index]);
            ecall_evg_add_rule(bridge_id, table_ids[i], cls_rules[i  - start_index], group_prios[i], rule_prios[i], sgx_cls_rule->rule_evg_node);
        } else {
            sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, skip_rules[i]);
            ecall_evg_add_rule(bridge_id, table_ids[i], skip_rules[i], group_prios[i], rule_prios[i], sgx_cls_rule->rule_evg_node);
        }
    }

     *n_evictions = total_nr_evictions;
     return n - start_index;
}

bool
ecall_need_to_evict(uint8_t bridge_id, uint8_t table_id) {
    bool eviction_is_enabled = ecall_is_eviction_fields_enabled(bridge_id, table_id);
    bool overflow = ecall_oftable_cls_count(bridge_id, table_id) > ecall_oftable_mflows(bridge_id, table_id);
    return eviction_is_enabled && overflow;
}

size_t ecall_get_cls_rules_and_enable_eviction(uint8_t bridge_id,
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
                                             bool *is_eviction_fields_enabled)
{
    ecall_oftable_enable_eviction(bridge_id, table_id, fields, n_fields, random_v, no_change);
    *is_eviction_fields_enabled = ecall_is_eviction_fields_enabled(bridge_id, table_id);
    if(*no_change || !(*is_eviction_fields_enabled)) {
      return 0;
    }

    return ecall_oftable_get_cls_rules(bridge_id, table_id, 0, -1, buf, buf_size, n_rules);
}


void ecall_eviction_group_add_rules(uint8_t bridge_id,
                                           uint8_t table_id,
                                           size_t n,
                                           struct cls_rule **cls_rules,
                                           struct heap_node *evg_nodes,
                                           uint32_t *rule_priorities,
                                           uint32_t group_priority)
{
    for(size_t i = 0; i < n; ++i) {
        ecall_evg_add_rule(bridge_id,
                           table_id,
                           cls_rules[i],
                           group_priority,
                           rule_priorities[i],
                           evg_nodes[i]);
    }
}

// Helpers

static struct eviction_group *
sgx_evg_find(uint8_t bridge_id, uint8_t table_id, uint32_t evg_id, uint32_t priority){
    struct eviction_group * evg;

    HMAP_FOR_EACH_WITH_HASH(evg, id_node, evg_id, &SGX_oftables[bridge_id][table_id].eviction_groups_by_id){
        return evg;
    }
    #ifdef BATCH_ALLOCATION
    struct bblock *b;
    b = batch_allocator_get_block(&evg_ba);
    evg = (struct eviction_group *) b->ptr;
    evg->block_list_node = &b->list_node;
    #else
    evg = xmalloc(sizeof *evg);
    #endif
    hmap_insert(&SGX_oftables[bridge_id][table_id].eviction_groups_by_id, &evg->id_node, evg_id, NULL, 0);
    heap_insert_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size, &evg->size_node, priority);
    heap_init_ovs(&evg->rules);
    return evg;
}

static uint32_t
eviction_group_hash_rule(uint8_t bridge_id, uint8_t table_id, struct cls_rule * cls_rule){
    const struct mf_subfield * sf;
    struct flow flow;
    uint32_t hash;

    hash = SGX_oftables[bridge_id][table_id].eviction_group_id_basis;
    miniflow_expand(&cls_rule->match.flow, &flow);
    for (sf = SGX_oftables[bridge_id][table_id].eviction_fields;
      sf < &SGX_oftables[bridge_id][table_id].eviction_fields[SGX_oftables[bridge_id][table_id].n_eviction_fields];
      sf++)
    {
        if (mf_are_prereqs_ok(sf->field, &flow)) {
            union mf_value value;
            mf_get_value(sf->field, &flow, &value);
            if (sf->ofs) {
                bitwise_zero(&value, sf->field->n_bytes, 0, sf->ofs);
            }
            if (sf->ofs + sf->n_bits < sf->field->n_bytes * 8) {
                unsigned int start = sf->ofs + sf->n_bits;
                bitwise_zero(&value, sf->field->n_bytes, start,
                  sf->field->n_bytes * 8 - start);
            }
            hash = hash_bytes(&value, sf->field->n_bytes, hash);
        } else  {
            hash = hash_int(hash, 0);
        }
    }
    return hash;
}


/* 18 Eviction_group_destroy: Destroys 'evg' and eviction_group within 'table
 * removes all the rules, if any, from evg. it does not destroy the rules just removes them from the eviction group.
 */
void
sgx_evg_destroy(uint8_t bridge_id, uint8_t table_id, struct eviction_group * evg){
    while (!heap_is_empty_ovs(&evg->rules)) {
        struct sgx_cls_rule * sgx_cls_rule = CONTAINER_OF(heap_pop_ovs(&evg->rules), struct sgx_cls_rule, rule_evg_node);
        sgx_cls_rule->evict_group = NULL;
    }
    hmap_remove(&SGX_oftables[bridge_id][table_id].eviction_groups_by_id, &evg->id_node);
    heap_remove_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size, &evg->size_node);
    heap_destroy_ovs(&evg->rules);

    #ifdef BATCH_ALLOCATION
    batch_allocator_free_block(&evg_ba, evg->block_list_node);
    #else
    free(evg);
    #endif
}


void
choose_rule_to_evict(uint8_t bridge_id, uint8_t table_id, struct sgx_cls_rule ** o_cls_rule, struct sgx_cls_rule **exclude_rules, size_t n_excluded){
    *o_cls_rule = NULL;
    struct sgx_cls_rule *victim = NULL;
    if (!SGX_oftables[bridge_id][table_id].eviction_fields) {
        return;
    }
    struct eviction_group * evg;
    HEAP_FOR_EACH(evg, size_node, &SGX_oftables[bridge_id][table_id].eviction_groups_by_size){
        struct sgx_cls_rule * sgx_cls_rule;

        HEAP_FOR_EACH(sgx_cls_rule, rule_evg_node, &evg->rules){
            if (sgx_cls_rule->evictable) {
                bool is_excluded = false;
                for(size_t i = 0; i < n_excluded; ++i) {
                    if(exclude_rules[i] == sgx_cls_rule) {
                        is_excluded = true;
                    }
                }
                if(is_excluded) {
                    continue;
                }
                if(!victim) {
                    victim = sgx_cls_rule;
                    continue;
                }

                if(victim->rule_evg_node.priority < sgx_cls_rule->rule_evg_node.priority) {
                    victim = sgx_cls_rule;
                }
            }
        }
    }
    *o_cls_rule = victim;
}


void
sgx_evg_group_resize(uint8_t bridge_id, uint8_t table_id, struct cls_rule * o_cls_rule, size_t priority,
  struct eviction_group * evg){
    struct heap * h      = &SGX_oftables[bridge_id][table_id].eviction_groups_by_size;
    struct heap_node * n = &evg->size_node;
    heap_change_ovs(h, n, priority);
}
