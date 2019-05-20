#include "enclave_t.h"
#include <stdbool.h>

#include "enclave.h"
#include "oftable.h"
#include "cls-rule.h"
#include "openflow-common.h"

enum nx_flow_monitor_flags {
    /* When to send updates. */
    NXFMF_INITIAL = 1 << 0,     /* Initially matching flows. */
    NXFMF_ADD = 1 << 1,         /* New matching flows as they are added. */
    NXFMF_DELETE = 1 << 2,      /* Old matching flows as they are removed. */
    NXFMF_MODIFY = 1 << 3,      /* Matching flows as they are changed. */

    /* What to include in updates. */
    NXFMF_ACTIONS = 1 << 4,     /* If set, actions are included. */
    NXFMF_OWN = 1 << 5,         /* If set, include own changes in full. */
};

struct rule {
    struct list ofproto_node;    /* Owned by ofproto base code. */
    void *ofproto;     /* The ofproto that contains this rule. */
    struct cls_rule cr;          /* In owning ofproto's classifier. */

    void *pending; /* Operation now in progress, if nonnull. */

    ovs_be64 flow_cookie;        /* Controller-issued identifier. */

    long long int created;       /* Creation time. */
    long long int modified;      /* Time of last modification. */
    long long int used;          /* Last use; time created if never used. */
    uint16_t hard_timeout;       /* In seconds from ->modified. */
    uint16_t idle_timeout;       /* In seconds from ->used. */
    uint8_t table_id;            /* Index in ofproto's 'tables' array. */
    bool send_flow_removed;      /* Send a flow removed message? */

    /* Eviction groups. */
    #ifndef SGX
    bool evictable;              /* If false, prevents eviction. */
    struct heap_node evg_node;   /* In eviction_group's "rules" heap. */
    struct eviction_group *eviction_group; /* NULL if not in any group. */
    #endif

    void *ofpacts;      /* Sequence of "struct ofpacts". */
    unsigned int ofpacts_len;    /* Size of 'ofpacts', in bytes. */

    /* Flow monitors. */
    enum nx_flow_monitor_flags monitor_flags;
    uint64_t add_seqno;         /* Sequence number when added. */
    uint64_t modify_seqno;      /* Sequence number when changed. */

    /* Optimisation for flow expiry. */
    struct list expirable;      /* In ofproto's 'expirable' list if this rule
                                 * is expirable, otherwise empty. */

    uint16_t tmp_storage_vid;
    uint8_t tmp_storage_vid_exist;
    uint16_t tmp_storage_vid_mask;
    uint8_t tmp_storage_vid_mask_exist;

    bool is_other_table;
    int table_update_taggable;
};


// Global data structures
extern struct oftable * SGX_oftables[100];
extern struct SGX_table_dpif * SGX_table_dpif[100];
extern int SGX_n_tables[100];
extern struct sgx_cls_table * SGX_hmap_table[100];

// Vanilla ECALLS

