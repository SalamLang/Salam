@echo off

REM Set the output executable name
set output=salam

REM List of source files
set sources=log.c file.c memory.c array.c parser.c parser_layout.c generator.c generator_layout.c generator_layout_style.c generator_identifier.c string_buffer.c validator.c hashmap.c hashmap_custom.c array_custom.c lexer.c ast.c ast_layout.c ast_layout_style.c main.c

REM Compile each source file into an object file
for %%f in (%sources%) do (
    gcc -std=c11 -c %%f -o %%~nf.wino
)

REM Link all object files into the final executable
gcc -o %output% *.wino

REM Run the executable with the provided argument
%output% ..\example\test6.salam ..\out\
