#ifndef _LEXER_LEXER_H_
#define _LEXER_LEXER_H_

#include "token.h"
#include "array_token.h"

#include "../common/base.h"
#include "../common/file.h"

typedef struct {
    const char *file_path; // NULL if source is REPL
    char *source;
    language_map_t selected_language;

    size_t index;
    size_t line;
    size_t column;
    size_t source_length;
    array_token_t *tokens;
    size_t token_index;  // For parsing purposes

    void (*destroy)(struct array_token_t *array);
    void (*print)(struct array_token_t *array);
    void (*stringify)(struct array_token_t *array);
} lexer_t;

lexer_t *lexer_create(const char *file_path, char *source, language_map_t selected_language);
void lexer_destroy(lexer_t *lexer);
void lexer_save(lexer_t *lexer, const char *tokens_output);
void lexer_debug(lexer_t *lexer);

#endif