void
ecall_oftable_set_name(uint8_t bridge_id, uint8_t table_id, char * name){
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

void
ecall_oftable_name(uint8_t bridge_id, uint8_t table_id, char * buf, size_t len){
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

enum
oftable_flags
ecall_oftable_get_flags(uint8_t bridge_id, uint8_t table_id){
    return SGX_oftables[bridge_id][table_id].flags;
}

int
ecall_oftable_cls_count(uint8_t bridge_id, uint8_t table_id){
    return classifier_count(&SGX_oftables[bridge_id][table_id].cls);
}

unsigned int
ecall_oftable_mflows(uint8_t bridge_id, uint8_t table_id){
    return SGX_oftables[bridge_id][table_id].max_flows;
}

void
ecall_oftable_mflows_set(uint8_t bridge_id, uint8_t table_id, unsigned int value){
    SGX_oftables[bridge_id][table_id].max_flows = value;
}

void
ecall_oftable_set_readonly(uint8_t bridge_id, uint8_t table_id){
    SGX_oftables[bridge_id][table_id].flags = OFTABLE_HIDDEN | OFTABLE_READONLY;
}

int
ecall_oftable_is_readonly(uint8_t bridge_id, uint8_t table_id){
    return SGX_oftables[bridge_id][table_id].flags & OFTABLE_READONLY;
}

void
ecall_oftable_hidden_check(uint8_t bridge_id){
    int i;
    for (i = 0; i + 1 < SGX_n_tables[bridge_id]; i++) {
        enum oftable_flags flags      = SGX_oftables[bridge_id][i].flags;
        enum oftable_flags next_flags = SGX_oftables[bridge_id][i + 1].flags;
        ovs_assert(!(flags & OFTABLE_HIDDEN) || (next_flags & OFTABLE_HIDDEN));
    }
}

static int i = 0;
void
ecall_oftable_classifier_replace(uint8_t bridge_id, uint8_t table_id, struct cls_rule *ut_cr, struct cls_rule **cls_rule_rtrn){
    struct sgx_cls_rule * sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, ut_cr);
    struct cls_rule * cls_rule = NULL;
    cls_rule = classifier_replace(&SGX_oftables[bridge_id][table_id].cls,
      &sgx_cls_rule->cls_rule);

    if (cls_rule) {
        struct sgx_cls_rule * sgx_cls_rule_r = CONTAINER_OF(cls_rule, struct sgx_cls_rule, cls_rule);
        *cls_rule_rtrn = sgx_cls_rule_r->o_cls_rule;
    } else  {
        *cls_rule_rtrn = NULL;
    }
}

int
ecall_oftable_is_other_table(uint8_t bridge_id, int id){
    if (SGX_table_dpif[bridge_id][id].other_table) {
        return 100;
    }
    return 0;
}

int
ecall_oftable_update_taggable(uint8_t bridge_id, uint8_t table_id){
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
}

/* Finds and returns a rule in 'cls' with priority 'priority' and exactly the
 * same matching criteria as 'target'.  Returns a null pointer if 'cls' doesn't
 * contain an exact match. */
void
ecall_oftable_cls_find_match_exactly(uint8_t bridge_id, uint8_t table_id, const struct match * target, unsigned int priority,
  struct cls_rule ** o_cls_rule){
    struct cls_rule * cls_rule = classifier_find_match_exactly(&SGX_oftables[bridge_id][table_id].cls, target, priority);

    *o_cls_rule = cls_rule ?
      CONTAINER_OF(cls_rule, struct sgx_cls_rule, cls_rule)->o_cls_rule :
      NULL;
}

void
ecall_oftable_cls_lookup(uint8_t bridge_id, struct cls_rule **ut_cr, uint8_t table_id, const struct flow *flow,
  struct flow_wildcards * wc){
    struct cls_rule * cls_rule;
    cls_rule = classifier_lookup(&SGX_oftables[bridge_id][table_id].cls, flow, wc);
    if (cls_rule) {
        // Need to retrieve the sgx_cls_rule and return the pointer
        // to untrusted memory
        struct sgx_cls_rule * sgx_cls_rule = CONTAINER_OF(cls_rule, struct sgx_cls_rule, cls_rule);
        *ut_cr = sgx_cls_rule->o_cls_rule;
    } else  {
        *ut_cr = NULL;
    }
}

size_t
ecall_collect_rules_loose_c(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id, const struct match * match){
    return ecall_collect_rules_loose_r(bridge_id, ofproto_n_tables, NULL, -1, table_id, match);
}

size_t
ecall_collect_rules_loose_r(uint8_t bridge_id, int ofproto_n_tables, struct cls_rule ** buf, int elem, uint8_t table_id,
  const struct match * match){
    struct oftable * table;
    struct cls_rule cr;
    bool count_only = buf == NULL ? true : false;

    sgx_cls_rule_init_i(bridge_id, &cr, match, 0);
    size_t p = 0;
    FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, ofproto_n_tables){
        struct cls_cursor cursor;
        struct sgx_cls_rule * rule;

        cls_cursor_init(&cursor, &table->cls, &cr);
        CLS_CURSOR_FOR_EACH(rule, cls_rule, &cursor){
            if(count_only) {
                p++;
                continue;
            }

            if (p > elem) {
                return p;
            }
            buf[p] = rule->o_cls_rule;
            p++;
        }
    }
    cls_rule_destroy(&cr);
    return p;
}

size_t
ecall_flush_c(uint8_t bridge_id){
    return ecall_flush_r(bridge_id, NULL, -1);
}

