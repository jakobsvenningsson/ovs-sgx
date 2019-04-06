#!/bin/bash

function join_by { local IFS="$1"; shift; echo "$*"; }

function get_targets() {
  local TARGETS=()
  if [ "$1" = "ALL" ]; then
    TARGETS=("BASELINE" "SGX" "HOTCALL")
  else
    TARGETS=$*
  fi
  echo ${TARGETS[@]}
}

function create_csv_file() {
    local TARGETS=`get_targets $1`
    local FILE_NAME="$2.csv"
    local FILE_PATH=$HOME/ovs-sgx/benchmark/data

    rm ${FILE_PATH}/${FILE_NAME}
    touch "${FILE_PATH}/${FILE_NAME}"
    for target in ${TARGETS[@]}; do
      paste ${FILE_PATH}/${FILE_NAME} ${FILE_PATH}/tmp/$2_${target} -d "," > ${FILE_PATH}/tmp_file && mv ${FILE_PATH}/tmp_file ${FILE_PATH}/${FILE_NAME}
    done

    cut ${FILE_PATH}/${FILE_NAME} -f 2- -d "," > ${FILE_PATH}/tmp_file && mv ${FILE_PATH}/tmp_file ${FILE_PATH}/${FILE_NAME}
    echo `join_by , $TARGETS` | cat - ${FILE_PATH}/${FILE_NAME} > ${FILE_PATH}/tmp_file && mv ${FILE_PATH}/tmp_file ${FILE_PATH}/${FILE_NAME}
}

function compile() {
  local FLAGS=""
  local C_FLAGS=$@
  for flag in $C_FLAGS; do
    FLAGS+="-D $flag "
    if [ $flag = "HOTCALL" ]; then
      FLAGS+="-D SGX"
    fi
  done

  echo "Compiling OVS with flags $FLAGS"
  
  cd $HOME/ovs-sgx

  ./build.sh "$FLAGS" > /dev/null 2> $HOME/ovs-sgx/benchmark/logs/benchmark.log
  if [ $? -eq "0" ]; then
    echo "Failed to build project, status code $?."
    echo "Check benchmark.log for more information."  
    exit
  fi
  echo "Compilation successfull."

  cd $HOME/ovs-sgx
}

function startup() {
  OUTPUT_FILE=$1
  echo "Starting ovsdb-server"
  ovsdb-server --remote=punix:/usr/local/var/run/openvswitch/db.sock \
          --remote=db:Open_vSwitch,Open_vSwitch,manager_options \
          --private-key=db:Open_vSwitch,SSL,private_key \
          --certificate=db:Open_vSwitch,SSL,certificate \
          --bootstrap-ca-cert=db:Open_vSwitch,SSL,ca_cert \
          --pidfile --detach --log-file
  echo $?
  ovs-vsctl --no-wait init
  echo "Starting ovs-vswitch"
  ovs-vswitchd --pidfile --log-file --detach 5>$HOME/ovs-sgx/benchmark/data/tmp/$OUTPUT_FILE
  echo "Adding bridge br0"
  ovs-vsctl add-br br0
  echo $?
}

function cleanup() {
  pkill ovs
  pkill CA_server
}

function prepare() {
  cleanup
  source /opt/intel/sgxsdk/environment
  rm /usr/local/etc/openvswitch/conf.db
  ovsdb-tool create /usr/local/etc/openvswitch/conf.db $HOME/ovs-sgx/ovs/vswitchd/vswitch.ovsschema
  /sbin/modprobe openvswitch
}




