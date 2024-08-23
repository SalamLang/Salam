gcc  -c log.c -o log.win.o
gcc  -c file.c -o file.win.o
gcc  -c memory.c -o memory.win.o
gcc  -c array.c -o array.win.o
gcc  -c array_custom.c -o array_custom.win.o
gcc  -c parser.c -o parser.win.o
gcc  -c generator.c -o generator.win.o
gcc  -c generator_layout.c -o generator_layout.win.o
gcc  -c generator_identifier.c -o generator_identifier.win.o
gcc  -c string.c -o string.win.o
gcc  -c validator.c -o validator.win.o
gcc  -c hashmap.c -o hashmap.win.o
gcc  -c hashmap_custom.c -o hashmap_custom.win.o
gcc  -c lexer.c -o lexer.win.o
gcc  -c ast.c -o ast.win.o
gcc  -c ast_layout.c -o ast_layout.win.o
gcc  -c main.c -o main.win.o
gcc  -o main.exe log.win.o file.win.o memory.win.o array.win.o array_custom.win.o parser.win.o generator.win.o generator_layout.win.o generator_identifier.win.o string.win.o validator.win.o hashmap.win.o hashmap_custom.win.o lexer.win.o ast.win.o ast_layout.win.o main.win.o
