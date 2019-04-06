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


COL1="1\n2\n3"
COL2="a\nb\nc"

paste -d "," <(echo -e "$COL1") #<(echo -e "$COL2")

function foo() {
  local X=("1" "2" "3")
  echo $X
}


XX=`foo`
echo ${#XX[@]}
Y=("a" "b")
for x in "${XX[@]}"; do
  echo $x
done

YY=("x" "y" "zyx")
echo ${YY[@]}

KEY="GRIS"
function bar() {
  local KEY="HELLO"
}
bar
echo $KEY
