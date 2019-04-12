#!/bin/bash

TARGETS=${@:3}
ITERATIONS=$2
BENCHMARK=$1

BASE_PATH=$HOME/ovs-sgx/benchmark

function add_flow() {
  source ${BASE_PATH}/scripts/add_flow.sh
  benchmark_add_flow $ITERATIONS $TARGETS
  echo $?
  create_csv_file $TARGETS "add_flow"
}

function del_flow_strict() {
  source ${BASE_PATH}/scripts/del_flow_strict.sh
  benchmark_del_flow_strict $ITERATIONS $TARGETS 
  echo $?
  create_csv_file $TARGETS "del_flow_strict"
}

function del_flow_loose() {
  source ${BASE_PATH}/scripts/del_flow_loose.sh
  benchmark_del_flow_loose $ITERATIONS $TARGETS
  echo $?
  create_csv_file $TARGETS "del_flow_loose"
}


function mod_flow_strict() {
  source ${BASE_PATH}/scripts/mod_flow_strict.sh
  benchmark_mod_flow_strict $ITERATIONS $TARGETS
  echo $?
  create_csv_file $TARGETS "mod_flow_strict"
}

function mod_flow_loose() {
  source ${BASE_PATH}/scripts/mod_flow_loose.sh
  benchmark_mod_flow_loose $ITERATIONS $TARGETS
  echo $?
  create_csv_file $TARGETS "mod_flow_loose"
}

function dump_flows() {
  source ${BASE_PATH}/scripts/dump_flows.sh
  benchmark_dump_flows $ITERATIONS $TARGETS
  echo $?
  create_csv_file $TARGETS "dump_flows"
}

function print_help() {
  echo "usage: ./benchmark.sh [test-case] [iterations] [ovs-version]"
  echo "example: ./benchmark.sh add-flow 100 SGX"
}

if [ -z "$BENCHMARK" ]; then
  print_help
  exit
fi

echo "Benchmarking $BENCHMARK for $ITERATIONS iterations with flags $TARGETS"

source ./scripts/common.sh

case $BENCHMARK in
  add-flow)
    add_flow
    ;;
  del-flow-strict)
    del_flow_strict
    ;;
  del-flow-loose)
    del_flow_loose
    ;;
  mod-flow-strict)
    mod_flow_strict
    ;;
  mod-flow-loose)
    mod_flow_loose
    ;;
  dump-flows)
    dump_flows
    ;;
  all)
    add_flow
    del_flow_strict
    del_flow_loose
    mod_flow_strict
    mod_flow_loose
    ;;
  help)
    print_help
    ;;
  *)
    echo "Unknown benchmark."
    echo ${@:2} 
  ;;
esac
