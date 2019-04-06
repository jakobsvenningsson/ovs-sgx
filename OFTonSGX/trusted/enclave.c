#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include "enclave.h"
#include "enclave_t.h" /* print_string */
#include <stdbool.h>
#include "classifier.h"
#include "ofproto-provider.h"

#include "call-table.h"
#include "hotcall.h"
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

void
ecall_ofproto_init_tables(int bridge_id, int n_tables){
    struct oftable * table;

    SGX_n_tables[bridge_id] = n_tables;
    SGX_oftables[bridge_id] = xmalloc(n_tables * sizeof(struct oftable));
    OFPROTO_FOR_EACH_TABLE(table, SGX_oftables[bridge_id]){
        oftable_init(table);
    }
    sgx_table_cls_init(bridge_id);
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
    printf("INSIDE\n");
    printf("%d\n", bridge_id);
    printf("%u\n", priority);
    printf("%p\n", o_cls_rule);
    printf("%p\n", match);

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
    printf("%d %d %p %p\n", bridge_id, table_id, o_cls_rule, cls_rule_rtrn);
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

// 20. choose and return a rule to evict from table, without including the rule itself
struct cls_rule *
ecall_choose_rule_to_evict_p(int bridge_id, int table_id, struct cls_rule ** o_cls_rule, struct cls_rule * replacer){
    struct sgx_cls_rule * sgx_cls_rule = node_search(bridge_id, replacer);
    bool was_evictable;

    was_evictable = sgx_cls_rule->evictable;
    // Make rule we are inserting immune to eviction.
    sgx_cls_rule->evictable = false;
    struct cls_rule * tmp;
    ecall_choose_rule_to_evict(bridge_id, table_id, &tmp);
    // Put back old eviction status
    sgx_cls_rule->evictable = was_evictable;
    return tmp;
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

        printf("heap_destroy_ovs size %d\n", heap_count_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size));


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
        printf("NOOOOO CHANGE ENCLAVE!!------------------------------------------------!!!");
        return;
    }

    ecall_oftable_disable_eviction(bridge_id, table_id);


    SGX_oftables[bridge_id][table_id].n_eviction_fields = n_fields;
    // CAUSES CRASH SGX_oftables[bridge_id][table_id].eviction_fields =
    SGX_oftables[bridge_id][table_id].eviction_fields         = xmemdup(fields, n_fields * sizeof *fields);
    SGX_oftables[bridge_id][table_id].eviction_group_id_basis = random_v;

    hmap_init(&SGX_oftables[bridge_id][table_id].eviction_groups_by_id);
    heap_init_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size);

    printf("enable eviction size %d\n", heap_count_ovs(&SGX_oftables[bridge_id][table_id].eviction_groups_by_size));

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

// 1. Creation and Initialization
void
ecall_SGX_table_dpif(int bridge_id, int n_tables){
    // I need to create the struct SGX_table_dpif in memory
    int i;

    SGX_table_dpif[bridge_id] = xmalloc(n_tables * sizeof(struct SGX_table_dpif));
    for (i = 0; i < n_tables; i++) {
        SGX_table_dpif[bridge_id][i].catchall_table = NULL;
        SGX_table_dpif[bridge_id][i].other_table    = NULL;
    }
}

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
void
ecall_table_dpif_init(int bridge_id){
    // Initialization of the table_dpif
    int i;

    for (i = 0; i < N_TABLES; i++) {
        struct SGX_table_dpif * table = &SGX_table_dpif[bridge_id][i];
        table->catchall_table = NULL;
        table->other_table    = NULL;
        table->basis = random_uint32();
    }
}

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
