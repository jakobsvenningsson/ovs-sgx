1. oftable_enable_eviction - merge 3 first ecalls into 1. Then do a single ecall for the loop.
2. eviction_group_add_rule - merge 4 ecalls into 1.
3. ofproto_get_vlan_usage - merge ecalls into 1.
4. oftable_remove_rule - remove evg rule and cls rule in same ecall.
5. ofproto_flush__ - get all hashes computes inside ofoperation_complete in a single ecall.
6. destruct - do only a single ecall
7. rule_construct - merge 2 ecalls into 1.
8. tag_the_flow - merge 2 ecalls into 1.
9. handle_table_stats_request - remove ecall out from loop and do a single ecall outside instead. This can de done for both loops in thie function.
10.  collect_rules_loose - remove is_hidden from loop. Merge collect and read into single call.
11. handle_flow_stats_request - merge 2 ecalls into 1.
