#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function mod_flow_strict() {
  local N_FLOWS=$1
  for i in `seq 0 $N_FLOWS`; do
    ovs-ofctl add-flow br0 in_port=2,actions=drop
    ovs-ofctl mod-flows br0 in_port=2,action=normal --strict
    ovs-ofctl del-flows br0 "in_port=2" --strict
  done
}

function benchmark_mod_flow_strict() {
  local ITERATIONS=$2
  local TARGETS=$(get_targets $1)
  for target in ${TARGETS[@]}; do
    echo "FLAGS = $target"
    prepare
    compile "BENCHMARK_MOD_FLOW_STRICT" $target
    startup "mod_flow_strict_$target"
    mod_flow_strict $ITERATIONS
    sleep 2
    cleanup
  done
}

