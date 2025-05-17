@echo off
setlocal ENABLEEXTENSIONS

rem ----------- Configuration -----------
set COMPILER=gcc
set SALAM_OUTPUT=s

rem ----------- Compilation -----------
echo Compiling C files listed in c_files.txt...
%COMPILER% -Wno-unused-parameter -g -Wall -Wextra -pedantic -I. @c_files.txt -o %SALAM_OUTPUT% 2> compile_warnings.txt

type compile_warnings.txt

if errorlevel 1 (
    echo Compilation and linking failed!
    exit /b 1
)

rem ----------- Run Program -----------
echo Running program with Valgrind to check for memory leaks and issues...
valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes .\%SALAM_OUTPUT% input.salam
set VALGRIND_EXIT_CODE=%ERRORLEVEL%

if not "%VALGRIND_EXIT_CODE%"=="0" (
    echo Program execution failed (exit code %VALGRIND_EXIT_CODE%). Skipping JSON beautification.
    exit /b %VALGRIND_EXIT_CODE%
)
