@echo off

set defines=-D_CRT_SECURE_NO_WARNINGS
set opts=-FC -GR- -EHa- -nologo -Zi -W4 -wd4201
set code=%cd%\src

IF NOT EXIST bin mkdir bin

pushd bin
cl %defines% %opts% %code%\test.c -Fetest.exe /link -incremental:no -opt:ref
cl %defines% %opts% %code%\test_windows.c -Fetest_windows.exe /link -incremental:no -opt:ref kernel32.lib
cl %defines% %opts% %code%\test_mem.c -Fetest_mem.exe /link -incremental:no -opt:ref

::cl %opts% /EP %code%\macro_test.c -Femacro_test.exe
::macro_test.exe

test.exe
test_windows.exe
test_mem.exe
popd