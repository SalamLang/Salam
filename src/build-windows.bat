@echo off
setlocal enabledelayedexpansion

REM Set the output executable name
set output=salam

REM List of source files
set sources=log.c file.c memory.c downloader.c array.c parser.c parser_layout.c generator.c generator_layout.c generator_salam.c generator_layout_style.c generator_identifier.c string_buffer.c validator.c hashmap.c hashmap_custom.c array_custom.c lexer.c ast.c ast_layout.c ast_layout_style.c main.c

REM Ensure the output directory exists
if not exist "..\out" (
	echo Creating output directory...
	mkdir ..\out
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
%output% ..\example\test6.salam ..\out\
if errorlevel 1 (
	echo Error: Execution failed with exit code %errorlevel%
	exit /b %errorlevel%
)

echo Script completed successfully.
