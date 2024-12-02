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

#include "generator_identifier.h"

/**
 *
 * @function generator_identifier_init
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {void}
 *
 */
void generator_identifier_init(generator_identifier_t *gen) {
    DEBUG_ME;
    gen->current = memory_allocate(2);

    if (gen->current) {
        gen->current[0] = 'a';
        gen->current[1] = '\0';
    }
}

/**
 *
 * @function generator_identifier_get
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {char*} identifier - Identifier
 *
 */
char *generator_identifier_get(generator_identifier_t *gen) {
    DEBUG_ME;
    int length = strlen(gen->current);
    char *identifier = memory_allocate(length + 1);

    strcpy(identifier, gen->current);

    for (int i = length - 1; i >= 0; i--) {
        if (gen->current[i] < 'z') {
            gen->current[i]++;

            return identifier;
        }

        gen->current[i] = 'a';

        if (i == 0) {
            char *new_current = memory_allocate(length + 2);

            memset_s(new_current, length + 1, 'a', length + 1);

            new_current[length + 1] = '\0';

            memory_destroy(gen->current);

            gen->current = new_current;
        }
    }

    return identifier;
}

/**
 *
 * @function generator_identifier_destroy
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {void}
 *
 */
void generator_identifier_destroy(generator_identifier_t *gen) {
    DEBUG_ME;
    if (gen != NULL) {
        if (gen->current != NULL) {
            memory_destroy(gen->current);
        }

        memory_destroy(gen);
    }
}