size_t
ecall_flush_r(uint8_t bridge_id, struct cls_rule ** buf, int elem){
    size_t p = 0;
    bool count_only = buf == NULL ? true : false;

    int i;
    for (i = 0; i < SGX_n_tables[bridge_id]; i++) {
        struct sgx_cls_rule * rule, * next_rule;
        struct cls_cursor cursor;
        if (SGX_oftables[bridge_id][i].flags & OFTABLE_HIDDEN) {
            continue;
        }
        cls_cursor_init(&cursor, &SGX_oftables[bridge_id][i].cls, NULL);
        CLS_CURSOR_FOR_EACH_SAFE(rule, next_rule, cls_rule, &cursor){
            if(count_only) {
                p++;
                continue;
            }

            if (p > elem) {
                return p;
            }
            buf[p] = rule->o_cls_rule;
            p++;
        }
    }
    return p;
}

size_t
ecall_collect_rules_strict_c(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id, const struct match * match, unsigned int priority){
    return ecall_collect_rules_strict_r(bridge_id, ofproto_n_tables, NULL, -1, table_id, match, priority);
}

size_t
ecall_collect_rules_strict_r(uint8_t bridge_id, int ofproto_n_tables, struct cls_rule ** buf, int elem, uint8_t table_id,
  const struct match * match,
  unsigned int priority){
    struct oftable * table;
    struct cls_rule cr;
    bool count_only = buf == NULL ? true : false;

    sgx_cls_rule_init_i(bridge_id, &cr, match, priority);
    size_t p = 0;
    FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, ofproto_n_tables){
        struct cls_rule * cls_rule = classifier_find_rule_exactly(&table->cls, &cr);

        if (cls_rule) {
            if(count_only) {
                p++;
                continue;
            }

            if (p > elem) {
                return p;
            }
            struct sgx_cls_rule * sgx_cls_rule = CONTAINER_OF(cls_rule, struct sgx_cls_rule, cls_rule);
            buf[p] = sgx_cls_rule->o_cls_rule;
            p++;
        }
    }
    cls_rule_destroy(&cr);
    return p;
}

// Optimized ECALLS

