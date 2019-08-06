#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function add_flows() {
  local N_FLOWS=$1
  echo "Adding $N_FLOWS flows..."
  for i in `seq 0 $N_FLOWS`; do
    ovs-ofctl add-flow br0 priority=$i,in_port=20,actions=drop
    #ovs-ofctl del-flows br0
    #sleep 0.1
  done

}

function benchmark_add_flow() {
  local ITERATIONS=$1
  local TARGETS=$(get_targets ${@:2})
  for target in ${TARGETS[@]}; do
      prepare
      compile "BENCHMARK_ADD_FLOW" $target
        for i in $(seq 0 19); do
            echo "FLAGS = $target"
            startup "add_flow_$target"
            add_flows $ITERATIONS
            sleep 1
            cleanup
        done
  done
}
