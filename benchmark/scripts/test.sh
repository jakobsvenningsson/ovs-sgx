#!/bin/bash

function foo {
  TARGETS=("SGX VANILLA" "SGX" "VANILLA")
  echo ${TARGETS[*]}
}

F=`foo`
:
for i in ${F[*]}; do
  echo $i
done

