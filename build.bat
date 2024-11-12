@echo off

chcp 1250 > nul

for /F "tokens=1,2*" %%i in ('reg query "HKLM\SOFTWARE\WOW6432Node\LLVM\LLVM" /v ""') DO (
	if "%%i"=="(Default)" (
		SET LLVM=%%k
	)
)

if "%LLVM%"=="" exit /B 1

set CLANG="%LLVM%\bin\clang++.exe"

%CLANG% -std=c++2c -O2 src/main.cpp -DNDEBUG -DLLVM_DIR="\"%LLVM:\=/%\"" -Iinc -o cut-clang++.exe || exit /b 1
xcopy /Q /-I /Y cut-clang++.exe cut-cl.exe > nul || exit /b 1
