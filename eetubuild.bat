@echo off
REM pistä vs variablet   echo %VS140COMNTOOLS%   -> /../../VC    ->   vs vrsall.bat
REM kerro mitä buildataan ja buildaa         cl käytä compiler
REM käynnistys
REM /Z7   debug flag            /O2     release


REM CL [option...] file... [option | file]... [lib...] [@command-file] [/link link-opt...] 

REM cl /z7          main.c            


REM %1 (- 9) arguments

REM SETLOCAL lokaaliksi variableksi
REM access variable ECHO %COMPILE_FLAGS%



if not defined DEV_ENV (
    CALL "%VS140COMNTOOLS%/../../VC/vcvarsall.bat" x64
)
set DEV_ENV=???


SETLOCAL


IF /I "%1"=="build_engine" (
SET COMPILE_FLAGS=-Z7
SET LINK=
SET BUILD_DIR=DebugBin\
SET EXE_NAME=engine

pushd %BUILD_DIR% 
echo %cd%
cl %COMPILE_FLAGS% ..\main.c /link %LINK% 
popd
)








ENDLOCAL
REM sama kuin EOF