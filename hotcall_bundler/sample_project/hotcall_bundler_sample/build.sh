#!/bin/bash

source /opt/intel/sgxsdk/environment

make -C ../../src/trusted

make -C ../../src/untrusted -j4 

make 
