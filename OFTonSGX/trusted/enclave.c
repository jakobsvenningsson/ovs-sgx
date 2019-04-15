#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include "enclave.h"
#include "enclave_t.h" /* print_string */
#include <stdbool.h>
#include "classifier.h"
#include "ofproto-provider.h"

#include "call-table.h"
#include "hotcall.h"
#include "openflow-common.h"
#include <sgx_spinlock.h>

#ifdef TIMEOUT
#define INIT_TIMER_VALUE 9999999;
static unsigned int timeout_counter = INIT_TIMER_VALUE;
#endif


// Global data structures
struct oftable * SGX_oftables[100];
struct SGX_table_dpif * SGX_table_dpif[100];
int SGX_n_tables[100];
struct sgx_cls_table * SGX_hmap_table[100];

// 1. Initialization of hmap table
void
sgx_table_cls_init(int bridge_id){
    SGX_hmap_table[bridge_id] = xmalloc(sizeof(struct sgx_cls_table));
    hmap_init(&SGX_hmap_table[bridge_id]->cls_rules);
}

/*2. Node_search: This method is in charge of the searching of a cls_rule based on
 * hash computed from the pointer holding the cls_rule in untrusted memory
 */
struct
sgx_cls_rule *
node_search(int bridge_id, const struct cls_rule * out){
    struct sgx_cls_rule * rule;

    HMAP_FOR_EACH_WITH_HASH(rule, hmap_node, hash_pointer(out, 0), &SGX_hmap_table[bridge_id]->cls_rules){
        return rule;
    }
    return NULL;
}

// 4. Node_insert: This function insert a new sgx_cls_rule to the hmap table.
struct
sgx_cls_rule *
node_insert(int bridge_id, uint32_t hash){
    struct sgx_cls_rule * new = xmalloc(sizeof(struct sgx_cls_rule));

    memset(new, 0, sizeof(struct sgx_cls_rule));
    new->hmap_node.hash = hash;
    hmap_insert(&SGX_hmap_table[bridge_id]->cls_rules, &new->hmap_node, new->hmap_node.hash);
    return new;
}

// 5. node_delete: deletes sgx_rule from the hmap table and free the sgx_cls_rule
void
node_delete(int bridge_id, struct cls_rule * out){
    struct sgx_cls_rule * rule;

    rule = node_search(bridge_id, out);
    hmap_remove(&SGX_hmap_table[bridge_id]->cls_rules, &rule->hmap_node);
    free(rule);
}

/* Open vSwitch Trusted function definitions */
static void
oftable_init(struct oftable * table){
    memset(table, 0, sizeof *table);
    classifier_init(&table->cls);
    table->max_flows = UINT_MAX;
}

// 1. Creation and Initialization
void
sgx_table_dpif_init(int bridge_id, int n_tables){
    // I need to create the struct SGX_table_dpif in memory
    int i;

    SGX_table_dpif[bridge_id] = xmalloc(n_tables * sizeof(struct SGX_table_dpif));
    for (i = 0; i < n_tables; i++) {
        SGX_table_dpif[bridge_id][i].catchall_table = NULL;
        SGX_table_dpif[bridge_id][i].other_table    = NULL;
    }
}

void
ecall_ofproto_init_tables(int bridge_id, int n_tables){
    struct oftable * table;

    SGX_n_tables[bridge_id] = n_tables;
    SGX_oftables[bridge_id] = xmalloc(n_tables * sizeof(struct oftable));
    OFPROTO_FOR_EACH_TABLE(table, SGX_oftables[bridge_id]){
        oftable_init(table);
    }
    sgx_table_cls_init(bridge_id);
    sgx_table_dpif_init(bridge_id, n_tables);
}

void
ecall_readonly_set(int bridge_id, int table_id){
    SGX_oftables[bridge_id][table_id].flags = OFTABLE_HIDDEN | OFTABLE_READONLY;
}

int
ecall_istable_readonly(int bridge_id, uint8_t table_id){
    return SGX_oftables[bridge_id][table_id].flags & OFTABLE_READONLY;
}

void
ecall_cls_rule_init(int bridge_id, struct cls_rule * o_cls_rule, const struct match * match, unsigned int priority){
    struct sgx_cls_rule * sgx_cls_rule = node_insert(bridge_id, hash_pointer(o_cls_rule, 0));
    sgx_cls_rule->o_cls_rule = o_cls_rule;
    cls_rule_init(&sgx_cls_rule->cls_rule, match, priority);
    sgx_cls_rule->evictable = true;
}

void
ecall_cls_rule_init_i(int bridge_id, struct cls_rule * cls_rule, const struct match * match, unsigned int priority){
    cls_rule_init(cls_rule, match, priority);
}

// 5. Classifier_rule_overlaps
int
ecall_cr_rule_overlaps(int bridge_id, int table_id, struct cls_rule * out){
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, out);
    struct cls_rule * cls_rule         = &sgx_cls_rule->cls_rule;
    const struct classifier * cls      = &SGX_oftables[bridge_id][table_id].cls;
    const struct cls_rule * target     = cls_rule;

    return classifier_rule_overlaps(cls, target);
}

// 6. cls_rule_destroy
void
ecall_cls_rule_destroy(int bridge_id, struct cls_rule * out){
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, out);
    struct cls_rule * cls_rule         = &sgx_cls_rule->cls_rule;

    cls_rule_destroy(cls_rule);
    node_delete(bridge_id, sgx_cls_rule->o_cls_rule);
    // free(sgx_cls_rule);
    sgx_cls_rule = NULL;
}

// 7. cls_rule_hash
uint32_t
ecall_cls_rule_hash(int bridge_id, const struct cls_rule * o_cls_rule, uint32_t basis){
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, o_cls_rule);
    struct cls_rule * cls_rule         = &sgx_cls_rule->cls_rule;

    return cls_rule_hash(cls_rule, basis);
}

// 8. cls_rule_equal
int
ecall_cls_rule_equal(int bridge_id, const struct cls_rule * out_a, const struct cls_rule * out_b){
    struct sgx_cls_rule * sgx_cls_rule_a = node_search(bridge_id, out_a);
    struct sgx_cls_rule * sgx_cls_rule_b = node_search(bridge_id, out_b);
    const struct cls_rule * a = &sgx_cls_rule_a->cls_rule;
    const struct cls_rule * b = &sgx_cls_rule_b->cls_rule;

    return cls_rule_equal(a, b);
}

