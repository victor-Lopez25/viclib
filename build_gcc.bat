@echo off

if not exist bin mkdir bin

set code=%cd%\src
set out=test.exe
pushd bin
gcc -Wall -Wextra %code%\test.c -o test.exe
popd