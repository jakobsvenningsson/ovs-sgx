#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function evict_rule_batch() {
  ovs-vsctl -- --id=@t0 create Flow_Table name=evict flow-limit=10 overflow-policy=evict -- set bridge br0 flow_tables:0=@t0
  local N_FLOWS=$1
  for i in `seq 0 $N_FLOWS`; do
    ovs-vsctl set Flow_Table evict flow-limit=10
    for i in `seq 0 10`; do
      ovs-ofctl add-flow br0 "table=0, in_port=${i},idle_timeout=${i},actions=drop"
    done
    ovs-vsctl set Flow_Table evict flow-limit=1
    ovs-ofctl del-flows br0
  done
}

function benchmark_evict_rule_batch() {
  local ITERATIONS=$1
  local TARGETS=$(get_targets ${@:2})
  for target in ${TARGETS[@]}; do
    echo "FLAGS = $target"
    prepare
    compile "BENCHMARK_EVICTION_BATCH" $target
    startup "evict_rule_batch_$target"
    evict_rule_batch $ITERATIONS
    sleep 2
    cleanup
  done
}

