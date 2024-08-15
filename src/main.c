#include <stdio.h>
#include <stdlib.h>

#include "base.h"
#include "file.h"
#include "memory.h"
#include "lexer.h"
#include "log.h"
#include "array.h"
#include "ast.h"
#include "parser.h"
#include "generator.h"

void doargs(int argc, char** argv)
{
    DEBUG_ME;
    if (argc < 2) {
        error(1, "Usage: %s <file>\n", argv[0]);
    }

    const char* path = argv[1];
    if (!file_exists(path)) {
        error(1, "File does not exist: %s\n", path);
    }

    size_t size = 0;
    char* content = file_reads(path, &size);

    lexer_t* lexer = lexer_create(path, content);
    lexer_lex(lexer);

    lexer_debug(lexer);

    lexer_save(lexer, "tokens.txt");

    ast_t* ast = parser_parse(lexer);
    
    ast_debug(ast);

    printf("end ast debug\n");

    generator_t* generator = generator_create(ast);
    generator->inlineCSS = false;
    generator->inlineJS = false;

    printf("generate code\n");

    generator_code(generator);

    printf("generate debug\n");

    generator_debug(generator);

    printf("generate save\n");

    generator_save(generator, "index.html", "style.css", "script.js");

    printf("generate destroy\n");
    generator_destroy(generator);

    printf("ast destroy\n");

    ast_destroy(ast);

    printf("end ast destroy\n");

    lexer_destroy(lexer);

    printf("end lexer destroy\n");

    memory_destroy(content);

    printf("end content destroy\n");

    printf("END SUCCESS\n");
}

int main(int argc, char** argv)
{
    DEBUG_ME;
    doargs(argc, argv);

    return 0;
}
