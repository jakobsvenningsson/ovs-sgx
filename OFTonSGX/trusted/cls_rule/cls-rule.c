#include "enclave_t.h"
#include "enclave.h"
#include "cls-rule.h"
#include "enclave-batch-allocator.h"

// Global data structures
extern struct oftable * SGX_oftables[100];
extern struct SGX_table_dpif * SGX_table_dpif[100];
extern int SGX_n_tables[100];
extern struct sgx_cls_table * SGX_hmap_table[100];
extern struct batch_allocator cr_ba;

// Vanilla ECALLS

void
ecall_cls_rule_init(uint8_t bridge_id, struct cls_rule * o_cls_rule, const struct match * match, unsigned int priority){
    struct sgx_cls_rule * sgx_cls_rule = node_insert(bridge_id, (uint32_t)(uintptr_t) o_cls_rule);
    sgx_cls_rule->o_cls_rule = o_cls_rule;
    cls_rule_init(&sgx_cls_rule->cls_rule, match, priority);
    sgx_cls_rule->evictable = true;
}

int
ecall_cr_rule_overlaps(uint8_t bridge_id, uint8_t table_id, struct cls_rule * out){
    struct sgx_cls_rule * sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, out);
    struct cls_rule * cls_rule         = &sgx_cls_rule->cls_rule;
    const struct classifier * cls      = &SGX_oftables[bridge_id][table_id].cls;
    const struct cls_rule * target     = cls_rule;
    return classifier_rule_overlaps(cls, target);
}

void
ecall_cls_rule_destroy(uint8_t bridge_id, struct cls_rule * out){
    struct sgx_cls_rule * sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, out);
    struct cls_rule * cls_rule         = &sgx_cls_rule->cls_rule;

    cls_rule_destroy(cls_rule);
    node_delete(bridge_id, sgx_cls_rule->o_cls_rule);
    sgx_cls_rule = NULL;
}

uint32_t
ecall_cls_rule_hash(uint8_t bridge_id, const struct cls_rule * o_cls_rule, uint32_t basis){
    struct sgx_cls_rule * sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, o_cls_rule);
    struct cls_rule * cls_rule         = &sgx_cls_rule->cls_rule;
    return cls_rule_hash(cls_rule, basis);
}

int
ecall_cls_rule_equal(uint8_t bridge_id, const struct cls_rule * out_a, const struct cls_rule * out_b){
    const struct cls_rule * a = &sgx_rule_from_ut_cr(bridge_id, out_a)->cls_rule;
    const struct cls_rule * b = &sgx_rule_from_ut_cr(bridge_id, out_b)->cls_rule;
    return cls_rule_equal(a, b);
}

void
ecall_cls_remove(uint8_t bridge_id, uint8_t table_id, struct cls_rule * o_cls_rule){
    struct sgx_cls_rule * sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, o_cls_rule);
    classifier_remove(&SGX_oftables[bridge_id][table_id].cls, &sgx_cls_rule->cls_rule);
}

unsigned int
ecall_cr_priority(uint8_t bridge_id, const struct cls_rule * o_cls_rule){
    struct sgx_cls_rule * sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, o_cls_rule);
    return sgx_cls_rule->cls_rule.priority;
}

int
ecall_cls_rule_is_loose_match(uint8_t bridge_id, struct cls_rule * o_cls_rule, const struct minimatch * criteria){
    struct sgx_cls_rule * sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, o_cls_rule);

    if (cls_rule_is_loose_match(&sgx_cls_rule->cls_rule, criteria)) {
        return 100;
    }

    return 0;
}

unsigned int
ecall_cls_rule_format(uint8_t bridge_id, const struct cls_rule * o_cls_rule, struct match * megamatch){
    struct sgx_cls_rule * sgx_cls_rule;

    sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, o_cls_rule);
    minimatch_expand(&sgx_cls_rule->cls_rule.match, megamatch);
    return sgx_cls_rule->cls_rule.priority;
}


uint32_t
ecall_rule_calculate_tag(uint8_t bridge_id, struct cls_rule * o_cls_rule, const struct flow * flow, uint8_t table_id){
    // Retrieve the cls_rule
    struct sgx_cls_rule * sgx_cls_rule;

    sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, o_cls_rule);
    if (minimask_is_catchall(&sgx_cls_rule->cls_rule.match.mask)) {
        return 0;
    } else {
        uint32_t secret = SGX_table_dpif[bridge_id][table_id].basis;
        uint32_t hash   = flow_hash_in_minimask(flow, &sgx_cls_rule->cls_rule.match.mask, secret);
        return hash;
    }
}

