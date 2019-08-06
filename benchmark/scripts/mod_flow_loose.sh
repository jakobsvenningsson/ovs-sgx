#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function mod_flow_loose() {
  local N_FLOWS=$1

  for i in `seq 0 24`; do
    ovs-ofctl add-flow br0 in_port=2,priority=$i,actions=drop
  done

  for i in `seq 0 $N_FLOWS`; do
    if [[ $(( $i % 2 )) == 0 ]]; then
      ovs-ofctl mod-flows br0 "in_port=2",actions=normal
    else
      ovs-ofctl mod-flows br0 "in_port=2",actions=drop
    fi
  done
}

function benchmark_mod_flow_loose() {
  local ITERATIONS=$1
  local TARGETS=$(get_targets ${@:2})
  for target in ${TARGETS[@]}; do
    echo "FLAGS = $target"
    prepare
    compile "BENCHMARK_MOD_FLOW_LOOSE" $target
    for i in $(seq 0 19); do
        startup "mod_flow_loose_$target"
        mod_flow_loose $ITERATIONS
    done
    cleanup
  done
}