// 9. classifier_replace
void
ecall_classifier_replace(int bridge_id, int table_id, struct cls_rule * o_cls_rule, struct cls_rule ** cls_rule_rtrn){
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, o_cls_rule);
    struct cls_rule * cls_rule         = classifier_replace(&SGX_oftables[bridge_id][table_id].cls,
      &sgx_cls_rule->cls_rule);

    if (cls_rule) {
        struct sgx_cls_rule * sgx_cls_rule_r = CONTAINER_OF(cls_rule, struct sgx_cls_rule, cls_rule);
        *cls_rule_rtrn = sgx_cls_rule_r->o_cls_rule;
    } else  {
        *cls_rule_rtrn = NULL;
    }
}

// 10. rule_get_flags
enum
oftable_flags
ecall_rule_get_flags(int bridge_id, int table_id){
    return SGX_oftables[bridge_id][table_id].flags;
}

// 11. Classifier_count
int
ecall_cls_count(int bridge_id, int table_id){
    return classifier_count(&SGX_oftables[bridge_id][table_id].cls);
}

// 12. Table has eviction_fields enable?
int
ecall_eviction_fields_enable(int bridge_id, int table_id){
    return SGX_oftables[bridge_id][table_id].eviction_fields ? true : false;
}

// 13 eviction group find
static struct eviction_group *
ecall_evg_find(int bridge_id, int table_id, uint32_t evg_id, uint32_t priority){
    struct eviction_group * evg;

    HMAP_FOR_EACH_WITH_HASH(evg, id_node, evg_id, &SGX_oftables[bridge_id][table_id].eviction_groups_by_id){
        return evg;
    }
    evg = xmalloc(sizeof *evg);
    hmap_insert(&SGX_oftables[bridge_id][table_id].eviction_groups_by_id, &evg->id_node, evg_id);
    heap_insert_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size, &evg->size_node, priority);
    heap_init_ovs(&evg->rules);
    return evg;
}

static uint32_t
eviction_group_hash_rule(int bridge_id, int table_id, struct cls_rule * cls_rule){
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

// 15. eviction_grpu_add and eviction group find
size_t
ecall_evg_add_rule(int bridge_id, int table_id, struct cls_rule * o_cls_rule, uint32_t priority,
  uint32_t rule_evict_prioriy,
  struct heap_node rule_evg_node){
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, o_cls_rule);
    struct eviction_group * evg;

    evg = ecall_evg_find(bridge_id, table_id, eviction_group_hash_rule(bridge_id, table_id,
          &sgx_cls_rule->cls_rule), priority);
    sgx_cls_rule->evict_group   = evg;
    sgx_cls_rule->rule_evg_node = rule_evg_node;
    heap_insert_ovs(&evg->rules, &sgx_cls_rule->rule_evg_node, rule_evict_prioriy);
    size_t n_rules      = (size_t) heap_count_ovs(&evg->rules);
    size_t new_priority = (MIN(UINT16_MAX, n_rules) << 16) | random_uint16();
    ecall_evg_group_resize(bridge_id, table_id, o_cls_rule, new_priority, evg);
    return 0;
}

// 16
void
ecall_evg_group_resize(int bridge_id, int table_id, struct cls_rule * o_cls_rule, size_t priority,
  struct eviction_group * evg){
    struct heap * h      = &SGX_oftables[bridge_id][table_id].eviction_groups_by_size;
    struct heap_node * n = &evg->size_node;

    heap_change_ovs(h, n, priority);
}

/* 18 Eviction_group_destroy: Destroys 'evg' and eviction_group within 'table
 * removes all the rules, if any, from evg. it does not destroy the rules just removes them from the eviction group.
 */
void
ecall_evg_destroy(int bridge_id, int table_id, struct eviction_group * evg){
    while (!heap_is_empty_ovs(&evg->rules)) {
        struct sgx_cls_rule * sgx_cls_rule = CONTAINER_OF(heap_pop_ovs(&evg->rules), struct sgx_cls_rule, rule_evg_node);
        sgx_cls_rule->evict_group = NULL;
    }
    hmap_remove(&SGX_oftables[bridge_id][table_id].eviction_groups_by_id, &evg->id_node);
    heap_remove_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size, &evg->size_node);
    heap_destroy_ovs(&evg->rules);
    free(evg);
}

// 17. Eviction_grouP_remove_rule: Delete the eviction group of a rule if it is not NULL
int
ecall_evg_remove_rule(int bridge_id, int table_id, struct cls_rule * o_cls_rule){
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, o_cls_rule);

    if (!sgx_cls_rule->evict_group) {
        return 0;
    }
    struct eviction_group * evg = sgx_cls_rule->evict_group;
    sgx_cls_rule->evict_group = NULL;
    heap_remove_ovs(&evg->rules, &sgx_cls_rule->rule_evg_node);
    // Remove heap if its empty, i.e. no eviction groups
    if (heap_is_empty_ovs(&evg->rules)) {
        ecall_evg_destroy(bridge_id, table_id, evg);
        return 0;
    }
    // Resize heap to reflect changes in eviction group size
    size_t n_rules = (size_t) heap_count_ovs(&evg->rules);
    uint16_t size  = MIN(UINT16_MAX, n_rules);
    size_t p       = (size << 16) | random_uint16();
    ecall_evg_group_resize(bridge_id, table_id, o_cls_rule, p, evg);
    return 0;
}

// 18. Classifier remove: removes a cls_rule from the classifier
void
ecall_cls_remove(int bridge_id, int table_id, struct cls_rule * o_cls_rule){
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, o_cls_rule);

    classifier_remove(&SGX_oftables[bridge_id][table_id].cls, &sgx_cls_rule->cls_rule);
}

// ////////////////////////////////////////////////////////////////////


