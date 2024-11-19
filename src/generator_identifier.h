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

#ifndef _GENERATOR_IDENTIFIER_H_
#define _GENERATOR_IDENTIFIER_H_

#include "base.h"
#include "memory.h"
#include "string_buffer.h"

typedef struct generator_identifier_t {
    char *current;
} generator_identifier_t;

/**
 *
 * @function generator_identifier_init
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {void}
 *
 */
void generator_identifier_init(generator_identifier_t *gen);

/**
 *
 * @function generator_identifier_get
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {char*} identifier - Identifier
 *
 */
char *generator_identifier_get(generator_identifier_t *gen);

/**
 *
 * @function generator_identifier_destroy
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {void}
 *
 */
void generator_identifier_destroy(generator_identifier_t *gen);

#endif
