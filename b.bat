@echo off
REM Variables
set INPUT_FILE=salam.c
set OUTPUT_FILE=s
set EXAMPLE_FILE=examples/input.salam

REM Check if an argument is provided
if "%~1" neq "" (
    set EXAMPLE_FILE=%~1
)

REM Clear screen
cls

REM Delete file
if exist "%OUTPUT_FILE%" (
    del "%OUTPUT_FILE%"
)

REM Check for efence library
REM where /R "C:\Program Files" efence.dll >nul 2>&1
REM if %errorlevel% neq 0 (
REM     echo efence library is missing
REM     echo Install efence - Electric Fence Malloc Debugger
REM     exit /b 1
REM )

REM Compile
REM gcc -g -ggdb -g -o "%OUTPUT_FILE%" "%INPUT_FILE%"
REM gcc -g -fsanitize=undefined,address -Walloca -o "%OUTPUT_FILE%" "%INPUT_FILE%" -lefence
gcc -g -fsanitize=undefined,address -Walloca -o "%OUTPUT_FILE%" "%INPUT_FILE%"
if %errorlevel% neq 0 (
    echo Compilation failed!
    exit /b 1
)

REM Check if compilation was successful
if exist "%OUTPUT_FILE%" (
    "%OUTPUT_FILE%" "%EXAMPLE_FILE%"
    REM set LSAN_OPTIONS=verbosity=1:log_threads=1
    REM "%OUTPUT_FILE%" "%EXAMPLE_FILE%"
    REM set ASAN_OPTIONS=detect_leaks=1
    REM "%OUTPUT_FILE%" "%EXAMPLE_FILE%"
    REM set LSAN_OPTIONS=verbosity=1:log_threads=1:detect_leaks=1
    REM "%OUTPUT_FILE%" "%EXAMPLE_FILE%"
) else (
    echo Compilation failed!
)