// 19 choose and return a rule to evict from table
void
ecall_choose_rule_to_evict(int bridge_id, int table_id, struct cls_rule ** o_cls_rule){
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

void
ecall_choose_rule_to_evict_2(int bridge_id, int table_id, struct cls_rule ** o_cls_rule, struct cls_rule **exclude, size_t n_exclude){
    *o_cls_rule = NULL;
    if (!SGX_oftables[bridge_id][table_id].eviction_fields) {
        return;
    }
    struct eviction_group * evg;
    HEAP_FOR_EACH(evg, size_node, &SGX_oftables[bridge_id][table_id].eviction_groups_by_size){
        struct sgx_cls_rule * sgx_cls_rule;

        HEAP_FOR_EACH(sgx_cls_rule, rule_evg_node, &evg->rules){
            if (sgx_cls_rule->evictable) {
                bool ex = false;
                for(int i = 0; i < n_exclude; ++i) {
                    if(exclude[i] == sgx_cls_rule->o_cls_rule) {
                        ex = true;
                    }
                }

                if(ex) {
                    continue;
                }


                *o_cls_rule = sgx_cls_rule->o_cls_rule;
                return;
            }
        }
    }
}

// 19 choose and return a rule to evict from table
void
choose_rule_to_evict(int bridge_id, int table_id, struct sgx_cls_rule ** o_cls_rule, struct sgx_cls_rule **exclude_rules, size_t n_excluded){
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


// 20. choose and return a rule to evict from table, without including the rule itself
struct cls_rule *
ecall_choose_rule_to_evict_p(int bridge_id, int table_id, struct cls_rule ** o_cls_rule, struct cls_rule * replacer){
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, replacer);
    bool was_evictable;

    was_evictable = sgx_cls_rule->evictable;
    // Make rule we are inserting immune to eviction.
    sgx_cls_rule->evictable = false;
    struct cls_rule * tmp = NULL;
    ecall_choose_rule_to_evict(bridge_id, table_id, &tmp);
    // Put back old eviction status
    sgx_cls_rule->evictable = was_evictable;
    return tmp ? tmp : NULL;
}

// 21 returns the table max_flows
unsigned int
ecall_table_mflows(int bridge_id, int table_id){
    return SGX_oftables[bridge_id][table_id].max_flows;
}

// 21 sets the table max_flows
void
ecall_table_mflows_set(int bridge_id, int table_id, unsigned int value){
    SGX_oftables[bridge_id][table_id].max_flows = value;
}

// 22. Minimatch_expand inside the enclave
void
ecall_minimatch_expand(int bridge_id, struct cls_rule * o_cls_rule, struct match * dst){
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, o_cls_rule);

    minimatch_expand(&sgx_cls_rule->cls_rule.match, dst);
}

// 23. cls_rule priority
unsigned int
ecall_cr_priority(int bridge_id, const struct cls_rule * o_cls_rule){
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, o_cls_rule);

    return sgx_cls_rule->cls_rule.priority;
}

/* Finds and returns a rule in 'cls' with priority 'priority' and exactly the
 * same matching criteria as 'target'.  Returns a null pointer if 'cls' doesn't
 * contain an exact match. */
void
ecall_cls_find_match_exactly(int bridge_id, int table_id, const struct match * target, unsigned int priority,
  struct cls_rule ** o_cls_rule){
    struct cls_rule * cls_rule = classifier_find_match_exactly(&SGX_oftables[bridge_id][table_id].cls, target, priority);

    *o_cls_rule = cls_rule ?
      CONTAINER_OF(cls_rule, struct sgx_cls_rule, cls_rule)->o_cls_rule :
      NULL;
}

// 25. Next Visible table
static struct oftable *
next_visible_table(int bridge_id, int ofproto_n_tables, uint8_t table_id){
    struct oftable * table;

    for (table = &SGX_oftables[bridge_id][table_id];
      table < &SGX_oftables[bridge_id][ofproto_n_tables];
      table++)
    {
        if (!(table->flags & OFTABLE_HIDDEN)) {
            return table;
        }
    }
    return NULL;
}

// 26. First _matching_table
static struct oftable *
first_matching_table(int bridge_id, int ofproto_n_tables, uint8_t table_id){
    if (table_id == 0xff) {
        return next_visible_table(bridge_id, ofproto_n_tables, 0);
    } else if (table_id < ofproto_n_tables) {
        return &SGX_oftables[bridge_id][table_id];
    } else {
        return NULL;
    }
}

// 27. next matching table
static struct oftable *
next_matching_table(int bridge_id, int ofproto_n_tables, const struct oftable * table, uint8_t table_id){
    return (table_id == 0xff ?
      next_visible_table(bridge_id, ofproto_n_tables, (table - SGX_oftables[bridge_id]) + 1) :
      NULL);
}

// ///////Special Functions with loops //////
int
ecall_femt_ccfe_c(int bridge_id, int ofproto_n_tables, uint8_t table_id, const struct match * match){
    struct oftable * table;
    struct cls_rule cr;

    ecall_cls_rule_init_i(bridge_id, &cr, match, 0);
    int count = 0;
    FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, ofproto_n_tables){
        struct cls_cursor cursor;
        struct sgx_cls_rule * rule;

        cls_cursor_init(&cursor, &table->cls, &cr);
        CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
            count++;
        }
    }
    cls_rule_destroy(&cr);
    return count;
}

void
ecall_femt_ccfe_r(int bridge_id, int ofproto_n_tables, struct cls_rule ** buf, int elem, uint8_t table_id,
  const struct match * match){
    struct oftable * table;
    struct cls_rule cr;

    ecall_cls_rule_init_i(bridge_id, &cr, match, 0);
    int p = 0;
    FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, ofproto_n_tables){
        struct cls_cursor cursor;
        struct sgx_cls_rule * rule;

        cls_cursor_init(&cursor, &table->cls, &cr);
        CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
            if (p > elem) {
                return;
            }
            buf[p] = rule->o_cls_rule;
            p++;
        }
    }
    cls_rule_destroy(&cr);
}

// 39. Dependecy in ofproto_collect_ofmonitor_refresh_rules.
int
ecall_collect_ofmonitor_util_c(int bridge_id, int ofproto_n_tables, int table_id, const struct minimatch * match){
    struct cls_rule target;
    const struct oftable * table;
    int count = 0;

    cls_rule_init_from_minimatch(&target, match, 0);
    FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, ofproto_n_tables){
        struct cls_cursor cursor;
        struct sgx_cls_rule * rule;

        cls_cursor_init(&cursor, &table->cls, &target);
        CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
            count++;
        }
    }
    minimatch_destroy(&target.match);
    return count;
}

void
ecall_collect_ofmonitor_util_r(int bridge_id, int ofproto_n_tables, struct cls_rule ** buf, int elem, int table_id,
  const struct minimatch * match){
    struct cls_rule target;
    const struct oftable * table;
    int p = 0;

    cls_rule_init_from_minimatch(&target, match, 0);
    FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, ofproto_n_tables){
        struct cls_cursor cursor;
        struct sgx_cls_rule * rule;

        cls_cursor_init(&cursor, &table->cls, &target);
        CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
            if (p > elem) {
                // overflow: this needs to be handle.
                return;
            }
            buf[p] = rule->o_cls_rule;
            p++;
        }
    }
    minimatch_destroy(&target.match); // to diassociate the target because if its a temp cls_rule
}

// Dependencies for ofproto_flush___
// 50.1 Need to know the size of the buffer to allocate
int
ecall_fet_ccfes_c(int bridge_id){
    int i;
    int count = 0;

    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        struct sgx_cls_rule * rule, * next_rule;
        struct cls_cursor cursor;
        if (SGX_oftables[bridge_id][i].flags & OFTABLE_HIDDEN) {
            continue;
        }
        cls_cursor_init(&cursor, &SGX_oftables[bridge_id][i].cls, NULL);
        CLS_CURSOR_FOR_EACH_SAFE(rule, next_rule, cls_rule, &cursor){
            count++;
        }
    }
    return count;
}

