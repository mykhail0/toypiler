#!/bin/bash

for n in $(seq 1 10); do
    filename=test${n}
    <tests/"${filename}".in valgrind --leak-check=full -q build/compile | valgrind --leak-check=full -q build/interpret >tests/"${filename}".myout
    if diff --color=auto tests/"${filename}".myout tests/"${filename}".out; then
        echo "${filename}: Ok"
        rm tests/"${filename}".myout
    else
        echo "${filename}: Failed"
    fi
done
