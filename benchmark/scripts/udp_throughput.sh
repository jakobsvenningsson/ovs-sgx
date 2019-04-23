#!/bin/bash

source $HOME/ovs-sgx/benchmark/scripts/common.sh

function udp_throughput() {

  NETWORK_INTERFACE="wlp4s0"
  CLIENT_IP="192.168.1.20"
  SERVER_IP="192.168.1.21"
  TARGET=$2

  setup_testbed $NETWORK_INTERFACE $CLIENT_IP $SERVER_IP $TARGET

  sudo -u jakob sshpass -p 'pw123' scp $HOME/ovs-sgx/benchmark/scripts/udp_echo.py server@$SERVER_IP:~/
  sudo -u jakob sshpass -p "pw123" ssh server@$SERVER_IP "iperf -u -s < /dev/null > /tmp/mylogfile 2>&1 &"

  sudo -u jakob sshpass -p "pw123" ssh client@$CLIENT_IP "iperf -u -c ${SERVER_IP} -b 1000m -i 1 -t 30 > udp_throughput_${TARGET}"
  sudo -u jakob sshpass -p 'pw123' scp client@$CLIENT_IP:~/udp_throughput_${TARGET} $HOME/ovs-sgx/benchmark/data/tmp

  cleanup_testbed
}

function benchmark_udp_throughput() {
  local ITERATIONS=$1
  local TARGETS=$(get_targets ${@:2})
  for target in ${TARGETS[@]}; do
    echo "FLAGS = $target"
    prepare
    TLS_FLAG=""
    if [ $target != "BASELINE" ]; done
        TLS_FLAG="-D SGX_TLS"
    fi
    compile $TLS_FLAG $target
    startup "/dev/null"
    udp_throughput $ITERATIONS $target
    sleep 2
    cleanup
  done
}
