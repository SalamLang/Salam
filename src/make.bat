gcc  -c log.c -o log.wino
gcc  -c file.c -o file.wino
gcc  -c memory.c -o memory.wino
gcc  -c array.c -o array.wino
gcc  -c parser.c -o parser.wino
gcc  -c parser_layout.c -o parser_layout.wino
gcc  -c generator.c -o generator.wino
gcc  -c generator_layout.c -o generator_layout.wino
gcc  -c generator_layout_style.c -o generator_layout_style.wino
gcc  -c generator_identifier.c -o generator_identifier.wino
gcc  -c string.c -o string.wino
gcc  -c validator.c -o validator.wino
gcc  -c hashmap.c -o hashmap.wino
gcc  -c hashmap_custom.c -o hashmap_custom.wino
gcc  -c array_custom.c -o array_custom.wino
gcc  -c lexer.c -o lexer.wino
gcc  -c ast.c -o ast.wino
gcc  -c ast_layout.c -o ast_layout.wino
gcc  -c ast_layout_style.c -o ast_layout_style.wino
gcc  -c main.c -o main.wino

gcc  -o salam log.wino file.wino memory.wino array.wino parser.wino parser_layout.wino generator.wino generator_layout.wino generator_layout_style.wino generator_identifier.wino string.wino validator.wino hashmap.wino hashmap_custom.wino array_custom.wino lexer.wino ast.wino ast_layout.wino ast_layout_style.wino main.wino

salam ../example/test6.salam
