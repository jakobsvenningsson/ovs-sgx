# Use cases that I want to benchmark


## Flow table operations

1. Flow table modifications
    * Adding flow
        * Adding simple flows i.e. flows which does not require multiple lookups in flow tables.
        * Adding complex flows i.e. flows which does need to consult multiple flows tables.
    * Deleting flow
        * strict match
        * loose match
    * Modifying flow
2. Flow eviction
    * Change the flow eviction policy on a pre-populated flow table. This will require the eviction policy of all flows to be recalculated and this might also causes flows to be eviction.
    * eviction of rule, adding a flow which makes the total number of flows in the table exceed the limit which will trigger a eviction.
    * Disable eviction
    * Enable eviction
3. Print out flows (dump flows)
4. Test which triggers facet_check_consistency

## Routing

1. Handing of cache miss when rule exists in flow table.
2. Handling of cache miss when no rule exists in flow table and controller has to be consulted.
3. Check different traffic patterns. Should check both longer and shorter flows. Longer flows should cause more misses which requires more interaction with flow tables and controller.


## Startup time

1. How long does it take to start OvS (Time for SSL setup, flow table initializations etc.)
2. Creating and deletion of bridge

## Stats

I don't know exactly what to benchmark since my knowledge about ovs stats are limited at this point. (probably the handle_desc_stats_request and handle_flow_stats_request function).
