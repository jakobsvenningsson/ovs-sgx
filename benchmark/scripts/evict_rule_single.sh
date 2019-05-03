#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function evict_rule_single() {
  ovs-vsctl -- --id=@t0 create Flow_Table name=evict flow-limit=5 overflow-policy=evict -- set bridge br0 flow_tables:0=@t0
  for i in `seq 0 5`; do
    ovs-ofctl add-flow br0 "table=0, in_port=${i},idle_timeout=${i},actions=drop"
  done
  local N_FLOWS=$1
  for i in `seq 0 $N_FLOWS`; do
    ovs-ofctl add-flow br0 "priority=$(($i + 5)),idle_timeout=$(($i + 4)),actions=drop"
  done

}

function benchmark_evict_rule_single() {
  local ITERATIONS=$1
  local TARGETS=$(get_targets ${@:2})
  for target in ${TARGETS[@]}; do
    echo "FLAGS = $target"
    prepare
    compile "BENCHMARK_ADD_FLOW" $target
    startup "evict_rule_single_$target"
    evict_rule_single $ITERATIONS
    sleep 2
    cleanup
  done
}

