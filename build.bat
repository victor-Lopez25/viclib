@echo off

set defines=-D_CRT_SECURE_NO_WARNINGS
set opts=-FC -GR- -EHa- -nologo -Zi -W4 -wd4201
set code=%cd%\src
set out=test.exe

IF NOT EXIST bin mkdir bin

pushd bin
cl %defines% %opts% %code%\test.c -Fe%out% /link -incremental:no -opt:ref

test.exe
popd