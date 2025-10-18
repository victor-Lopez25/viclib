#!/bin/bash

mkdir -p bin

cd bin
gcc -Wall -Wextra -Wmissing-field-initializers ../src/test.c -o test.exe
gcc -Wall -Wextra -Wmissing-field-initializers ../src/test_mem.c -o test_mem.exe
cd ..