size_t
ecall_oftable_get_cls_rules(uint8_t bridge_id,
                    uint8_t table_id,
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


void
ecall_oftable_configure(uint8_t bridge_id,
                     uint8_t table_id,
                     char *name,
                     unsigned int max_flows,
                     struct mf_subfield *groups,
                     size_t n_groups,
                     bool *need_to_evict,
                     bool *is_read_only)
{

    ecall_oftable_set_name(bridge_id, table_id, name);

    if(ecall_oftable_is_readonly(bridge_id, table_id)){
        *is_read_only = true;
        return;
    }

    if (groups) {
        return;
    } else {
        ecall_oftable_disable_eviction(bridge_id, table_id);
    }

    ecall_oftable_mflows_set(bridge_id, table_id, max_flows);
    *need_to_evict = ecall_need_to_evict(bridge_id, table_id);
}

void
ecall_oftable_remove_rules(uint8_t bridge_id, uint8_t *table_ids, struct cls_rule **rules, bool *is_hidden, size_t n_rules) {
    struct rule *rule = NULL;
    for(size_t i = 0; i < n_rules; ++i) {
        is_hidden[i] = ecall_cr_priority(bridge_id, rules[i]) > UINT16_MAX;
        rule = CONTAINER_OF(rules[i], struct rule, cr);
        if(rule) {
            rule->table_update_taggable = ecall_oftable_update_taggable(bridge_id, table_ids[i]);
            rule->is_other_table = ecall_oftable_is_other_table(bridge_id, table_ids[i]);
        }
        if(rules[i]) {
            ecall_cls_remove(bridge_id, table_ids[i], rules[i]);
            ecall_evg_remove_rule(bridge_id, table_ids[i], rules[i]);
        }
    }
}

void
ecall_add_flow(uint8_t bridge_id,
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
             uint32_t eviction_rule_priority,
             struct  cls_rule **pending_deletions,
             int n_pending,
             bool has_timout,
             uint16_t *state,
             int *table_update_taggable,
             unsigned int *evict_priority)
 {
     if (ecall_oftable_is_readonly(bridge_id, table_id)){
         //is_read_only
         *state |= (1 << 4);
         return;
     }

     ecall_cls_rule_init(bridge_id, cr, match, priority);

     struct sgx_cls_rule *sgx_cr = sgx_rule_from_ut_cr(bridge_id, cr);

     for(int i = 0; i < n_pending; ++i) {
         if (ecall_cls_rule_equal(bridge_id, cr, pending_deletions[i])) {
             ecall_cls_rule_destroy(bridge_id, cr);
             //is_deletion_pending
             *state |= (1 << 2);
             return;
         }
     }

    if (flags & OFPFF_CHECK_OVERLAP && ecall_cr_rule_overlaps(bridge_id, table_id, cr)) {
         ecall_cls_rule_destroy(bridge_id, cr);
        // is_rule_overlapping
         *state |= (1 << 3);
         return;
     }

     ecall_oftable_classifier_replace(bridge_id, table_id, cr, victim);
     if(*victim) {
         ecall_evg_remove_rule(bridge_id, table_id, *victim);
     }

     if(ecall_is_eviction_fields_enabled(bridge_id, table_id) && has_timout) {
     	ecall_evg_add_rule(bridge_id, table_id, cr, NULL,
     	    			eviction_rule_priority);

     }

     bool rule_is_mod = !(ecall_oftable_get_flags(bridge_id, table_id) & OFTABLE_READONLY);
     if(*victim && !rule_is_mod) {
        // is_rule_modifiable
         *state |= (1 << 1);
         return;
     }

     if (ecall_oftable_cls_count(bridge_id, table_id) > ecall_oftable_mflows(bridge_id, table_id)){
         // table_overflow
         *state |= (1 << 0);
         ecall_choose_rule_to_evict_p(bridge_id, table_id, evict, cr);
         if(*evict) {
             bool rule_is_hidden = false;
             ecall_ofproto_rule_send_removed(bridge_id, *evict, match, evict_priority, &rule_is_hidden);
             struct rule *evict_rule;
             evict_rule = CONTAINER_OF(*evict, struct rule, cr);
             evict_rule->table_update_taggable = ecall_oftable_update_taggable(bridge_id, table_id);
             evict_rule->is_other_table = ecall_oftable_is_other_table(bridge_id, table_id);
             ecall_cls_remove(bridge_id, table_id, *evict);
             ecall_evg_remove_rule(bridge_id, table_id, *evict);
             *evict_rule_hash = ecall_cls_rule_hash(bridge_id, *evict, table_id);
         }
     }

     *vid = ecall_miniflow_get_vid(bridge_id, cr);
     *vid_mask = ecall_minimask_get_vid_mask(bridge_id, cr);

     //*is_hidden
     *state |= ((ecall_cr_priority(bridge_id, cr) > UINT16_MAX) << 5);

     *table_update_taggable = ecall_oftable_update_taggable(bridge_id, table_id);

     //*is_other_table
     *state |= (ecall_oftable_is_other_table(bridge_id, table_id) << 6);

 }

 void
 ecall_delete_flows(uint8_t bridge_id,
                  uint8_t *rule_table_ids,
                  struct cls_rule **cls_rules,
                  bool *rule_is_hidden,
                  uint32_t *rule_hashes,
                  unsigned int *rule_priorities,
                  struct match *match,
                  int *table_update_taggable,
                  uint8_t *is_other_table,
                  size_t n)
{
  for(int i = 0; i < n; ++i) {
      struct sgx_cls_rule * sgx_cls_rule;
      sgx_cls_rule = sgx_rule_from_ut_cr(bridge_id, cls_rules[i]);

      rule_is_hidden[i] = ecall_cr_priority(bridge_id, sgx_cls_rule->o_cls_rule) > UINT16_MAX;
      ecall_minimatch_expand(bridge_id, sgx_cls_rule->o_cls_rule, &match[i]);
      rule_priorities[i] = ecall_cr_priority(bridge_id, sgx_cls_rule->o_cls_rule);
      rule_hashes[i] = ecall_cls_rule_hash(bridge_id, sgx_cls_rule->o_cls_rule, rule_table_ids[i]);

      ecall_cls_remove(bridge_id, rule_table_ids[i], sgx_cls_rule->o_cls_rule);
      ecall_evg_remove_rule(bridge_id, rule_table_ids[i], sgx_cls_rule->o_cls_rule);


      table_update_taggable[i] = ecall_oftable_update_taggable(bridge_id, rule_table_ids[i]);
      is_other_table[i] = ecall_oftable_is_other_table(bridge_id, rule_table_ids[i]);
  }
}


 size_t
 ecall_collect_rules_strict(uint8_t bridge_id,
                            uint8_t table_id,
                              int ofproto_n_tables,
                              struct match *match,
                              unsigned int priority,
                              struct cls_rule **cls_rule_buffer,
                              bool *rule_is_modifiable,
                              size_t buffer_size)
  {
      struct oftable * table;
      struct cls_rule cr;
      size_t n = 0;
      sgx_cls_rule_init_i(bridge_id, &cr, match, priority);
      FOR_EACH_MATCHING_TABLE(bridge_id, table, table_id, ofproto_n_tables){
          struct cls_rule * cls_rule = classifier_find_rule_exactly(&table->cls, &cr);
          if(cls_rule) {
              struct sgx_cls_rule * sgx_cls_rule = CONTAINER_OF(cls_rule, struct sgx_cls_rule, cls_rule);
              bool is_hidden = ecall_cr_priority(bridge_id, sgx_cls_rule->o_cls_rule) > UINT16_MAX;
              if(is_hidden) {
                  continue;
              }
              cls_rule_buffer[n] = sgx_cls_rule->o_cls_rule;
              //rule_is_hidden_buffer[n] = ecall_cr_priority(bridge_id, sgx_cls_rule->o_cls_rule) > UINT16_MAX;
              rule_is_modifiable[n] = !(ecall_oftable_get_flags(bridge_id, table_id) & OFTABLE_READONLY);
              n++;
          }
      }
      cls_rule_destroy(&cr);
      return n;

  }



  size_t
  ecall_collect_rules_loose(uint8_t bridge_id,
                            uint8_t table_id,
                            int ofproto_n_tables,
                            size_t start_index,
                            struct match *match,
                            struct cls_rule **cls_rule_buffer,
                            size_t buffer_size,
                            bool *rule_is_modifiable,
                            bool *rule_is_hidden,
                            int *table_update_tagable,
                            uint8_t *is_other_table,
                            size_t *n_rules)
   {
       struct oftable * table;
       struct cls_rule cr;
       sgx_cls_rule_init_i(bridge_id, &cr, match, 0);
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
               rule_is_hidden[n] = ecall_cr_priority(bridge_id, rule->o_cls_rule) > UINT16_MAX;
               cls_rule_buffer[n] = rule->o_cls_rule;
               rule_is_modifiable[n] = !(ecall_oftable_get_flags(bridge_id, table_id) & OFTABLE_READONLY);
               /*if(table_update_tagable) {
                   table_update_tagable[n] = ecall_oftable_update_taggable(bridge_id, table_id);
               }
               if(is_other_table) {
                   is_other_table[n] = ecall_oftable_is_other_table(bridge_id, table_id) > UINT16_MAX;
               }*/
               n++;
           }
       }
       cls_rule_destroy(&cr);
       *n_rules = count;
       return n;
   }



