#!/bin/bash


GLOBAL=""
function foo {
  echo $@
  local TARGETS=("1VANILLA" "SGX" "VANILLA")
  GLOBAL=($@)
}

foo $@

for i in "${GLOBAL[@]}"; do
  echo $i
done

