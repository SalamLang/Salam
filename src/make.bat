gcc  -c log.c -o log.o
gcc  -c file.c -o file.o
gcc  -c memory.c -o memory.o
gcc  -c array.c -o array.o
gcc  -c parser.c -o parser.o
gcc  -c parser_layout.c -o parser_layout.o
gcc  -c generator.c -o generator.o
gcc  -c generator_layout.c -o generator_layout.o
gcc  -c generator_layout_style.c -o generator_layout_style.o
gcc  -c generator_identifier.c -o generator_identifier.o
gcc  -c string.c -o string.o
gcc  -c validator.c -o validator.o
gcc  -c hashmap.c -o hashmap.o
gcc  -c hashmap_custom.c -o hashmap_custom.o
gcc  -c array_custom.c -o array_custom.o
gcc  -c lexer.c -o lexer.o
gcc  -c ast.c -o ast.o
gcc  -c ast_layout.c -o ast_layout.o
gcc  -c ast_layout_style.c -o ast_layout_style.o
gcc  -c main.c -o main.o
gcc  -o main log.o file.o memory.o array.o parser.o parser_layout.o generator.o generator_layout.o generator_layout_style.o generator_identifier.o string.o validator.o hashmap.o hashmap_custom.o array_custom.o lexer.o ast.o ast_layout.o ast_layout_style.o main.o
main ../example/test6.salam