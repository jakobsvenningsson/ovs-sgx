
#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function dump_flows() {
  local N_FLOWS=$1
  for j in `seq 0 100`; do
    ovs-ofctl add-flow br0 priority=$j,actions=drop
  done
  for i in `seq 0 $N_FLOWS`; do
    ovs-ofctl dump-flows br0 > /dev/null
  done

}

function benchmark_dump_flows() {
  local ITERATIONS=$1
  local TARGETS=$(get_targets ${@:2})
  for target in ${TARGETS[@]}; do
    echo "FLAGS = $target"
    prepare
    compile "BENCHMARK_DUMP_FLOWS" $target
    startup "dump_flows_$target"
    dump_flows $ITERATIONS
    sleep 2
    cleanup
  done
}

