#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function del_flow_strict() {
  N_FLOWS=$1
  for i in `seq 1 50`; do 
    ovs-ofctl add-flow br0 "table=0,in_port=$i,priority=10,actions=drop"
  done
  for i in `seq 0 $N_FLOWS`; do
    ovs-ofctl del-flows br0 "table=0,in_port=2,priority=2" --strict
    ovs-ofctl add-flow br0 "table=0,in_port=2,priority=2,actions=drop"
  done
}

function benchmark_del_flow_strict() {
  local ITERATIONS=$1
  local TARGETS=$(get_targets ${@:2})
  for target in ${TARGETS[@]}; do
    echo "FLAGS = $target"
    prepare
    compile "BENCHMARK_DEL_FLOW_STRICT" $target
    startup "del_flow_strict_$target"
    del_flow_strict $ITERATIONS
    sleep 2
    cleanup
  done
}

