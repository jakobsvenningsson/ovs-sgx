#!/bin/bash


function setup_testbed() {

    local NETWORK_INTERFACE=$1
    local CLIENT_IP=$2
    local SERVER_IP=$3
    local TARGET=$4

    # Setup virtual interfaces
    ip tuntap add mode tap vport1
    ip tuntap add mode tap vport2

    ifconfig vport1 up
    ifconfig vport2 up

    ovs-vsctl add-port br0 $NETWORK_INTERFACE -- add-port br0 vport1 -- add-port br0 vport2

    ovs-vsctl set-ssl $HOME/ovs-sgx/ssl/ovs.key $HOME/ovs-sgx/ssl/ovs.crt $HOME/ovs-sgx/ssl/rootCA.crt
    ovs-vsctl set-controller br0 ssl:127.0.0.1:6633

    ifconfig $NETWORK_INTERFACE 0
    dhclient br0

    sudo -u jakob VBoxManage modifyvm CLIENT --nic1 bridged --bridgeadapter1 vport1
    sudo -u jakob VBoxManage modifyvm SERVER --nic1 bridged --bridgeadapter1 vport2

    # start server
    echo "Starting server VM."
    sudo -u jakob VBoxManage startvm SERVER --type headless
    echo "Trying to connect to server VM."
    until sudo -u jakob sshpass -p 'pw123' ssh server@$SERVER_IP "exit"; do
        sleep 3
    done
    echo "Connected."

    # start client
    echo "Starting client VM."
    sudo -u jakob VBoxManage startvm CLIENT --type headless
    echo "Trying to connect to client VM."
    until sudo -u jakob sshpass -p 'pw123' ssh client@$CLIENT_IP "exit"; do
        sleep 3
    done
    echo "Connected."

}

function cleanup_testbed() {
    sudo -u jakob VBoxManage controlvm CLIENT poweroff
    sudo -u jakob VBoxManage controlvm SERVER poweroff

    ip link delete vport1
    ip link delete vport2

    pkill -9 SDN_Controller
    pkill -9 ryu-manager
}

function start_sdn_controller_learning() {
    local CUR_DIR=$(pwd)
    cd $HOME/ovs-sgx/SDN_Controller
    make run_controller_not_learning_switch
    cd $CUR_DIR
}

function start_sdn_controller_not_learning() {
    #local CUR_DIR=$(pwd)
    #cd $HOME/ovs-sgx/SDN_Controller
    make -C $HOME/ovs-sgx/SDN_Controller run_controller_not_learning_switch
    #cd $CUR_DIR
}

function start_ca_authority() {
    local CUR_DIR=$(pwd)
    cd $HOME/ovs-sgx/CA_server
    make
    ./CA_server &
    cd $CUR_DIR
}



function join_by { local IFS="$1"; shift; echo "$*"; }

function get_targets() {
  local TARGETS=()
  if [ "$1" = "ALL_FLOW" ]; then
    TARGETS=("BASELINE" "SGX" "BATCHING" "OPTIMIZED" "HOTCALL" "HOTCALL+OPTIMIZED")
  elif [ "$1" = "ALL_TLS" ]; then
    TARGETS=("BASELINE" "SGX" "HOTCALL")
  else
    TARGETS=("$*")
  fi
  echo ${TARGETS[@]}
}

function get_compile_flags() {
  local TARGET=$1
  local FLAGS=""
  case $TARGET in
    BASELINE)
      ;;
    SGX)
      FLAGS="-D SGX"
      ;;
    OPTIMIZED)
      FLAGS="-D SGX -D OPTIMIZED"
      ;;
    BATCHING)
      FLAGS="-D SGX -D BATCHING -D HOTCALL"
      ;;
    HOTCALL)
      FLAGS="-D SGX -D HOTCALL"
      ;;
    HOTCALL+OPTIMIZED)
      FLAGS="-D SGX -D HOTCALL -D OPTIMIZED"
      ;;
    *)
      echo "Unknown target."
      echo ${@:2}
    ;;
  esac
  echo "$FLAGS"
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
  echo "First $1"
  echo "Second $2"
  local TARGET=$2
  echo "compile ${TARGET} $1"
  local FLAGS=`get_compile_flags ${TARGET}`
  local C_FLAGS="-D ${1} ${FLAGS}"
  #local C_FLAGS=$@
  #for flag in $C_FLAGS; do
  #  FLAGS+="-D $flag "
  #  if [ $flag = "HOTCALL" ]; then
  #    FLAGS+="-D SGX"
  #  fi
  #done

  echo "Compiling OVS with flags $C_FLAGS"

  cd $HOME/ovs-sgx

  ./build.sh "$C_FLAGS" > /dev/null 2> $HOME/ovs-sgx/benchmark/logs/benchmark.log
  if [ $? == 0 ]; then
    echo "Failed to build project, status code $?."
    echo "Check benchmark.log for more information."
    exit
  fi
  echo $?
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

  OUTPUT_PATH="${HOME}/ovs-sgx/benchmark/data/tmp/"
  if [ $OUTPUT_FILE == "/dev/null" ]; then
      OUTPUT_PATH=""
  fi

  echo "Writing output to ${OUTPUT_PATH}${OUTPUT_FILE}"
  ovs-vswitchd --pidfile --log-file=$HOME/ovs-log --detach 1>$HOME/ovs-sgx/benchmark/logs/benchmark-stdout.log 2>$HOME/ovs-sgx/benchmark/logs/benchmark-stderr.log 5>${OUTPUT_PATH}${OUTPUT_FILE}
  echo "Adding bridge br0"
  ovs-vsctl add-br br0
  echo $?
}

function cleanup() {
  pkill -9 ovs
  pkill -9 SDN_Controller
  pkill -9 CA_server
}

function prepare() {
  cleanup
  source /opt/intel/sgxsdk/environment
  rm /usr/local/etc/openvswitch/conf.db
  ovsdb-tool create /usr/local/etc/openvswitch/conf.db $HOME/ovs-sgx/ovs/vswitchd/vswitch.ovsschema
  /sbin/modprobe openvswitch
}
