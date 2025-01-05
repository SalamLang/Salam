@echo off
setlocal enabledelayedexpansion

:: Check if emcc is installed
where emcc >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: emcc is not installed.
    echo Install from https://emscripten.org/docs/getting_started/downloads.html
    exit /b 1
)

:: Define variables
set OUTPUT_BASE=salam-wa
set EDITOR_DIR=..\Salam-Editor\
set MEMORY_FLAGS=-s ALLOW_MEMORY_GROWTH=1
set RUNTIME_FLAGS=-s EXIT_RUNTIME=0 -s NO_EXIT_RUNTIME=1
set COMMON_FLAGS=-s EXPORTED_FUNCTIONS=['_main'] -s TOTAL_STACK=8388608

:: Define source files
set sources=src\log.c src\file.c src\memory.c src\array.c src\parser.c src\parser_layout.c src\generator.c src\generator_layout.c src\downloader.c src\generator_salam.c src\generator_layout_style.c src\generator_identifier.c src\string_buffer.c src\validator.c src\validator_style.c src\hashmap.c src\hashmap_custom.c src\array_custom.c src\lexer.c src\ast.c src\ast_layout.c src\ast_layout_style.c src\main.c

:: Check debug mode
set DEBUG_FLAGS=
if "%~1" == "debug" (
    set DEBUG_FLAGS=-s VERBOSE=1 -s ASSERTIONS=2 -v
    echo Debug mode enabled.
) else (
    echo Debug mode not enabled.
)

:: Compile C files to WebAssembly
echo Compiling C files to WebAssembly...
emcc %sources% -o %OUTPUT_BASE%.html %MEMORY_FLAGS% %RUNTIME_FLAGS% %COMMON_FLAGS% %DEBUG_FLAGS%
if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b 1
)

:: Compilation success
echo Compilation successful. Output files:
echo   %OUTPUT_BASE%.html
echo   %OUTPUT_BASE%.js
echo   %OUTPUT_BASE%.wasm

:: Check if npx is available for transpiling
where npx >nul 2>&1
if %errorlevel% equ 0 (
    echo Transpiling JavaScript for older browsers...
    npx esbuild %OUTPUT_BASE%.js --outfile=%OUTPUT_BASE%.transpiled.js --minify=true --target=esnext
    if %errorlevel% equ 0 (
        move /y %OUTPUT_BASE%.transpiled.js %OUTPUT_BASE%.js >nul
    ) else (
        echo Warning: Babel transpiling failed. JavaScript was not transpiled.
    )
) else (
    echo Warning: npx command not found. JavaScript will not be transpiled for older browsers.
)

:: Copy output files to editor directory
if exist "%EDITOR_DIR%" (
    echo Copying output files to %EDITOR_DIR%
    copy /y %OUTPUT_BASE%.html "%EDITOR_DIR%"
    copy /y %OUTPUT_BASE%.js "%EDITOR_DIR%"
    copy /y %OUTPUT_BASE%.wasm "%EDITOR_DIR%"
    echo Files copied successfully.
) else (
    echo Directory %EDITOR_DIR% does not exist. Skipping copy.
)