// Helpers

void
sgx_table_cls_init(uint8_t bridge_id){
    SGX_hmap_table[bridge_id] = xmalloc(sizeof(struct sgx_cls_table));
    hmap_init(&SGX_hmap_table[bridge_id]->cls_rules, NULL);
}

void
oftable_init(struct oftable * table){
    memset(table, 0, sizeof *table);
    classifier_init(&table->cls);
    table->max_flows = UINT_MAX;
}

void
sgx_table_dpif_init(uint8_t bridge_id, int n_tables){
    // I need to create the struct SGX_table_dpif in memory
    int i;

    SGX_table_dpif[bridge_id] = xmalloc(n_tables * sizeof(struct SGX_table_dpif));
    for (i = 0; i < n_tables; i++) {
        SGX_table_dpif[bridge_id][i].catchall_table = NULL;
        SGX_table_dpif[bridge_id][i].other_table    = NULL;
    }
}

void
sgx_oftable_destroy(uint8_t bridge_id, uint8_t table_id){
    ovs_assert(classifier_is_empty(&SGX_oftables[bridge_id][table_id].cls));
    ecall_oftable_disable_eviction(bridge_id, table_id);
    classifier_destroy(&SGX_oftables[bridge_id][table_id].cls);
    free(SGX_oftables[bridge_id][table_id].name);
}

struct oftable *
next_visible_table(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id){
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

struct oftable *
first_matching_table(uint8_t bridge_id, int ofproto_n_tables, uint8_t table_id){
    if (table_id == 0xff) {
        return next_visible_table(bridge_id, ofproto_n_tables, 0);
    } else if (table_id < ofproto_n_tables) {
        return &SGX_oftables[bridge_id][table_id];
    } else {
        return NULL;
    }
}

struct oftable *
next_matching_table(uint8_t bridge_id, int ofproto_n_tables, const struct oftable * table, uint8_t table_id){
    return (table_id == 0xff ?
      next_visible_table(bridge_id, ofproto_n_tables, (table - SGX_oftables[bridge_id]) + 1) :
      NULL);
}
