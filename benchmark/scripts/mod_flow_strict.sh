#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function mod_flow_strict() {

  for i in `seq 0 50`; do
    ovs-ofctl add-flow br0 "table=0,priority=${i},in_port=${i},actions=drop"
  done

  local N_FLOWS=$1
  for i in `seq 0 $N_FLOWS`; do
    ovs-ofctl add-flow br0 "table=0,priority=1,in_port=2,actions=drop"
    ovs-ofctl mod-flows br0 "table=0,priority=1,in_port=2,actions=normal" --strict
    ovs-ofctl del-flows br0 "table=0,priority=1,in_port=2" --strict
  done
}

#function mod_flow_strict() {
#  N_FLOWS=$1
#  for i in `seq 1 10`; do 
#    ovs-ofctl add-flow br0 in_port=$i,priority=10,actions=drop
#  done
#  for i in `seq 0 $N_FLOWS`; do
#    ovs-ofctl del-flows br0 "in_port=2" --strict
#    ovs-ofctl add-flow br0 in_port=2,priority=10,actions=drop
#    if [[ $(V($i % 300)) == 0 ]]; then
#      sleep 0.2
#    fi
#  done
#}

function benchmark_mod_flow_strict() {
  local ITERATIONS=$1
  local TARGETS=$(get_targets ${@:2})
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

