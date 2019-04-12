#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function del_flow_loose() {
  local N_FLOWS=$1
  for i in `seq 1 10`; do 
    ovs-ofctl add-flow br0 in_port=$i,priority=10,actions=drop
  done
  for i in `seq 0 $N_FLOWS`; do
    ovs-ofctl del-flows br0 "in_port=2"
    ovs-ofctl add-flow br0 in_port=2,priority=10,actions=drop
    if [[ $(($i % 300)) == 0 ]]; then
      sleep 0.2
    fi
  done
}

function benchmark_del_flow_loose() {
  local ITERATIONS=$1
  local TARGETS=$(get_targets ${@:2})
  for target in ${TARGETS[@]}; do
    echo "FLAGS = $target"
    prepare
    compile "BENCHMARK_DEL_FLOW_LOOSE" $target
    startup "del_flow_loose_$target"
    del_flow_loose $ITERATIONS
    sleep 2
    cleanup
  done
}

