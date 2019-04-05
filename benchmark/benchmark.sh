#!/bin/bash

C_FLAGS=${@:3}
ITERATIONS=$2
BENCHMARK=$1

function print_help() {
  echo "usage: ./benchmark.sh [test-case] [iterations] [ovs-version]"
  echo "example: ./benchmark.sh add-flow 100 SGX"
}

if [ -z "$BENCHMARK" ]; then
  print_help
  exit
fi

echo "Benchmarking $BENCHMARK for $ITERATIONS iterations with flags $C_FLAGS"

case $BENCHMARK in
  add-flow)
    source ./scripts/add_flow.sh
    benchmark_add_flow $C_FLAGS $ITERATIONS
    echo $?
    ;;
  del-flow-strict)
    source ./scripts/del_flow_strict.sh
    benchmark_del_flow_strict $C_FLAGS $ITERATIONS
    echo $?
    ;;
  help)
    print_help
    ;;
  *)
    echo "Unknown benchmark."
    echo ${@:2} 
  ;;
esac
