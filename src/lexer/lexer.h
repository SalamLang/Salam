#ifndef _LEXER_LEXER_H_
#define _LEXER_LEXER_H_

#include "token.h"
#include "array_token.h"

#include "../common/base.h"
#include "../common/file.h"

// typedef struct lexer_t lexer_t;

typedef struct lexer_t {
    const char *file_path; // NULL if source is REPL
    char *source;
    language_map_t selected_language;

    size_t index;
    size_t line;
    size_t column;
    size_t source_length;
    array_token_t *tokens;
    size_t token_index;  // For parsing purposes

    void (*destroy)(struct lexer_t *lexer);
    void (*print)(struct lexer_t *lexer);
    char *(*stringify)(struct lexer_t *lexer);
} lexer_t;

lexer_t *lexer_create(const char *file_path, const char *source, language_map_t selected_language);
void lexer_save(lexer_t *lexer, const char *tokens_output);
void lexer_destroy(lexer_t *lexer);
void lexer_print(lexer_t *lexer);
char *lexer_stringify(lexer_t *lexer);

#endif
