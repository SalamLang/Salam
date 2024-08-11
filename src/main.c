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

    ast_t* ast = parser_parse(lexer);
    
    ast_debug(ast);

    ast_free(ast);

    lexer_destroy(lexer);

    memory_free(content);
}

int main(int argc, char** argv)
{
    doargs(argc, argv);

    return 0;
}
