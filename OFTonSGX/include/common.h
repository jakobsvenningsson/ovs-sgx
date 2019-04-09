#ifndef _SGX_COMMON_H
#define _SGX_COMMON_H

#include <sgx_spinlock.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sgx_thread.h>

#define hotcall_ecall_myenclave_sample 0
#define hotcall_ecall_ofproto_init_tables 1
#define hotcall_ecall_readonly_set 2
#define hotcall_ecall_istable_readonly 3
#define hotcall_ecall_cls_rule_init 4
#define hotcall_ecall_cr_rule_overlaps 5
#define hotcall_ecall_cls_rule_destroy 6
#define hotcall_ecall_cls_rule_hash 7
#define hotcall_ecall_cls_rule_equal 8
#define hotcall_ecall_classifier_replace 9
#define hotcall_ecall_rule_get_flags 10
#define hotcall_ecall_cls_count 11
#define hotcall_ecall_eviction_fields_enable 12
#define hotcall_ecall_evg_group_resize 13
#define hotcall_ecall_evg_add_rule 14
#define hotcall_ecall_evg_remove_rule 15
#define hotcall_ecall_cls_remove 16
#define hotcall_ecall_choose_rule_to_evict 17
#define hotcall_ecall_table_mflows 18
#define hotcall_ecall_choose_rule_to_evict_p 19
#define hotcall_ecall_minimatch_expand 20
#define hotcall_ecall_cr_priority 21
#define hotcall_ecall_cls_find_match_exactly 22
#define hotcall_ecall_femt_ccfe_c 23
#define hotcall_ecall_femt_ccfe_r 24
#define hotcall_ecall_femt_c 25
#define hotcall_ecall_femt_r 26
#define hotcall_ecall_oftable_enable_eviction 27
#define hotcall_ecall_oftable_disable_eviction 28
#define hotcall_ecall_ccfe_c 29
#define hotcall_ecall_ccfe_r 30
#define hotcall_ecall_table_mflows_set 31
#define hotcall_ecall_ofproto_destroy 32
#define hotcall_ecall_total_rules 33
#define hotcall_ecall_table_name 34
#define hotcall_ecall_collect_ofmonitor_util_c 35
#define hotcall_ecall_collect_ofmonitor_util_r 36
#define hotcall_ecall_cls_rule_is_loose_match 37
#define hotcall_ecall_fet_ccfes_c 38
#define hotcall_ecall_fet_ccfes_r 39
#define hotcall_ecall_fet_ccfe_c 40
#define hotcall_ecall_fet_ccfe_r 41
#define hotcall_ecall_cls_lookup 42
#define hotcall_ecall_cls_rule_priority 43
#define hotcall_ecall_desfet_ccfes_c 44
#define hotcall_ecall_desfet_ccfes_r 45
#define hotcall_ecall_cls_rule_format 46
#define hotcall_ecall_miniflow_expand 47
#define hotcall_ecall_rule_calculate_tag 48
//#define hotcall_ecall_sgx_table_dpif 49
#define hotcall_ecall_table_update_taggable 50
#define hotcall_ecall_is_sgx_other_table 51
#define hotcall_ecall_rule_calculate_tag_s 52
#define hotcall_ecall_hidden_tables_check 53
#define hotcall_ecall_oftable_set_name 54
#define hotcall_ecall_minimask_get_vid_mask 55
#define hotcall_ecall_miniflow_get_vid 56
#define hotcall_ecall_ofproto_get_vlan_c 57
#define hotcall_ecall_ofproto_get_vlan_r 58

#define hotcall_ecall_get_cls_rules 59
#define hotcall_ecall_get_cls_rules_and_enable_eviction 60
#define hotcall_ecall_eviction_group_add_rules 61
#define hotcall_ecall_ofproto_get_vlan_usage 62
#define hotcall_ecall_ofproto_flush 63
#define hotcall_ecall_ofproto_evict 64
//#define hotcall_ecall_destroy_rule_if_overlaps 59
//#define hotcall_ecall_get_rule_to_evict_if_neccesary 60
//#define hotcall_ecall_miniflow_expand_and_tag 61
//#define hotcall_ecall_allocate_cls_rule_if_not_read_only 62
//#define hotcall_ecall_classifier_replace_if_modifiable 63
//#define hotcall_ecall_ofproto_configure_table 64

//#define hotcall_ecall_table_dpif_init 65


/*typedef struct {
  int n_args;
  void *arg1;
  void *arg2;
  void *arg3;
  void *arg4;
  void *arg5;
  void *arg6;
  void *arg7;
  void *arg8;
  void *arg9;
  void *arg10;

} argument_list;*/


typedef struct {
    int n_args;
    void *args[12];
} argument_list;


typedef struct {
  size_t allocated_size;
  void *val;
} return_value;

typedef struct {
  sgx_thread_mutex_t mutex;
  sgx_spinlock_t spinlock;
  sgx_thread_cond_t cond;
  bool run;
  bool running_function;
  bool is_done;
  bool sleeping;
  int timeout_counter;
  int function;
  argument_list *args;
  void *ret;
} async_ecall;

#endif
