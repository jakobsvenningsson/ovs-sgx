#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function del_flow_strict() {
  local N_FLOWS=$1
  for i in `seq 0 $N_FLOWS`; do
    ovs-ofctl add-flow br0 in_port=2,actions=drop
    ovs-ofctl del-flows br0 "in_port=2" --strict
  done
}

function benchmark_del_flow_strict() {
  local ITERATIONS=$2
  local TARGETS=$(get_targets $1)
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

