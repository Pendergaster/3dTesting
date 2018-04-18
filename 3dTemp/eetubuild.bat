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

::echo %INCLUDE%
::echo %LIB%

:: log("%{}", array, 10);


set includes=-I"../include" -I"../Shared"
set game_includes=-I"../Shared"
set lib_path="../libraries/"
set libs=glfw3.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  Shell32.lib
SETLOCAL


IF /I "%1"=="build_engine" (
REM ECHO BUILDING_ENGINE
SET COMPILE_FLAGS=-Zi

SET BUILD_DIR=DebugBin
SET EXE_NAME=engine.exe

pushd DebugBin


REM -LD -> buildaa .dll -MD jälkee
REM nologo ei turhaa printtiä / /MD common runtime multithreaded   /   /link alottaa linkkaamisen / 
cl -Zi %COMPILE_FLAGS% /DEBUG ..\main.c ..\source\nuklear_inc.c ..\source\glad.c %includes% /MD /link %libs% -LIBPATH:../libraries 
popd
)



IF /I "%1"=="build_game" (
REM ECHO BUILDING_ENGINE
SET COMPILE_FLAGS=-Z7

SET BUILD_DIR=DebugBin
SET LIB_NAME=engine

pushd DebugBin


REM -LD -> buildaa .dll -MD jälkee
REM nologo ei turhaa printtiä / /MD common runtime multithreaded   /   /link alottaa linkkaamisen / 
cl -Zi -nologo %COMPILE_FLAGS% %game_includes% ..\game\game.c  /MD /LD /link  %libs% -LIBPATH:../libraries  
popd
)








ENDLOCAL
REM sama kuin EOF