#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function udp_latency() {

  local NETWORK_INTERFACE="wlp4s0"
  local CLIENT_IP="192.168.1.20"
  local SERVER_IP="192.168.1.21"
  local TARGET=$2

  echo "Starting testbed"
  setup_testbed $NETWORK_INTERFACE $CLIENT_IP $SERVER_IP

  # Start echo server
  sudo -u jakob sshpass -p 'pw123' scp $HOME/ovs-sgx/benchmark/scripts/udp_echo.py server@$SERVER_IP:~/
  sudo -u jakob sshpass -p "pw123" ssh server@$SERVER_IP "python3 udp_echo.py < /dev/null > /tmp/mylogfile 2>&1 &"

  # Create traffic
  sudo -u jakob sshpass -p 'pw123' scp $HOME/ovs-sgx/benchmark/scripts/udp_latency.py client@$CLIENT_IP:~/
  sudo -u jakob sshpass -p "pw123" ssh client@$CLIENT_IP "python3 udp_latency.py > udp_latency_${TARGET}"
  sudo -u jakob sshpass -p 'pw123' scp client@$CLIENT_IP:~/udp_latency_${TARGET} $HOME/ovs-sgx/benchmark/data/tmp

  cleanup_testbed
}

function benchmark_udp_latency() {
  local ITERATIONS=$1
  local TARGETS=$(get_targets ${@:2})
  for target in ${TARGETS[@]}; do
    echo "FLAGS = $target"
    prepare
    #local TLS_FLAG="DUMMY"
    #if [ $target != "BASELINE" ]; then
    #    TLS_FLAG="SGX_TLS"
    #    start_ca_authority
    #fi
    #start_sdn_controller
    compile $target
    startup "/dev/null"
    udp_latency $ITERATIONS $target
    sleep 2
    cleanup
  done
}
