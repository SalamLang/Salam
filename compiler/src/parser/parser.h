/*
 * Salam Programming Language (2024–2026)
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

#endif /* SALAM_PARSER_PARSER_H */