// 50.2
void
ecall_fet_ccfes_r(int bridge_id, struct cls_rule ** buf, int elem){
    int p = 0;
    int i;

    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        struct sgx_cls_rule * rule, * next_rule;
        struct cls_cursor cursor;
        if (SGX_oftables[bridge_id][i].flags & OFTABLE_HIDDEN) {
            continue;
        }
        cls_cursor_init(&cursor, &SGX_oftables[bridge_id][i].cls, NULL);
        CLS_CURSOR_FOR_EACH_SAFE(rule, next_rule, cls_rule, &cursor){
            if (p > elem) {
                // overflow: this needs to be handle.
                return;
            }
            buf[p] = rule->o_cls_rule;
            p++;
        }
    }
}

// Dependencies for ofproto_get_all_flows
// 51. COUNT
int
ecall_fet_ccfe_c(int bridge_id){
    int i;
    int count = 0;

    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        struct sgx_cls_rule * rule;
        struct cls_cursor cursor;
        cls_cursor_init(&cursor, &SGX_oftables[bridge_id][i].cls, NULL);
        CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
            count++;
        }
    }
    return count;
}

// 51.2 REQUEST
void
ecall_fet_ccfe_r(int bridge_id, struct cls_rule ** buf, int elem){
    int p = 0;
    int i;

    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        struct sgx_cls_rule * rule;
        struct cls_cursor cursor;
        cls_cursor_init(&cursor, &SGX_oftables[bridge_id][i].cls, NULL);
        CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
            if (p > elem) {
                // overflow: this needs to be handle.
                return;
            }
            buf[p] = rule->o_cls_rule;
            p++;
        }
    }
}

int
ecall_femt_c(int bridge_id, int ofproto_n_tables, uint8_t table_id, const struct match * match, unsigned int priority){
    struct oftable * table;
    struct cls_rule cr;

    ecall_cls_rule_init_i(bridge_id, &cr, match, priority);
    int count = 0;
    FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, ofproto_n_tables){
        if (classifier_find_rule_exactly(&table->cls, &cr)) {
            count++;
        }
    }
    cls_rule_destroy(&cr);
    return count;
}

void
ecall_femt_r(int bridge_id, int ofproto_n_tables, struct cls_rule ** buf, int elem, uint8_t table_id,
  const struct match * match,
  unsigned int priority){
    struct oftable * table;
    struct cls_rule cr;

    ecall_cls_rule_init_i(bridge_id, &cr, match, priority);
    int p = 0;
    FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, ofproto_n_tables){
        struct cls_rule * cls_rule = classifier_find_rule_exactly(&table->cls, &cr);

        if (cls_rule) {
            if (p > elem) {
                // overflow: this needs to be handle.
                return;
            }
            struct sgx_cls_rule * sgx_cls_rule = CONTAINER_OF(cls_rule, struct sgx_cls_rule, cls_rule);
            buf[p] = sgx_cls_rule->o_cls_rule;
            p++;
        }
    }
    cls_rule_destroy(&cr);
}

int
ecall_ccfe_c(int bridge_id, int table_id){
    int count = 0;
    struct cls_cursor cursor;
    struct sgx_cls_rule * rule;

    cls_cursor_init(&cursor, &SGX_oftables[bridge_id][table_id].cls, NULL);
    CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
        count++;
    }
    return count;
}

void
ecall_ccfe_r(int bridge_id, struct cls_rule ** buf, int elem, int table_id){
    struct cls_cursor cursor;
    struct sgx_cls_rule * rule;
    int p = 0;

    cls_cursor_init(&cursor, &SGX_oftables[bridge_id][table_id].cls, NULL);
    CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
        if (rule) {
            if (p > elem) {
                // overflow: this needs to be handle.
                return;
            }
            // struct sgx_cls_rule * sgx_cls_rule=CONTAINER_OF(rule,struct sgx_cls_rule,cls_rule);
            // buf[p]=sgx_cls_rule->o_cls_rule;
            buf[p] = rule->o_cls_rule;
            p++;
        }
    }
}

// 32  oftable_set_name
void
ecall_oftable_set_name(int bridge_id, int table_id, char * name){
    if (name && name[bridge_id]) {
        int len = strnlen(name, OFP_MAX_TABLE_NAME_LEN);
        if (!SGX_oftables[bridge_id][table_id].name || strncmp(name, SGX_oftables[bridge_id][table_id].name, len)) {
            free(SGX_oftables[bridge_id][table_id].name);
            SGX_oftables[bridge_id][table_id].name = xmemdup0(name, len);
        }
    } else {
        free(SGX_oftables[bridge_id][table_id].name);
        SGX_oftables[bridge_id][table_id].name = NULL;
    }
}

// 33. oftable_disable_eviction
void
ecall_oftable_disable_eviction(int bridge_id, int table_id){
    if (SGX_oftables[bridge_id][table_id].eviction_fields) {
        struct eviction_group * evg, * next;
        HMAP_FOR_EACH_SAFE(evg, next, id_node,
          &SGX_oftables[bridge_id][table_id].eviction_groups_by_id)
        {
            ecall_evg_destroy(bridge_id, table_id, evg);
        }


        hmap_destroy(&SGX_oftables[bridge_id][table_id].eviction_groups_by_id);
        heap_destroy_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size);

        //printf("heap_destroy_ovs size %d\n", heap_count_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size));


        free(SGX_oftables[bridge_id][table_id].eviction_fields);
        SGX_oftables[bridge_id][table_id].eviction_fields   = NULL;
        SGX_oftables[bridge_id][table_id].n_eviction_fields = 0;
    } else {
        // printf("NO disable eviction!!!!\n");
    }
}

void
ecall_oftable_enable_eviction(int bridge_id, int table_id, const struct mf_subfield * fields, size_t n_fields,
  uint32_t random_v,
  bool * no_change){
    if (SGX_oftables[bridge_id][table_id].eviction_fields &&
      n_fields == SGX_oftables[bridge_id][table_id].n_eviction_fields &&
      (!n_fields ||
          !memcmp(fields, SGX_oftables[bridge_id][table_id].eviction_fields,
              n_fields * sizeof *fields)))
    {
        *no_change = true;
        //printf("NOOOOO CHANGE ENCLAVE!!------------------------------------------------!!!");
        return;
    }

    ecall_oftable_disable_eviction(bridge_id, table_id);


    SGX_oftables[bridge_id][table_id].n_eviction_fields = n_fields;
    // CAUSES CRASH SGX_oftables[bridge_id][table_id].eviction_fields =
    SGX_oftables[bridge_id][table_id].eviction_fields         = xmemdup(fields, n_fields * sizeof *fields);
    SGX_oftables[bridge_id][table_id].eviction_group_id_basis = random_v;

    hmap_init(&SGX_oftables[bridge_id][table_id].eviction_groups_by_id);
    heap_init_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size);

    //printf("enable eviction size %d\n", heap_count_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size));

    /*
     * table->eviction_group_id_basis = random_uint32();
     * hmap_init(&table->eviction_groups_by_id);
     * heap_init(&table->eviction_groups_by_size);
     */
}

