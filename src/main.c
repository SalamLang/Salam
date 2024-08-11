#include <stdio.h>
#include <stdlib.h>

#include "base.h"
#include "file.h"
#include "memory.h"
#include "lexer.h"
#include "log.h"
#include "array.h"

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
    printf("%s\n---------------------------------\n", lexer->source);
    lexer_lex(lexer);

    printf("Tokens: %zu\n", lexer->tokens->size);
    for (size_t i = 0; i < lexer->tokens->size; i++) {
        token_t* token = lexer->tokens->data[i];
        token->print(token);
    }

    // array_t* arr = array_create(sizeof(char*), 10);
    // for (int i = 0; i < 10; i++) {
    //     char* str = memory_allocate(10);
    //     snprintf(str, 10, "Hello %d", i);
    //     array_push(arr, &str);
    // }
    // printf("Array: ");
    // for (size_t i = 0; i < arr->size; i++) {
    //     char* str = *(char**)array_get(arr, i);
    //     printf("%s ", str);
    // }
    

    // array_t* arr = array_create(sizeof(int), 10);
    // for (int i = 0; i < 10; i++) {
    //     array_push(arr, &i);
    // }
    // printf("Array: ");
    // for (size_t i = 0; i < arr->size; i++) {
    //     printf("%d ", *(int*)array_get(arr, i));
    // }
    // printf("\n");

    printf("done?\n");
    // lexer_destroy(lexer);

    // memory_free(content);
}

int main(int argc, char** argv)
{
    doargs(argc, argv);

    return 0;
}
