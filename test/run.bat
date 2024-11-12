@echo off

pushd ..
call build.bat || exit /b 1
popd

set OUTPUT_DIR=%1
set BOOST=%2
set CATCH=%3

if "%OUTPUT_DIR%"=="" set OUTPUT_DIR=%TEMP%

set CUT_CLANG=../cut-clang++.exe
set CUT_CL=../cut-cl.exe
set COMMON=-I. -I../inc -I%BOOST% -I%CATCH%/extras -I%CATCH%/src -D_WIN32_WINNT=0x0A00 -DBOOST_TIMER_ENABLE_DEPRECATED -DBOOST_ALL_NO_LIB -DBOOST_SYSTEM_USE_UTF8 -DBOOST_ASIO_NO_DEPRECATED -DWIN32_LEAN_AND_MEAN -DNOMINMAX -DDIRECT3D_VERSION=0x0900
set OPTIONS_CLANG=-c -w -std=c++2c %COMMON% -o %OUTPUT_DIR%
set OPTIONS_CL=/nologo /c /w /EHsc /std:c++latest %COMMON% /Fo%OUTPUT_DIR%

if "%VSINSTALLDIR%" == ""  PowerShell.exe -Command "Write-Host '|'File'|'clang++',' ms'|'cut-clang++ run#1',' ms'|'cut-clang++ run#2',' ms'|'"
if "%VSINSTALLDIR%" neq "" PowerShell.exe -Command "Write-Host '|'File'|'clang++',' ms'|'cl',' ms'|'cut-clang++ run#1',' ms'|'cut-clang++ run#2',' ms'|'cut-cl run#1',' ms'|'cut-cl run#2',' ms'|'"
                           PowerShell.exe -Command "Write-Host            '|' '|'"

                    call :benchmark import_std        import_std.cpp
                    call :benchmark import_windows    import_windows.cpp
if "%CATCH%" neq "" call :benchmark import_catch      import_catch.cpp
if "%CATCH%" neq "" call :benchmark catch_evt         %CATCH%\examples\210-Evt-EventListeners.cpp
if "%BOOST%" neq "" call :benchmark import_boost      import_boost.cpp
if "%BOOST%" neq "" call :benchmark stacktrace_test   %BOOST%\libs\stacktrace\test\test.cpp
if "%BOOST%" neq "" call :benchmark random_snips      %BOOST%\libs\multiprecision\example\random_snips.cpp
if "%BOOST%" neq "" call :benchmark filtering_example %BOOST%\libs\test\example\filtering_example.cpp
                    call :benchmark cut!              ../src/main.cpp

exit /b 0

:benchmark
call :compile %1 %2
PowerShell.exe -Command "Write-Host"
exit /b 0

:compile
PowerShell.exe -Command "Write-Host -NoNewLine '|'%1'|'"
                           PowerShell.exe -Command "Write-Host -NoNewLine ([math]::round((Measure-Command { &'%CLANG%' %OPTIONS_CLANG%/%1.obj %2 }).TotalMilliseconds))'|';"
if "%VSINSTALLDIR%" neq "" PowerShell.exe -Command "Write-Host -NoNewLine ([math]::round((Measure-Command { &cl        %OPTIONS_CL%/%1.obj    %2 }).TotalMilliseconds))'|';"

set CUT=%CUT_CLANG%
set OPTIONS=%OPTIONS_CLANG%
call :with_cache %1 %2

set CUT=%CUT_CL%
set OPTIONS=%OPTIONS_CL%
if "%VSINSTALLDIR%" neq "" call :with_cache %1 %2

exit /b 0

:with_cache
PowerShell.exe -Command "$NEW_CACHE = Get-Date -Format o; Write-Host -NoNewLine ([math]::round((Measure-Command { &%CUT% %OPTIONS%/%1.obj %2 """-DNEW_CACHE=$NEW_CACHE""" }).TotalMilliseconds))'|'; Write-Host -NoNewLine ([math]::round((Measure-Command { &%CUT% %OPTIONS%/%1.obj %2 """-DNEW_CACHE=$NEW_CACHE""" }).TotalMilliseconds))'|'"
exit /b 0
