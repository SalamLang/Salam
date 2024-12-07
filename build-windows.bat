@echo off
setlocal enabledelayedexpansion

REM Set the output executable name
set output=salam.exe

REM List of source files
set sources=src/log.c src/file.c src/memory.c src/downloader.c src/array.c src/parser.c src/parser_layout.c src/generator.c src/generator_layout.c src/generator_salam.c src/generator_layout_style.c src/generator_identifier.c src/string_buffer.c src/validator.c src/validator_style.c src/hashmap.c src/hashmap_custom.c src/array_custom.c src/lexer.c src/ast.c src/ast_layout.c src/ast_layout_style.c src/main.c

REM Ensure the output directory exists
if not exist "out" (
	echo Creating output directory...
	mkdir out
	if errorlevel 1 (
		echo Error: Failed to create output directory.
		exit /b %errorlevel%
	)
)

REM Compile each source file into an object file
for %%f in (%sources%) do (
	echo Compiling %%f...
	gcc -std=c11 -c %%f -o %%~nf.wino -lws2_32
	if !errorlevel! neq 0 (
		echo Error: Compilation failed for %%f with exit code !errorlevel!
		exit /b !errorlevel!
	)
)

REM Link all object files into the final executable
echo Linking object files...
gcc -o %output% *.wino -lws2_32
if errorlevel 1 (
	echo Error: Linking failed with exit code %errorlevel%
	exit /b %errorlevel%
)

REM Run the executable with the provided argument
echo Running the executable...
%output% example\test6.salam out\
if errorlevel 1 (
	echo Error: Execution failed with exit code %errorlevel%
	exit /b %errorlevel%
)

echo Script completed successfully.
