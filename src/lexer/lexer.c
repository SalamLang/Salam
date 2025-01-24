#include "lexer.h"

/**
 *
 * @function lexer_create
 * @brief Creating a new lexer state
 * @params {char*} file_path - File path
 * @params {char*} source - Source code
 * @returns {lexer_t*}
 *
 */
lexer_t *lexer_create(const char *file_path, char *source, language_map_t selected_language) {
    DEBUG_ME;
    lexer_t *lexer = memory_allocate(sizeof(lexer_t));

    lexer->file_path = file_path;
    lexer->source = source;
    lexer->selected_language = selected_language;
    lexer->source_length = strlen(lexer->source);
    lexer->index = 0;
    lexer->line = 1;
    lexer->column = 1;

    lexer->tokens = array_token_init(10);

    lexer->token_index = 0;

    return lexer;
}

/**
 *
 * @function lexer_destroy
 * @brief Destroying a lexer state
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 *
 */
void lexer_destroy(lexer_t *lexer) {
    DEBUG_ME;
    if (lexer != NULL) {
        array_token_destroy(lexer->tokens);

        memory_destroy(lexer);
    }
}

/**
 *
 * @function lexer_save
 * @brief Saving the lexer state
 * @params {lexer_t*} lexer - Lexer state
 * @params {const char*} tokens_output - Tokens output file
 * @returns {void}
 *
 */
void lexer_save(lexer_t *lexer, const char *tokens_output) {
    DEBUG_ME;
    file_writes(tokens_output, "");

    file_appends(tokens_output, "Tokens:\n");
    file_appends(tokens_output, "Lexer source: ");
    file_appends(tokens_output, lexer->source == NULL ? "REPL" : lexer->source);

    file_appends(tokens_output, "\n");
    file_appends(tokens_output, "\n");

    file_appends(tokens_output, "Lexer index: ");
    file_appends(tokens_output, int2string(lexer->index));
    file_appends(tokens_output, "\n");
    file_appends(tokens_output, "Lexer line: ");
    file_appends(tokens_output, int2string(lexer->line));
    file_appends(tokens_output, "\n");
    file_appends(tokens_output, "Lexer column: ");
    file_appends(tokens_output, int2string(lexer->column));

    file_appends(tokens_output, "\n");
    file_appends(tokens_output, "\n");

    for (size_t i = 0; i < lexer->tokens->length; i++) {
        token_t *token = array_token_get(lexer->tokens, i);

        file_appends(tokens_output, token_stringify(token));
        file_appends(tokens_output, "\n");
    }
}

/**
 *
 * @function lexer_debug
 * @brief Debugging the lexer state
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 *
 */
void lexer_debug(lexer_t *lexer) {
    DEBUG_ME;
    printf("============= START LEXER DEBUG =============\n");

    printf("Lexer source: %s\n",
           lexer->source == NULL ? "REPL" : lexer->source);
    printf("Lexer index: %zu\n", lexer->index);
    printf("Lexer line: %zu\n", lexer->line);
    printf("Lexer column: %zu\n", lexer->column);

    lexer->tokens->print(lexer->tokens);

    printf("============= END LEXER DEBUG =============\n");
}
