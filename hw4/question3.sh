#!/usr/bin/env bash

gcc -O2 -pthread -std=gnu11 matrix_mul_pthreads.c -o matmul

set -e
runs=50

./matmul 2 1 >/dev/null || { echo "compile test failed"; exit 1; }

for pow in {1..10}; do
    n=$((2**pow))
    printf "n=%d ...\n" "$n"
    ./matmul "$n" "$runs" > "${n}.out"
done
echo "all done"
