#!/bin/bash

mkdir -p bin

cd bin
gcc -Wall -Wextra ..\src\test.c -o test.exe
cd ..