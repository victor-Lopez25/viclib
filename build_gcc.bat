@echo off

if not exist bin mkdir bin

set code=%cd%\src
set out=test.exe
pushd bin
gcc -g -Wall -Wextra %code%\test.c -o test.exe
gcc -g -Wall -Wextra %code%\test_windows.c -o test_windows.exe -l kernel32
gcc -g -Wall -Wextra %code%\test_mem.c -o test_mem.exe

::test_windows.exe
popd