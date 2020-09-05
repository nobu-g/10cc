#!/usr/bin/env bash

# compile tmp.s and print exit status of tmp
# $ cat tmp.s | ./exasm.sh
# or
# $ exasm.sh tmp.s

set -u

if [[ -p /dev/stdin ]]; then
  file=/dev/stdin
else
  file=$1
fi

tmp_dir=$(mktemp -d)
cc -c ${file} -o ${tmp_dir}/tmp.o
cc -static -o ${tmp_dir}/tmp ${tmp_dir}/tmp.o
${tmp_dir}/tmp
echo $?