uint32_t
ecall_rule_calculate_tag_s(uint8_t bridge_id, int id, const struct flow * flow){
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
ecall_miniflow_expand(uint8_t bridge_id, struct cls_rule * o_cls_rule, struct flow * flow){
    // From untrusted the Pointer the sgx_cls_rule is retrieved.
    struct sgx_cls_rule * sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, o_cls_rule);

    // Need to call miniflow_expand to copy the information in the just passed flow struct.
    miniflow_expand(&sgx_cls_rule->cls_rule.match.flow, flow);
}


// This functions are for ofopgroup_complete()
uint16_t
ecall_minimask_get_vid_mask(uint8_t bridge_id, struct cls_rule * o_cls_rule){
    struct sgx_cls_rule * sgx_cls_rule;
    sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, o_cls_rule);
    return minimask_get_vid_mask(&sgx_cls_rule->cls_rule.match.mask);
}

uint16_t
ecall_miniflow_get_vid(uint8_t bridge_id, struct cls_rule * o_cls_rule){
    struct sgx_cls_rule * sgx_cls_rule;
    sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, o_cls_rule);
    return miniflow_get_vid(&sgx_cls_rule->cls_rule.match.flow);
}

void
ecall_minimatch_expand(uint8_t bridge_id, struct cls_rule * o_cls_rule, struct match * dst){
    struct sgx_cls_rule * sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, o_cls_rule);
    minimatch_expand(&sgx_cls_rule->cls_rule.match, dst);
}


// Optimized ECALLS

void
ecall_cls_rules_format(uint8_t bridge_id, const struct cls_rule *cls_rules, struct match *megamatches, size_t n) {
    for(size_t i = 0; i < n; ++i) {
        ecall_cls_rule_format(bridge_id, &cls_rules[i], &megamatches[i]);
    }
}

void
ecall_minimatch_expand_and_get_priority(uint8_t bridge_id, struct cls_rule *ut_cr, struct match *match, unsigned int *priority) {
    ecall_minimatch_expand(bridge_id, ut_cr, match);
    *priority = ecall_cr_priority(bridge_id, ut_cr);
}

uint32_t
ecall_miniflow_expand_and_tag(uint8_t bridge_id, struct cls_rule *ut_cr, struct flow *flow, uint8_t table_id) {
    ecall_miniflow_expand(bridge_id, ut_cr, flow);
    return ecall_rule_calculate_tag(bridge_id, ut_cr, flow, table_id);
}

// Helpers

void
sgx_cls_rule_init_i(uint8_t bridge_id, struct cls_rule * cls_rule, const struct match * match, unsigned int priority){
    cls_rule_init(cls_rule, match, priority);
}


struct sgx_cls_rule *
node_insert(uint8_t bridge_id, uint32_t hash){
    struct sgx_cls_rule * new;
    #ifdef BATCH_ALLOCATION
    struct bblock *b = batch_allocator_get_block(&cr_ba);
    new = (struct sgx_cls_rule *)  b->ptr;
    new->block_list_node = &b->list_node;
    #else
    new = xmalloc(sizeof(struct sgx_cls_rule));
    #endif
    new->hmap_node.hash = hash;
    hmap_insert(&SGX_hmap_table[bridge_id]->cls_rules, &new->hmap_node, new->hmap_node.hash, NULL, 0);
    return new;
}

void
node_delete(uint8_t bridge_id, struct cls_rule * out){
    struct sgx_cls_rule * rule;

    rule = sgx_rule_from_ut_cr(bridge_id, out);
    hmap_remove(&SGX_hmap_table[bridge_id]->cls_rules, &rule->hmap_node);
    #ifdef BATCH_ALLOCATION
    batch_allocator_free_block(&cr_ba, rule->block_list_node);
    #else
    free(rule);
    #endif
}

struct sgx_cls_rule *
sgx_rule_from_ut_cr(uint8_t bridge_id, const struct cls_rule * out){
    struct sgx_cls_rule * rule;

    HMAP_FOR_EACH_WITH_HASH(rule, hmap_node, (uint32_t)(uintptr_t) out, &SGX_hmap_table[bridge_id]->cls_rules){
        return rule;
    }
    return NULL;
}
