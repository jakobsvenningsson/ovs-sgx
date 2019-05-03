#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function add_flows() {
  local N_FLOWS=$1
  echo "Adding $N_FLOWS flows..."
  for i in `seq 0 $N_FLOWS`; do
    ovs-ofctl add-flow br0 priority=$1,actions=drop
    #sleep 0.1
  done

}

function benchmark_add_flow() {
  local ITERATIONS=$1
  local TARGETS=$(get_targets ${@:2})
  for target in ${TARGETS[@]}; do
    echo "FLAGS = $target"
    prepare
    compile "BENCHMARK_ADD_FLOW" $target
    startup "add_flow_$target"
    add_flows $ITERATIONS
    sleep 2
    cleanup
  done
}

