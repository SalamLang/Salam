/**
 * ---------------------------------------------------------------------------
 * Salam Programming Language
 * ---------------------------------------------------------------------------
 *
 * Welcome to the Salam Programming Language! Salam is a modern, efficient,
 * and developer-friendly language created to inspire and empower programmers
 * of all ages. With its unique features and Persian roots, Salam aims to make
 * programming more accessible and enjoyable for everyone.
 *
 * This file is part of the core implementation of Salam, including its runtime
 * and compiler components. By contributing to or using this codebase, you are
 * part of a growing community dedicated to innovation and inclusivity in
 * programming.
 *
 * Explore Salam further:
 * - Website: https://www.salamlang.ir/
 * - GitHub: https://github.com/SalamLang/Salam
 *
 * Thank you for being part of this journey!
 * ---------------------------------------------------------------------------
 */

#ifndef _LEXER_LEXER_H
#define _LEXER_LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "token.h"
#include "lex.h"
#include "../common/base.h"
#include "../common/file.h"

void print_help();
void print_version();
int run_code(const char *source, const char *filepath, language_map_t selected_language);
int handle_file_command(const char *filename, language_map_t selected_language);
int handle_code_command(const char *content, language_map_t selected_language);
int handle_missing_arguments();
int handle_missing_language_value();
int process_language_flag(int argc, char **argv, language_map_t *selected_language);
int process_command(int argc, char **argv, language_map_t selected_language);

#endif