void
ecall_oftable_destroy(int bridge_id, int table_id){
    ovs_assert(classifier_is_empty(&SGX_oftables[bridge_id][table_id].cls));
    ecall_oftable_disable_eviction(bridge_id, table_id);
    classifier_destroy(&SGX_oftables[bridge_id][table_id].cls);
    free(SGX_oftables[bridge_id][table_id].name);
}

void
ecall_ofproto_destroy(int bridge_id){
    int i;

    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        ecall_oftable_destroy(bridge_id, i);
    }
    free(SGX_oftables[bridge_id]);
}

// 37 count total number of rules
unsigned int
ecall_total_rules(int bridge_id){
    int i;
    unsigned int n_rules;

    n_rules = 0;
    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        n_rules += classifier_count(&SGX_oftables[bridge_id][i].cls);
    }
    return n_rules;
}

// 38 Table name
void
ecall_table_name(int bridge_id, int table_id, char * buf, size_t len){
    // I set the value manually to 100;
    if (SGX_oftables[bridge_id][table_id].name) {
        if (len > strlen(SGX_oftables[bridge_id][table_id].name)) {
            memcpy(buf, SGX_oftables[bridge_id][table_id].name, strlen(SGX_oftables[bridge_id][table_id].name) + 1);
        } else  {
            memcpy(buf, SGX_oftables[bridge_id][table_id].name, len);
        }
    } else  {
        memset(buf, 0, len);
    }
}

// another


int
ecall_cls_rule_is_loose_match(int bridge_id, struct cls_rule * o_cls_rule, const struct minimatch * criteria){
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, o_cls_rule);

    if (cls_rule_is_loose_match(&sgx_cls_rule->cls_rule, criteria)) {
        return 100;
    }

    return 0;
}

// ////////////////////////////////////////////////

// /FUNCTION FOR OFPROTO_DPIF.c

// 1. Classifier_lookup
void
ecall_cls_lookup(int bridge_id, struct cls_rule ** o_cls_rule, int table_id, const struct flow * flow,
  struct flow_wildcards * wc){
    struct cls_rule * cls_rule;
    cls_rule = classifier_lookup(&SGX_oftables[bridge_id][table_id].cls, flow, wc);
    if (cls_rule) {
        // Need to retrieve the sgx_cls_rule and return the pointer
        // to untrusted memory
        struct sgx_cls_rule * sgx_cls_rule = CONTAINER_OF(cls_rule, struct sgx_cls_rule, cls_rule);
        *o_cls_rule = sgx_cls_rule->o_cls_rule;
    } else  {
        *o_cls_rule = NULL;
    }
}

// 2. cls_rule priority
unsigned int
ecall_cls_rule_priority(int bridge_id, struct cls_rule * o_cls_rule){
    // we need to find this rule using this pointer
    struct sgx_cls_rule * sgx_cls_rule;

    sgx_cls_rule = node_search(bridge_id, o_cls_rule);
    return sgx_cls_rule->cls_rule.priority;
}

// Dependencies for destruct
int
ecall_desfet_ccfes_c(int bridge_id){
    int i;
    int count = 0;

    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        struct sgx_cls_rule * rule, * next_rule;
        struct cls_cursor cursor;

        cls_cursor_init(&cursor, &SGX_oftables[bridge_id][i].cls, NULL);
        CLS_CURSOR_FOR_EACH_SAFE(rule, next_rule, cls_rule, &cursor){
            count++;
        }
    }
    return count;
}

// 50.2
void
ecall_desfet_ccfes_r(int bridge_id, struct cls_rule ** buf, int elem){
    int p = 0;
    int i;

    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        struct sgx_cls_rule * rule, * next_rule;
        struct cls_cursor cursor;
        cls_cursor_init(&cursor, &SGX_oftables[bridge_id][i].cls, NULL);
        CLS_CURSOR_FOR_EACH_SAFE(rule, next_rule, cls_rule, &cursor){
            if (p > elem) {
                // overflow: this needs to be handle.
                return;
            }
            // struct sgx_cls_rule * sgx_cls_rule=CONTAINER_OF(rule,struct sgx_cls_rule,cls_rule);
            // buf[p]=sgx_cls_rule->o_cls_rule;
            buf[p] = rule->o_cls_rule;
            p++;
        }
    }
}

// cls_rule_format : We are performing just one part of the entired function.


unsigned int
ecall_cls_rule_format(int bridge_id, const struct cls_rule * o_cls_rule, struct match * megamatch){
    struct sgx_cls_rule * sgx_cls_rule;

    sgx_cls_rule = node_search(bridge_id, o_cls_rule);
    minimatch_expand(&sgx_cls_rule->cls_rule.match, megamatch);
    return sgx_cls_rule->cls_rule.priority;
}

uint32_t
ecall_rule_calculate_tag(int bridge_id, struct cls_rule * o_cls_rule, const struct flow * flow, int table_id){
    // Retrieve the cls_rule
    struct sgx_cls_rule * sgx_cls_rule;

    sgx_cls_rule = node_search(bridge_id, o_cls_rule);
    if (minimask_is_catchall(&sgx_cls_rule->cls_rule.match.mask)) {
        return 0;
    } else {
        uint32_t secret = SGX_table_dpif[bridge_id][table_id].basis;
        uint32_t hash   = flow_hash_in_minimask(flow, &sgx_cls_rule->cls_rule.match.mask, secret);
        return hash;
    }
}

void
ecall_miniflow_expand(int bridge_id, struct cls_rule * o_cls_rule, struct flow * flow){
    // From untrusted the Pointer the sgx_cls_rule is retrieved.
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, o_cls_rule);

    // Need to call miniflow_expand to copy the information in the just passed flow struct.
    miniflow_expand(&sgx_cls_rule->cls_rule.match.flow, flow);
}

// These functions are for the ofproto_dpif tables.



