#!/bin/bash

TARGETS=("" "SGX" "VANILLA")

for i in "${TARGETS[@]}"; do
  echo $i
done

echo "-----------------"

TARGETS2=$*

for i in "${TARGETS2[@]}"; do
  echo $i
done
