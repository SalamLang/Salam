/*
 * Salam Programming Language (2024-2026)
 *
 *   +-------------------+
 *   |     S A L A M     |
 *   +-------------------+
 *
 * Designed by Seyyed Ali Mohammadiyeh and the Salam Team
 * Born from a decade of language design experience (since 2018)
 *
 * Repository: https://github.com/SalamLang/Salam
 *
 */

#ifndef SALAM_PARSER_PARSER_H
#define SALAM_PARSER_PARSER_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "token/token.h"
#include "ast/ast.h"

bool parser_run(arena_t *a, logger_t *log, const token_stream_t *toks,
                ast_node_t **out_program);

/* True when this token stream fails to parse only because it ends too
 * early - a block still open, an expression still owed an operand. The
 * REPL asks this to decide whether to keep reading the turn instead of
 * guessing block structure from punctuation. Pass a throwaway arena and a
 * silent logger: the parse is a question, not a diagnosis. */
bool parser_needs_more(arena_t *a, logger_t *log, const token_stream_t *toks);

#endif /* SALAM_PARSER_PARSER_H */