// 2. void ecall_table_update_taggable
int
ecall_table_update_taggable(int bridge_id, uint8_t table_id){
    // SGX_table_dpif[table_id]
    struct cls_table * catchall, * other;
    struct cls_table * t;

    catchall = other = NULL;
    switch (hmap_count(&SGX_oftables[bridge_id][table_id].cls.tables)) {
        case 0:
            break;
        case 1:
        case 2:
            HMAP_FOR_EACH(t, hmap_node, &SGX_oftables[bridge_id][table_id].cls.tables){
                if (cls_table_is_catchall(t)) {
                    catchall = t;
                } else if (!other)  {
                    other = t;
                } else  {
                    other = NULL;
                }
            }
            break;
        default:
            break;
    }

    if (SGX_table_dpif[bridge_id][table_id].catchall_table != catchall ||
      SGX_table_dpif[bridge_id][table_id].other_table != other) {
        SGX_table_dpif[bridge_id][table_id].catchall_table = catchall;
        SGX_table_dpif[bridge_id][table_id].other_table    = other;
        return 4; // REV_FLOW_TABLE
    }

    return 0; // No need to do anything to backer.
} /* ecall_table_update_taggable */

// 3. is table_dpif_other_table set?

int
ecall_is_sgx_other_table(int bridge_id, int id){
    if (SGX_table_dpif[bridge_id][id].other_table) {
        return 100;
    }
    return 0;
}

// 4. This is a rule_calculate_tag dependencies for tag_the flow.
// using table->other_table.
uint32_t
ecall_rule_calculate_tag_s(int bridge_id, int id, const struct flow * flow){
    // bridge_id = bridge_id - 1;
    // MINIMASK BREAKS TEST 244
    if (minimask_is_catchall(&SGX_table_dpif[bridge_id][id].other_table->mask)) {
        return 0;
    } else {
        uint32_t hash =
          flow_hash_in_minimask(flow, &SGX_table_dpif[bridge_id][id].other_table->mask,
          SGX_table_dpif[bridge_id][id].basis);
        return hash;
    }
}

void
ecall_hidden_tables_check(int bridge_id){
    int i;

    for (i = 0; i + 1 < SGX_n_tables[bridge_id]; i++) {
        enum oftable_flags flags      = SGX_oftables[bridge_id][i].flags;
        enum oftable_flags next_flags = SGX_oftables[bridge_id][i + 1].flags;
        ovs_assert(!(flags & OFTABLE_HIDDEN) || (next_flags & OFTABLE_HIDDEN));
    }
}

// DEBUG
/*void
ecall_table_dpif_init(int bridge_id){
    // Initialization of the table_dpif
    int i;

    for (i = 0; i < N_TABLES; i++) {
        struct SGX_table_dpif * table = &SGX_table_dpif[bridge_id][i];
        table->catchall_table = NULL;
        table->other_table    = NULL;
        table->basis = random_uint32();
    }
}*/

// This functions are for ofopgroup_complete()

uint16_t
ecall_minimask_get_vid_mask(int bridge_id, struct cls_rule * o_cls_rule){
    // Retrieve the cls_rule
    struct sgx_cls_rule * sgx_cls_rule;

    sgx_cls_rule = node_search(bridge_id, o_cls_rule);

    return minimask_get_vid_mask(&sgx_cls_rule->cls_rule.match.mask);
}

uint16_t
ecall_miniflow_get_vid(int bridge_id, struct cls_rule * o_cls_rule){
    struct sgx_cls_rule * sgx_cls_rule;

    sgx_cls_rule = node_search(bridge_id, o_cls_rule);
    return miniflow_get_vid(&sgx_cls_rule->cls_rule.match.flow);
}

// //Dependencies for ofproto_get_vlan
int
ecall_ofproto_get_vlan_c(int bridge_id){
    struct oftable * oftable;
    int i;
    int count = 0;

    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        const struct cls_table * table;

        HMAP_FOR_EACH(table, hmap_node, &oftable->cls.tables){
            if (minimask_get_vid_mask(&table->mask) == VLAN_VID_MASK) {
                const struct cls_rule * rule;

                HMAP_FOR_EACH(rule, hmap_node, &table->rules){
                    // uint16_t vid = miniflow_get_vid(&rule->match.flow);
                    count++;
                }
            }
        }
    }
    return count;
}

void
ecall_ofproto_get_vlan_r(int bridge_id, uint16_t * buf, int elem){
    struct oftable * oftable;
    int i;
    int p = 0;

    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        const struct cls_table * table;

        HMAP_FOR_EACH(table, hmap_node, &oftable->cls.tables){
            if (minimask_get_vid_mask(&table->mask) == VLAN_VID_MASK) {
                const struct cls_rule * rule;

                HMAP_FOR_EACH(rule, hmap_node, &table->rules){
                    uint16_t vid = miniflow_get_vid(&rule->match.flow);

                    if (p > elem) {
                        return;
                    }
                    buf[p] = vid;
                    p++;
                }
            }
        }
    }
}


size_t
ecall_get_cls_rules(int bridge_id,
                    int table_id,
                    size_t start_index,
                    size_t end_index,
                    struct cls_rule ** buf,
                    size_t buf_size,
                    size_t *n_rules) {

    size_t n = 0, i = 0;
    struct cls_cursor cursor;
    struct sgx_cls_rule * rule;
    cls_cursor_init(&cursor, &SGX_oftables[bridge_id][table_id].cls, NULL);
    CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
        bool buffer_is_full = n >= buf_size;
        // We only want to fetch the rules between [start, end)
        bool is_outside_range = i < start_index || (end_index != -1 && i >= end_index);
        i++;
        if(is_outside_range || buffer_is_full) {
            continue;
        }
        buf[n++] = rule->o_cls_rule;
    }
    *n_rules = i;
    return n;
}

size_t ecall_get_cls_rules_and_enable_eviction(int bridge_id,
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
                                             bool *is_eviction_fields_enabled)
{
    ecall_oftable_enable_eviction(bridge_id, table_id, fields, n_fields, random_v, no_change);
    *is_eviction_fields_enabled = ecall_eviction_fields_enable(bridge_id, table_id);
    if(*no_change || !(*is_eviction_fields_enabled)) {
      return 0;
    }

    return ecall_get_cls_rules(bridge_id, table_id, 0, -1, buf, buf_size, n_rules);
}


/*void ecall_eviction_group_add_rules(struct rule **rules, size_t n) {
    for(int i = 0; i < n; ++i) {
        struct rule *rule = rules[i];
        if(ecall_eviction_fields_enable(rule->ofproto->bridge_id, rule->table_id)) {
            ecall_evg_add_rule(rule->ofproto->bridge_id,
                               rule->table_id, &rule->cr,
                               eviction_group_priority(0),
                               rule_eviction_priority(rule),
                               rule->evg_node);
        }
    }
}*/

