#!/bin/bash

mkdir -p bin

cd bin
gcc -Wall -Wextra ..\src\test.c -o test.exe
gcc -Wall -Wextra ..\src\test_mem.c -o test_mem.exe
cd ..