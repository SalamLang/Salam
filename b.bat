@echo off
setlocal ENABLEEXTENSIONS
setlocal ENABLEDELAYEDEXPANSION

rem ----------- Configuration -----------
rem Choose: tcc or gcc
set COMPILER=gcc
set SALAM_OUTPUT=s
set C_FILES=c_files.txt

set CFLAGS_COMMON=
@REM set CFLAGS_COMMON=-Wno-unused-parameter -g -Wall -Wextra -pedantic
set INCLUDE_DIRS=-I. -Isrc

set CFLAGS=%CFLAGS_COMMON% %INCLUDE_DIRS%
set LDFLAGS=

set OBJ_FILES=

rem ----------- Compilation -----------
if /i "%COMPILER%"=="tcc" (
    for /f "usebackq delims=" %%a in ("c_files.txt") do (
        echo Compiling %%a ...
        %COMPILER% -c %CFLAGS% "%%a" -o "%%~na.o" 2> "compile_error.tmp"

        for %%b in (compile_error.tmp) do if %%~zb NEQ 0 (
            echo Compilation failed for %%a:
            type compile_error.tmp
            del compile_error.tmp
            exit /b 1
        )

        del compile_error.tmp
        set OBJ_FILES=!OBJ_FILES! %%~na.o
    )

    echo Linking...
    %COMPILER% -o %SALAM_OUTPUT% !OBJ_FILES! %LDFLAGS%
    if errorlevel 1 (
        echo Linking failed!
        exit /b 1
    )
) else if /i "%COMPILER%"=="gcc" (
    echo Compiling and linking with GCC...
    %COMPILER% %CFLAGS% @%C_FILES% -o %SALAM_OUTPUT% %LDFLAGS%
    if errorlevel 1 (
        echo Compilation or linking failed!
        exit /b 1
    )
) else (
    echo Unknown compiler: %COMPILER%
    exit /b 1
)

echo Compilation and linking successful.

rem ----------- Run Program -----------
echo Running program...
.\%SALAM_OUTPUT% input.salam
if errorlevel 1 (
    echo Program execution failed!
    exit /b 1
)
echo Program executed successfully.
