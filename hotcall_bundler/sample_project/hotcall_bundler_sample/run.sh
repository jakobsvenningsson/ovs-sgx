#!/bin/bash

source /opt/intel/sgxsdk/environment

cset shield --cpu 2,3
cset shield --kthread on
#cset shield -e source /opt/intel/sgxsdk/environment
cset shield -e ./sample -- $1
cset shield --reset