void ecall_eviction_group_add_rules(int bridge_id,
                                           int table_id,
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



size_t
ecall_ofproto_get_vlan_usage(int bridge_id,
                           size_t buf_size,
                           uint16_t *vlan_buffer,
                           size_t start_index,
                           size_t end_index,
                           size_t *n_vlan)
{
    struct oftable * oftable;
    size_t i = 0, n = 0;
    for (size_t i = 0; i < SGX_n_tables[bridge_id]; i++) {
        const struct cls_table * table;
        HMAP_FOR_EACH(table, hmap_node, &oftable->cls.tables){
            if (minimask_get_vid_mask(&table->mask) == VLAN_VID_MASK) {
                const struct cls_rule * rule;
                HMAP_FOR_EACH(rule, hmap_node, &table->rules){
                    bool buffer_is_full = n >= buf_size;
                    bool is_outside_range = i < start_index || (end_index != -1 && i >= end_index);
                    i++;
                    if(is_outside_range || buffer_is_full) {
                        continue;
                    }
                    vlan_buffer[n++] = miniflow_get_vid(&rule->match.flow);
                }
            }
        }
    }
    *n_vlan = i;
    return n;
}



size_t
ecall_ofproto_flush(int bridge_id,
                    struct cls_rule **cls_rules,
                    uint32_t *hashes,
                    size_t buf_size,
                    size_t start_index,
                    size_t end_index,
                    size_t *n_rules) {

    int count = 0, n = 0;
    for (size_t i = 0; i < SGX_n_tables[bridge_id]; i++) {
        struct sgx_cls_rule * rule, * next_rule;
        struct cls_cursor cursor;
        if (SGX_oftables[bridge_id][i].flags & OFTABLE_HIDDEN) {
            continue;
        }
        cls_cursor_init(&cursor, &SGX_oftables[bridge_id][i].cls, NULL);
        CLS_CURSOR_FOR_EACH_SAFE(rule, next_rule, cls_rule, &cursor){
            bool buffer_is_full = n >= buf_size;
            count++;
            if(buffer_is_full) {
                continue;
            }
            cls_rules[n] = rule->o_cls_rule;
            hashes[n++] = ecall_cls_rule_hash(bridge_id, rule->o_cls_rule, i);
            ecall_cls_remove(bridge_id, i, rule->o_cls_rule);
            ecall_evg_remove_rule(bridge_id, i, rule->o_cls_rule);
        }
    }
    *n_rules = count;
    return n;
}


bool
operation_is_pending(bool *pendings, struct cls_rule **all_cls_rules, size_t n, struct cls_rule *r) {
    for(size_t i = 0; i < n; ++i) {
        if(all_cls_rules[i] == r) {
            return pendings[i];
        }
    }
    return false;
}

size_t
ecall_ofproto_evict(int bridge_id,
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
    int table_ids[size];

    struct cls_rule *skip_rules[start_index];

    for(size_t table_id = 0; table_id < ofproto_n_tables; table_id++){

        if(!ecall_eviction_fields_enable(bridge_id, table_id)) {
            continue;
        }

        int rules_in_table = ecall_cls_count(bridge_id, table_id);
        int max_flows = ecall_table_mflows(bridge_id, table_id);
        total_nr_evictions += (rules_in_table > max_flows ? rules_in_table - max_flows : 0);
        int count = ecall_cls_count(bridge_id, table_id);

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
            sgx_cls_rule = node_search(bridge_id, tmp);
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
            sgx_cls_rule = node_search(bridge_id, cls_rules[i - start_index]);
            ecall_evg_add_rule(bridge_id, table_ids[i], cls_rules[i  - start_index], group_prios[i], rule_prios[i], sgx_cls_rule->rule_evg_node);
        } else {
            sgx_cls_rule = node_search(bridge_id, skip_rules[i]);
            ecall_evg_add_rule(bridge_id, table_ids[i], skip_rules[i], group_prios[i], rule_prios[i], sgx_cls_rule->rule_evg_node);
        }
    }

     *n_evictions = total_nr_evictions;
     return n - start_index;
}

size_t
ecall_remove_rules(int bridge_id, int *table_ids, struct cls_rule **rules, bool *is_hidden, size_t n_rules) {
    size_t n = 0;
    for(size_t i = 0; i < n_rules; ++i) {
        is_hidden[i] = ecall_cr_priority(bridge_id, rules[i]) > UINT16_MAX;
        ecall_cls_remove(bridge_id, table_ids[i], rules[i]);
        ecall_evg_remove_rule(bridge_id, table_ids[i], rules[i]);
        ecall_cls_rule_destroy(bridge_id, rules[i]);
    }
    return n;
}

void
ecall_add_flow(int bridge_id,
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
             uint32_t eviction_group_priority,
             uint32_t eviction_rule_priority,
             struct heap_node eviction_node,
             struct  cls_rule **pending_deletions,
             int n_pending,
             bool has_timout,
             bool *table_overflow,
             bool *is_rule_modifiable,
             bool *is_deletion_pending,
             bool *is_rule_overlapping,
			 bool *is_read_only)
 {
     if (ecall_istable_readonly(bridge_id, table_id)){
         *is_read_only = true;
         return;
     }

     ecall_cls_rule_init(bridge_id, cr, match, priority);

     for(int i = 0; i < n_pending; ++i) {
         if (ecall_cls_rule_equal(bridge_id, cr, pending_deletions[i])) {
             ecall_cls_rule_destroy(bridge_id, cr);
             *is_deletion_pending = true;
             return;
         }
     }

    if (flags & OFPFF_CHECK_OVERLAP && ecall_cr_rule_overlaps(bridge_id, table_id, cr)) {
         ecall_cls_rule_destroy(bridge_id, cr);
         *is_rule_overlapping = true;
         return;
     }

     ecall_classifier_replace(bridge_id, table_id, cr, victim);
     if(*victim) {
         ecall_evg_remove_rule(bridge_id, table_id, *victim);
     }

     if(ecall_eviction_fields_enable(bridge_id, table_id) && has_timout) {
     	size_t result = ecall_evg_add_rule(bridge_id, table_id, cr, eviction_group_priority,
     	    			eviction_rule_priority, eviction_node);

     }

     bool rule_is_mod = !(ecall_rule_get_flags(bridge_id, table_id) & OFTABLE_READONLY);

     if(*victim && !rule_is_mod) {
         *is_rule_modifiable = false;
         return;
     }

     if (ecall_cls_count(bridge_id, table_id) > ecall_table_mflows(bridge_id, table_id)){
         *table_overflow = true;
         *evict = ecall_choose_rule_to_evict_p(bridge_id, table_id, NULL, cr);
         if(*evict) {
             ecall_cls_remove(bridge_id, table_id, *evict);
             ecall_evg_remove_rule(bridge_id, table_id, *evict);
             *evict_rule_hash = ecall_cls_rule_hash(bridge_id, *evict, table_id);
         }
     }

     *vid = ecall_miniflow_get_vid(bridge_id, cr);
     *vid_mask = ecall_minimask_get_vid_mask(bridge_id, cr);
 }

 size_t
 ecall_collect_rules_strict(int bridge_id,
                            int table_id,
                              int ofproto_n_tables,
                              struct match *match,
                              unsigned int priority,
                              bool *rule_is_hidden_buffer,
                              struct cls_rule **cls_rule_buffer,
                              bool *rule_is_modifiable,
                              size_t buffer_size)
  {
      struct oftable * table;
      struct cls_rule cr;
      size_t n = 0;
      ecall_cls_rule_init_i(bridge_id, &cr, match, priority);
      FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, ofproto_n_tables){
          struct cls_rule * cls_rule = classifier_find_rule_exactly(&table->cls, &cr);
          if(cls_rule) {
              struct sgx_cls_rule * sgx_cls_rule = CONTAINER_OF(cls_rule, struct sgx_cls_rule, cls_rule);
              cls_rule_buffer[n] = sgx_cls_rule->o_cls_rule;
              rule_is_hidden_buffer[n] = ecall_cr_priority(bridge_id, sgx_cls_rule->o_cls_rule) > UINT16_MAX;
              rule_is_modifiable[n] = !(ecall_rule_get_flags(bridge_id, table_id) & OFTABLE_READONLY);
              n++;
          }
      }
      cls_rule_destroy(&cr);
      return n;

  }



  size_t
  ecall_collect_rules_loose(int bridge_id,
                            int table_id,
                            int ofproto_n_tables,
                            size_t start_index,
                            struct match *match,
                            bool *rule_is_hidden_buffer,
                            struct cls_rule **cls_rule_buffer,
                            size_t buffer_size,
                            bool *rule_is_modifiable,
                            size_t *n_rules)
   {
       struct oftable * table;
       struct cls_rule cr;
       ecall_cls_rule_init_i(bridge_id, &cr, match, 0);
       size_t n = 0, count = 0;

       FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, ofproto_n_tables){
           struct cls_cursor cursor;
           struct sgx_cls_rule * rule;

           cls_cursor_init(&cursor, &table->cls, &cr);
           CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
               if (n >= buffer_size || (count < start_index)) {
                   count++;
                   continue;
               }
               count++;
               cls_rule_buffer[n] = rule->o_cls_rule;
               rule_is_hidden_buffer[n] = ecall_cr_priority(bridge_id, rule->o_cls_rule) > UINT16_MAX;
               rule_is_modifiable[n] = !(ecall_rule_get_flags(bridge_id, table_id) & OFTABLE_READONLY);
               n++;
           }
       }
       cls_rule_destroy(&cr);
       *n_rules = count;
       return n;
   }




   void
   ecall_delete_flows(int bridge_id,
   				 int *rule_table_ids,
   				 struct cls_rule **cls_rules,
   				 bool *rule_is_hidden,
   				 uint32_t *rule_hashes,
   				 unsigned int *rule_priorities,
   				 struct match *match, size_t n)
{
    for(int i = 0; i < n; ++i) {
        struct sgx_cls_rule * sgx_cls_rule;
        sgx_cls_rule = node_search(bridge_id, cls_rules[i]);

        rule_is_hidden[i] = ecall_cr_priority(bridge_id, sgx_cls_rule->o_cls_rule) > UINT16_MAX;
        ecall_minimatch_expand(bridge_id, sgx_cls_rule->o_cls_rule, &match[i]);
        rule_priorities[i] = ecall_cr_priority(bridge_id, sgx_cls_rule->o_cls_rule);
        rule_hashes[i] = ecall_cls_rule_hash(bridge_id, sgx_cls_rule->o_cls_rule, rule_table_ids[i]);

        ecall_cls_remove(bridge_id, rule_table_ids[i], sgx_cls_rule->o_cls_rule);
        ecall_evg_remove_rule(bridge_id, rule_table_ids[i], sgx_cls_rule->o_cls_rule);
    }
}

bool
ecall_need_to_evict(int bridge_id, int table_id) {
    bool eviction_is_enabled = ecall_eviction_fields_enable(bridge_id, table_id);
    bool overflow = ecall_cls_count(bridge_id, table_id) > ecall_table_mflows(bridge_id, table_id);
    return eviction_is_enabled && overflow;
}

void
ecall_configure_table(int bridge_id,
                     int table_id,
                     char *name,
                     unsigned int max_flows,
                     struct mf_subfield *groups,
                     size_t n_groups,
                     bool *need_to_evict,
                     bool *is_read_only)
{

    ecall_oftable_set_name(bridge_id, table_id, name);

    if(ecall_istable_readonly(bridge_id, table_id)){
        *is_read_only = true;
        return;
    }

    if (groups) {
        return;
    } else {
        ecall_oftable_disable_eviction(bridge_id, table_id);
    }

    ecall_table_mflows_set(bridge_id, table_id, max_flows);
    *need_to_evict = ecall_need_to_evict(bridge_id, table_id);
}


size_t
ecall_collect_rules_loose_stats_request(int bridge_id,
                                      int table_id,
                                      int n_tables,
                                      size_t start_index,
                                      size_t buffer_size,
                                      struct match *match,
                                      struct cls_rule **cls_rules,
                                      struct match *matches,
                                      unsigned int *priorities,
                                      size_t *n_rules)
{
    struct oftable * table;
    struct cls_rule cr;
    ecall_cls_rule_init_i(bridge_id, &cr, match, 0);

    size_t n = 0, count = 0;

    FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, n_tables){
        struct cls_cursor cursor;
        struct sgx_cls_rule * rule;

        cls_cursor_init(&cursor, &table->cls, &cr);
        CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
            unsigned int priority = ecall_cr_priority(bridge_id, rule->o_cls_rule);
            bool rule_is_hidden = priority > UINT16_MAX;
            if(rule_is_hidden) {
                continue;
            }
            if (n >= buffer_size || (count < start_index)) {
                count++;
                continue;
            }
            count++;

            cls_rules[n] = rule->o_cls_rule;
            priorities[n] = ecall_cr_priority(bridge_id, rule->o_cls_rule);
            ecall_minimatch_expand(bridge_id, rule->o_cls_rule, &matches[n]);
            n++;
        }
    }
    cls_rule_destroy(&cr);
    if(n_rules) {
        *n_rules = count;
    }
    return n;
}


void
ecall_ofproto_rule_send_removed(int bridge_id, struct cls_rule *cr, struct match *match, unsigned int *priority, bool *rule_is_hidden)
{
    unsigned int pr = ecall_cr_priority(bridge_id, cr);
    *rule_is_hidden = pr > UINT16_MAX;
    if (*rule_is_hidden) {
        return;
    }
    ecall_minimatch_expand(bridge_id, cr, match);
    *priority = pr;
}
